// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/header.h"

#include <fmt/format.h>

#include "http/constants.h"
#include "http/http_utils.h"

namespace longlp::http {

Header::Header(const std::string_view key, const std::string_view value) :
  key_(key),
  value_(value) {}

Header::Header(const std::string_view line) {
  auto tokens = Split(line, kColon);
  if (tokens.size() < 2) {
    valid_ = false;
    return;
  }
  key_ = std::move(tokens[0]);
  tokens.erase(tokens.begin());
  // the value could be like '127.0.0.1:20080' and get split into more than 1
  value_ = (tokens.size() == 1) ? tokens[0] : Join(tokens, kColon);
}

auto Header::Serialize() const -> std::string {
  return fmt::format(
    "{key}{colon}{value}{crlf}",
    fmt::arg("key", key_),
    fmt::arg("colon", kColon),
    fmt::arg("value", value_),
    fmt::arg("crlf", kCRLF));
}

auto operator<<(std::ostream& os, const Header& header) -> std::ostream& {
  os << fmt::format(
    "HTTP Header contains:\n"
    "Key: {key}\n"
    "Value: {value}\n"
    "IsValid: {is_valid}\n"
    "Serialize to: {serialize}",
    fmt::arg("key", header.GetKey()),
    fmt::arg("value", header.GetValue()),
    fmt::arg("is_valid", (header.IsValid()) ? "True" : "False"),
    fmt::arg("serialize", header.Serialize()));

  return os;
}

}    // namespace longlp::http
