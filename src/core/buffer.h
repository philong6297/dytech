// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_BUFFER_H_
#define CORE_BUFFER_H_

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
    // default initial underlying capacity of Buffer
    static constexpr size_t kDefaultCapacity = 1024;

    explicit Buffer(size_t initial_capacity = kDefaultCapacity);

    ~Buffer() = default;
    NON_MOVEABLE(Buffer);
    DEFAULT_COPY(Buffer);

    void Append(const unsigned char* new_char_data, size_t data_size);

    void Append(const std::string& new_str_data);

    void Append(std::vector<unsigned char>&& other_buffer);

    void AppendHead(const unsigned char* new_char_data, size_t data_size);

    void AppendHead(const std::string& new_str_data);

    auto
    FindAndPopTill(const std::string& target) -> std::optional<std::string>;

    [[nodiscard]] auto Size() const noexcept -> size_t;

    [[nodiscard]] auto Capacity() const noexcept -> size_t;

    auto Data() noexcept -> const unsigned char*;

    [[nodiscard]] auto ToStringView() const noexcept -> std::string_view;

    void Clear() noexcept;

   private:
    std::vector<unsigned char> buf_;
  };

}    // namespace longlp
#endif    // CORE_BUFFER_H_
