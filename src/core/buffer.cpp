// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/buffer.h"

#include <algorithm>

#include "base/utils.h"

namespace longlp {

Buffer::Buffer(size_t initial_capacity) {
  buf_.reserve(initial_capacity);
}

Buffer::~Buffer() = default;

void Buffer::AppendUnsafe(const Byte* data, size_t size) {
  buf_.insert(buf_.end(), data, data + size);
}

void Buffer::Append(const std::string& str) {
  std::transform(
    str.begin(),
    str.end(),
    std::back_inserter(buf_),
    [](const char c) { return narrow_cast<Byte>(c); });
}

void Buffer::Append(ByteData&& other_buffer) {
  buf_.insert(
    buf_.end(),
    std::make_move_iterator(other_buffer.begin()),
    std::make_move_iterator(other_buffer.end()));
}

void Buffer::AppendHeadUnsafe(const Byte* data, size_t size) {
  buf_.insert(buf_.begin(), data, data + size);
}

void Buffer::AppendHead(ByteData&& other_buffer) {
  buf_.insert(
    buf_.begin(),
    std::make_move_iterator(other_buffer.begin()),
    std::make_move_iterator(other_buffer.end()));
}

void Buffer::AppendHead(const std::string& str) {
  std::transform(str.begin(), str.end(), buf_.begin(), [](const char c) {
    return narrow_cast<Byte>(c);
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
        narrow_cast<decltype(buf_)::difference_type>(pos + target.size())));
  }
  return res;
}

auto Buffer::ToStringView() const noexcept -> std::string_view {
  return {bit_cast<const char*>(buf_.data()), buf_.size()};
}

}    // namespace longlp