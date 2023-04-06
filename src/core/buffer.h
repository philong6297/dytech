// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_BUFFER_H_
#define CORE_BUFFER_H_

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/utils.h"

namespace longlp {

// Buffer to push-in and pop-out bytes in order.
// This Buffer abstracts an underlying dynamic char array that allows pushing
// in byte data from two ends.
// NOT thread-safe
class Buffer {
 public:
  static constexpr size_t kDefaultCapacity = 1024;

  explicit Buffer(size_t initial_capacity = kDefaultCapacity);

  ~Buffer();
  DISALLOW_MOVE(Buffer);
  DEFAULT_COPY(Buffer);

  void AppendUnsafe(const uint8_t* data, size_t size);

  void Append(const std::string& str);

  void Append(std::vector<uint8_t>&& other_buffer);

  void AppendHeadUnsafe(const uint8_t* data, size_t size);

  void AppendHead(std::vector<uint8_t>&& other_buffer);

  void AppendHead(const std::string& str);

  [[nodiscard]] auto
  FindAndPopTill(const std::string& target) -> std::optional<std::string>;

  [[nodiscard]] auto Size() const noexcept -> size_t { return buf_.size(); }

  [[nodiscard]] auto Capacity() const noexcept -> size_t {
    return buf_.capacity();
  }

  [[nodiscard]] auto Data() noexcept -> const uint8_t* { return buf_.data(); }

  void Clear() noexcept { buf_.clear(); }

  [[nodiscard]] auto ToStringView() const noexcept -> std::string_view;

 private:
  std::vector<uint8_t> buf_;
};

}    // namespace longlp
#endif    // CORE_BUFFER_H_
