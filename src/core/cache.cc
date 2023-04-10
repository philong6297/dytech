// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/cache.h"

#include <cassert>
#include <utility>

#include "base/chrono.h"

namespace longlp {

// Helper class inside the Cache
// It represents a single file cached in the form of an Byte vector
// and serves as a node in the doubly-linked list data structure
class Cache::CacheNode {
 public:
  CacheNode() noexcept { UpdateTimestamp(); }

  CacheNode(std::string identifier, const DynamicByteArray& data) :
    identifier_(std::move(identifier)),
    data_(data) {
    UpdateTimestamp();
  }

  void Serialize(DynamicByteArray& destination) {
    if (const auto new_cap = destination.size() + data_.size();
        destination.capacity() < new_cap) {
      destination.reserve(new_cap);
    }
    destination.insert(destination.end(), data_.begin(), data_.end());
  }

  void Detach() {
    prev_->next_ = next_;
    next_->prev_ = prev_;
  }

  void UpdateTimestamp() noexcept { last_access_ = GetCurrentTimeMs().count(); }

 private:
  friend class Cache;

  // the resource identifier for this node
  std::string identifier_;
  // may contain binary data
  DynamicByteArray data_;
  // the timestamp of last access in milliseconds
  int64_t last_access_{0};
  CacheNode* prev_{nullptr};
  CacheNode* next_{nullptr};
};

Cache::Cache(size_t capacity) noexcept :
  capacity_(capacity),
  head_(std::make_unique<CacheNode>()),
  tail_(std::make_unique<CacheNode>()) {
  head_->next_ = tail_.get();
  tail_->prev_ = head_.get();
}

Cache::~Cache() = default;

auto Cache::TryLoad(
  const std::string& resource_url,
  DynamicByteArray& destination) -> bool {
  // TODO(longlp): Try to use unique
  std::shared_lock<std::shared_mutex> lock(mtx_);
  auto iter = mapping_.find(resource_url);
  if (iter != mapping_.end()) {
    iter->second->Serialize(destination);
    // move this node to the tailer as most recently accessed
    iter->second->Detach();
    AppendToListTail(iter->second);
    iter->second->UpdateTimestamp();
    return true;
  }
  return false;
}

auto Cache::TryInsert(
  const std::string& resource_url,
  const DynamicByteArray& source) -> bool {
  // TODO(longlp): Try to use unique
  std::unique_lock<std::shared_mutex> lock(mtx_);

  // already exists
  if (mapping_.contains(resource_url)) {
    return false;
  }

  // single resource's size exceeds the capacity
  if (source.size() > capacity_) {
    return false;
  }

  // try to eliminate older resources to add new one
  while (!mapping_.empty() && (capacity_ - occupancy_) < source.size()) {
    auto* first_node   = head_->next_;
    auto resource_size = first_node->data_.size();
    auto iter          = mapping_.find(first_node->identifier_);
    // it should be in the map
    assert(iter != mapping_.end());
    iter->second->Detach();
    mapping_.erase(iter);
    occupancy_ -= resource_size;
  }

  auto node = std::make_shared<CacheNode>(resource_url, source);
  AppendToListTail(node);
  occupancy_ += source.size();
  mapping_.emplace(resource_url, node);
  return true;
}

void Cache::Clear() {
  head_->next_ = tail_.get();
  tail_->prev_ = head_.get();
  mapping_.clear();
  occupancy_ = 0;
}

void Cache::AppendToListTail(const std::shared_ptr<CacheNode>& node) noexcept {
  auto* node_ptr   = node.get();
  auto* node_prev  = tail_->prev_;
  node_prev->next_ = node_ptr;
  tail_->prev_     = node_ptr;
  node_ptr->prev_  = node_prev;
  node_ptr->next_  = tail_.get();
}
}    // namespace longlp
