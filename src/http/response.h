// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_HTTP_RESPONSE_H_
#define SRC_HTTP_RESPONSE_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/typedefs.h"

namespace longlp::http {

class Header;

// The HTTP Response class use vector of char to be able to contain binary data
class Response {
 public:
  // 200 OK response
  [[nodiscard]] static auto
  Make200Response(bool should_close, std::optional<std::string> resource_url)
    -> Response;
  // 400 Bad Request response, close connection
  [[nodiscard]] static auto Make400Response() noexcept -> Response;
  // 404 Not Found response, close connection
  [[nodiscard]] static auto Make404Response() noexcept -> Response;
  // 503 Service Unavailable response, close connection
  [[nodiscard]] static auto Make503Response() noexcept -> Response;

  Response(
    std::string_view status_code,
    bool should_close,
    std::optional<std::string> resource_url);

  // no content, content should separately be loaded
  void Serialize(DynamicByteArray& buffer);

  [[nodiscard]] auto GetHeaders() -> std::vector<Header> { return headers_; }

  [[nodiscard]] auto
  ChangeHeader(std::string_view key, std::string_view new_value) noexcept
    -> bool;

 private:
  std::string status_line_;
  bool should_close_;
  std::vector<Header> headers_;
  std::optional<std::string> resource_url_;
  DynamicByteArray body_;
};

}    // namespace longlp::http

#endif    // SRC_HTTP_RESPONSE_H_
