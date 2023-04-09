// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_BUFFER_H_
#define SRC_CORE_BUFFER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "base/macros.h"
#include "core/typedefs.h"

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

  void AppendUnsafe(const Byte* data, size_t size);

  void Append(const std::string& str);

  void Append(DynamicByteArray&& other_buffer);

  void AppendHeadUnsafe(const Byte* data, size_t size);

  void AppendHead(DynamicByteArray&& other_buffer);

  void AppendHead(const std::string& str);

  [[nodiscard]] auto
  FindAndPopTill(const std::string& target) -> std::optional<std::string>;

  [[nodiscard]] auto Size() const noexcept -> size_t { return buf_.size(); }

  [[nodiscard]] auto Capacity() const noexcept -> size_t {
    return buf_.capacity();
  }

  [[nodiscard]] auto Data() noexcept -> const Byte* { return buf_.data(); }

  void Clear() noexcept { buf_.clear(); }

  [[nodiscard]] auto ToStringView() const noexcept -> std::string_view;

 private:
  DynamicByteArray buf_;
};

}    // namespace longlp
#endif    // SRC_CORE_BUFFER_H_
