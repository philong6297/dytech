// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/response.h"

#include <sstream>
#include <utility>

#include <fmt/format.h>

#include "http/constants.h"
#include "http/header.h"
#include "http/http_utils.h"

namespace longlp::http {

// static
auto Response::Make200Response(
  bool should_close,
  std::optional<std::string> resource_url) -> Response {
  return {kResponseStatusOK.data(), should_close, std::move(resource_url)};
}

// static
auto Response::Make400Response() noexcept -> Response {
  return {kResponseStatusBadRequest.data(), true, std::nullopt};
}

// static
auto Response::Make404Response() noexcept -> Response {
  return {kResponseStatusNotFound.data(), true, std::nullopt};
}

// static
auto Response::Make503Response() noexcept -> Response {
  return {kResponseStatusServiceUnavailable.data(), true, std::nullopt};
}

Response::Response(
  const std::string_view status_code,
  bool should_close,
  std::optional<std::string> resource_url) :
  should_close_(should_close),
  resource_url_(std::move(resource_url)) {
  // construct the status line
  status_line_ = fmt::format("{}{}{}", kHTTPVersion, kSpace, status_code);

  // add necessary headers
  headers_.emplace_back(kHeaderServer, kServerName);
  headers_.emplace_back(
    kHeaderConnection,
    ((should_close_) ? kConnectionClose : kConnectionKeepAlive));
  // if resource is specified and available
  if (resource_url_.has_value() && IsFileExists(resource_url_.value())) {
    size_t content_length = CheckFileSize(resource_url_.value());
    headers_.emplace_back(kHeaderContentLength, std::to_string(content_length));
    // parse out the extension
    auto last_dot = resource_url_.value().find_last_of(kDot);
    if (last_dot != std::string::npos) {
      auto extension_raw_str = resource_url_.value().substr(last_dot + 1);
      auto extension         = ToExtension(extension_raw_str);
      headers_.emplace_back(kHeaderContentType, ExtensionToMime(extension));
    }
  }
  else {
    resource_url_ = std::nullopt;
    headers_.emplace_back(kHeaderContentLength, kContentLengthZero);
  }
}

void Response::Serialize(DynamicByteArray& buffer) {
  // construct everything before body
  std::stringstream str_stream;
  str_stream << fmt::format("{}{}", status_line_, kCRLF);
  for (const auto& header : headers_) {
    str_stream << header.Serialize();
  }
  str_stream << kCRLF;
  std::string response_head = str_stream.str();
  buffer.insert(buffer.end(), response_head.begin(), response_head.end());
}

auto Response::ChangeHeader(
  const std::string_view key,
  const std::string_view new_value) noexcept -> bool {
  for (auto& it : headers_) {
    if (it.GetKey() == key) {
      it.SetValue(new_value);
      return true;
    }
  }
  return false;
}

}    // namespace longlp::http
