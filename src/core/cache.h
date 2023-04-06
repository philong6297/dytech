// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_CACHE_H_
#define CORE_CACHE_H_

#include <core/utils.h>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace longlp {

// An concurrent LRU cache to reduce load on server disk I/O and improve the
// responsiveness.
// It uses doubly-linked list and hashmap to achieve O(1) time
// seek, insert, update nodes that are closer to the header are to be victims to
// be evicted next time, i.e with older timestamp nodes newly-added or accessed
// are closer to the tail, i.e. with newer timestamp
class Cache {
 public:
  /* default cache size 10 MB */
  static constexpr size_t kDefaultCapacity = 10U * 1024U * 1024U;

  explicit Cache(size_t capacity) noexcept;
  DISALLOW_COPY_AND_MOVE(Cache);
  ~Cache();

  [[nodiscard]] auto GetOccupancy() const noexcept -> size_t {
    return occupancy_;
  }

  [[nodiscard]] auto GetCapacity() const noexcept -> size_t {
    return capacity_;
  }

  [[nodiscard]] auto
  TryLoad(const std::string& resource_url, std::vector<uint8_t>& destination)
    -> bool;

  [[nodiscard]] auto
  TryInsert(const std::string& resource_url, const std::vector<uint8_t>& source)
    -> bool;

  void Clear();

 private:
  class CacheNode;

  // Evict out the head cache node to save space
  void EvictOne() noexcept;

  /**
   * Append a node to the tail of the doubly-linked list
   */
  void AppendToListTail(const std::shared_ptr<CacheNode>& node) noexcept;

  // concurrency
  std::shared_mutex mtx_;

  // map a key (resource name) to the corresponding cache node if exists
  std::unordered_map<std::string, std::shared_ptr<CacheNode>> mapping_;

  // the upper limit of cache storage capacity in bytes
  size_t capacity_;

  // current occupancyin bytes
  size_t occupancy_{0};

  std::shared_ptr<CacheNode> head_;
  std::shared_ptr<CacheNode> tail_;
};

}    // namespace longlp

#endif    // CORE_CACHE_H_
