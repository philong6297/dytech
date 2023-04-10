// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_HTTP_HEADER_H_
#define SRC_HTTP_HEADER_H_

#include <iostream>
#include <string_view>

#include "base/macros.h"

namespace longlp::http {

// The HTTP Header in the form of string "key : value"
class Header {
 public:
  Header(std::string_view key, std::string_view value);

  explicit Header(std::string_view line);    // deserialize method

  [[nodiscard]] auto Serialize() const -> std::string;

  [[nodiscard]] auto IsValid() const -> bool { return valid_; }

  [[nodiscard]] auto GetKey() const -> std::string { return key_; }

  [[nodiscard]] auto GetValue() const -> std::string { return value_; }

  void SetValue(const std::string_view new_value) noexcept {
    value_ = new_value;
  }

  friend auto
  operator<<(std::ostream& os, const Header& header) -> std::ostream&;

 private:
  std::string key_;
  std::string value_;
  bool valid_{true};
};

}    // namespace longlp::http

#endif    // SRC_HTTP_HEADER_H_
