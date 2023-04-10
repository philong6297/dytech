// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_HTTP_REQUEST_H_
#define SRC_HTTP_REQUEST_H_

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "base/macros.h"

namespace longlp::http {

class Header;
enum class Method;
enum class Version;

// The (limited GET/HEAD-only HTTP 1.1) HTTP Request class
// contains necessary request line features including method, resource url, http
// version and since we supports http 1.1, it also cares if the client
// connection should be kept alive
class Request {
 public:
  Request(
    Method method,
    Version version,
    std::string_view resource_url,
    const std::vector<Header>& headers) noexcept;

  explicit Request(std::string_view request_str) noexcept;    // deserialize
                                                              // method
  DISALLOW_COPY(Request);
  DEFAULT_MOVE(Request);
  ~Request() = default;

  [[nodiscard]] auto ShouldClose() const noexcept -> bool {
    return should_close_;
  }

  [[nodiscard]] auto IsValid() const noexcept -> bool { return is_valid_; }

  [[nodiscard]] auto GetMethod() const noexcept -> Method { return method_; }

  [[nodiscard]] auto GetVersion() const noexcept -> Version { return version_; }

  [[nodiscard]] auto GetResourceUrl() const noexcept -> std::string {
    return resource_url_;
  }

  [[nodiscard]] auto GetHeaders() const noexcept -> std::vector<Header> {
    return headers_;
  }

  [[nodiscard]] auto GetInvalidReason() const noexcept -> std::string {
    return invalid_reason_;
  }

  friend auto
  operator<<(std::ostream& os, const Request& request) -> std::ostream&;

 private:
  [[nodiscard]] auto ParseRequestLine(std::string_view request_line) -> bool;
  void ScanHeader(const Header& header);

  Method method_;
  Version version_;
  std::string resource_url_;
  std::vector<Header> headers_;
  bool should_close_{true};
  bool is_valid_{false};
  std::string invalid_reason_;
};
}    // namespace longlp::http

#endif    // SRC_HTTP_REQUEST_H_
