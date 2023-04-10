// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/request.h"

#include <algorithm>

#include <fmt/format.h>

#include "http/constants.h"
#include "http/header.h"
#include "http/http_utils.h"

namespace longlp::http {

namespace {
auto ToString(const Method method) -> std::string {
  switch (method) {
    case Method::kGET:
      return "GET";
    case Method::kHEAD:
      return "HEAD";
    case Method::kUnsupported:
      return "UNSUPPORTED";
  }
}

auto ToString(const Version version) -> std::string {
  switch (version) {
    case Version::kHTTP_1_1:
      return "HTTP/1.1";
    case Version::kUnsupported:
      return "UNSUPPORTED";
  }
}
}    // namespace

Request::Request(
  Method method,
  Version version,
  const std::string_view resource_url,
  const std::vector<Header>& headers) noexcept :
  method_(method),
  version_(version),
  resource_url_(resource_url),
  headers_(headers),
  is_valid_(true) {}

Request::Request(const std::string_view request_str) noexcept :
  method_{Method::kUnsupported},
  version_{Version::kUnsupported} {
  auto lines = Split(request_str, kCRLF);
  if (lines.size() < 2 || !lines.back().empty()) {
    invalid_reason_ = "Request format is wrong.";
    return;
  }

  // the ending of a request should be '\r\n\r\n' which is split to empty string
  // in the last token
  if (!lines.back().empty()) {
    invalid_reason_ = "Ending of the request is not \r\n\r\n";
    return;
  }

  lines.pop_back();
  if (!ParseRequestLine(lines[0])) {
    return;
  }

  lines.erase(lines.begin());
  for (const auto& line : lines) {
    Header header{line};
    if (!header.IsValid()) {
      invalid_reason_ = "Fail to parse header line: " + line;
      return;
    }
    ScanHeader(header);
    headers_.push_back(std::move(header));
  }
  is_valid_ = true;
}

auto Request::ParseRequestLine(const std::string_view request_line) -> bool {
  const auto tokens = Split(request_line, kSpace);
  if (tokens.size() != 3) {
    invalid_reason_ =
      fmt::format("Invalid first request headline: {}", request_line);
    return false;
  }

  method_ = ToMethod(tokens[0]);
  if (method_ == Method::kUnsupported) {
    invalid_reason_ = fmt::format("Unsupported method: {}", tokens[0]);
    return false;
  }

  version_ = ToVersion(tokens[2]);
  if (version_ == Version::kUnsupported) {
    invalid_reason_ = fmt::format("Unsupported version: {}", tokens[2]);
    return false;
  }

  // default route to index.html
  resource_url_ =
    (tokens[1].empty() || tokens[1].at(tokens[1].size() - 1) == '/')
      ? fmt::format("{}{}", tokens[1], kDefaultRoute)
      : tokens[1];
  return true;
}

void Request::ScanHeader(const Header& header) {
  // currently only scan for whether the connection should be closed after
  // service
  const auto key = Format(header.GetKey());
  if (key == Format(kHeaderConnection)) {
    const auto value = Format(header.GetValue());
    if (value == Format(kConnectionKeepAlive)) {
      should_close_ = false;
    }
  }
}

auto operator<<(std::ostream& os, const Request& request) -> std::ostream& {
  if (!request.IsValid()) {
    os << fmt::format(
      "Request is not invalid.\n"
      "Reason: {}\n",
      request.invalid_reason_);
  }
  else {
    os << fmt::format(
      "Request is valid.\n"
      "Method: {method}\n"
      "HTTP Version: {http_version}\n"
      "Resource Url: {resource_url}\n"
      "Connection Keep Alive: {alive}\n"
      "Headers: \n",
      fmt::arg("method", ToString(request.method_)),
      fmt::arg("http_version", ToString(request.version_)),
      fmt::arg("resource_url", request.resource_url_),
      fmt::arg("alive", ((request.should_close_) ? "False" : "True")));

    const auto headers = request.GetHeaders();
    std::for_each(headers.begin(), headers.end(), [&](auto header) {
      os << header.Serialize();
    });
  }
  return os;
}

}    // namespace longlp::http
