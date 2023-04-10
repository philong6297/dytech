/**
 * @file response.cpp
 * @author Yukun J
 * @expectation this
 *
 *

 *
 *

 * * *






 * * * *
 * *

 * * *


 * * * *



 * * * *
 * implementation
 *

 * *

 * * file should
 * be

 * *

 * * compatible to

 * *
 * compile
 * in


 * * *

 * *

 * * C++
 *
 *

 * *

 * * program
 *


 * *

 * * * on


 * * *

 * * Linux

 * *

 *
 * *
 * @init_date

 * *
 * Jan

 * * 10
 *
 *
 * 2023
 *


 * * *
 *
 * This

 * *
 * is
 *

 * * an
 *
 *
 *
 *
 *

 * * implementation

 * *
 * file


 * * *

 *
 * *

 * *

 * *
 * implementing the
 *
 * HTTP

 * *

 * *
 *
 *
 *
 *
 * response

 */

#include "http/response.h"

#include <sstream>
#include <utility>

#include "http/constants.h"
#include "http/header.h"

namespace longlp::http {

auto Response::Make200Response(
  bool should_close,
  std::optional<std::string> resource_url) -> Response {
  return {kResponseStatusOK, should_close, std::move(resource_url)};
}

auto Response::Make400Response() noexcept -> Response {
  return {kResponseStatusBadRequest, true, std::nullopt};
}

auto Response::Make404Response() noexcept -> Response {
  return {kResponseStatusNotFound, true, std::nullopt};
}

auto Response::Make503Response() noexcept -> Response {
  return {kResponseStatusServiceUnavailable, true, std::nullopt};
}

Response::Response(
  const std::string& status_code,
  bool should_close,
  std::optional<std::string> resource_url) :
  should_close_(should_close),
  resource_url_(std::move(resource_url)) {
  // construct the status line
  std::stringstream str_stream;
  str_stream << kHTTPVersion << kSpace << status_code;
  status_line_ = str_stream.str();
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
  str_stream << status_line_ << kCRLF;
  for (const auto& header : headers_) {
    str_stream << header.Serialize();
  }
  str_stream << kCRLF;
  std::string response_head = str_stream.str();
  buffer.insert(buffer.end(), response_head.begin(), response_head.end());
}

auto Response::GetHeaders() -> std::vector<Header> {
  return headers_;
}

auto Response::ChangeHeader(
  const std::string& key,
  const std::string& new_value) noexcept -> bool {
  for (auto& it : headers_) {
    if (it.GetKey() == key) {
      it.SetValue(new_value);
      return true;
    }
  }
  return false;
}

}    // namespace longlp::http
