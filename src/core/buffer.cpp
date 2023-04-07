// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/buffer.h"

#include <algorithm>
#include <bit>
#include <gsl/assert>

namespace longlp {

Buffer::Buffer(size_t initial_capacity) {
  buf_.reserve(initial_capacity);
}

Buffer::~Buffer() = default;

void Buffer::AppendUnsafe(const uint8_t* data, size_t size) {
  buf_.insert(buf_.end(), data, data + size);
}

void Buffer::Append(const std::string& str) {
  std::transform(
    str.begin(),
    str.end(),
    std::back_inserter(buf_),
    [](const char c) { return static_cast<uint8_t>(c); });
}

void Buffer::Append(std::vector<uint8_t>&& other_buffer) {
  buf_.insert(
    buf_.end(),
    std::make_move_iterator(other_buffer.begin()),
    std::make_move_iterator(other_buffer.end()));
}

void Buffer::AppendHeadUnsafe(const uint8_t* data, size_t size) {
  buf_.insert(buf_.begin(), data, data + size);
}

void Buffer::AppendHead(std::vector<uint8_t>&& other_buffer) {
  buf_.insert(
    buf_.begin(),
    std::make_move_iterator(other_buffer.begin()),
    std::make_move_iterator(other_buffer.end()));
}

void Buffer::AppendHead(const std::string& str) {
  std::transform(str.begin(), str.end(), buf_.begin(), [](const char c) {
    return static_cast<uint8_t>(c);
  });
}

auto Buffer::FindAndPopTill(const std::string& target)
  -> std::optional<std::string> {
  std::optional<std::string> res = std::nullopt;
  auto curr_content              = ToStringView();
  auto pos                       = curr_content.find(target);
  if (pos != std::string::npos) {
    res = curr_content.substr(0, pos + target.size());

    buf_.erase(
      buf_.begin(),
      std::next(
        buf_.begin(),
        static_cast<decltype(buf_)::difference_type>(pos + target.size())));
  }
  return res;
}

auto Buffer::ToStringView() const noexcept -> std::string_view {
  return {std::bit_cast<const char*>(buf_.data()), buf_.size()};
}

}    // namespace longlp
