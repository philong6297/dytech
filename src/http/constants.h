// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_HTTP_CONSTANTS_H_
#define SRC_HTTP_CONSTANTS_H_

#include <map>
#include <string_view>

namespace longlp::http {

constexpr auto kReadWritePermission       = 0600;
constexpr std::string_view kCGIFolderName = "cgi-bin";
constexpr std::string_view kCGIPrefix     = "cgi_temp";
constexpr std::string_view kSeparator     = "&";
constexpr std::string_view kUnderscore    = "_";
constexpr std::string_view kSpace         = " ";
constexpr std::string_view kDot           = ".";
constexpr std::string_view kCRLF          = "\r\n";
constexpr std::string_view kColon         = ":";
constexpr std::string_view kDefaultRoute  = "index.html";

// Common Header and Value
constexpr std::string_view kHeaderServer        = "Server";
constexpr std::string_view kServerName          = "longlp/1.0";
constexpr std::string_view kHeaderContentLength = "Content-Length";
constexpr std::string_view kHeaderContentType   = "Content-Type";
constexpr std::string_view kContentLengthZero   = "0";
constexpr std::string_view kHeaderConnection    = "Connection";
constexpr std::string_view kConnectionClose     = "Close";
constexpr std::string_view kConnectionKeepAlive = "Keep-Alive";
constexpr std::string_view kHTTPVersion         = "HTTP/1.1";

// MIME Types
constexpr std::string_view kMimeTypeHTML  = "text/html";
constexpr std::string_view kMimeTypeCSS   = "text/css";
constexpr std::string_view kMimeTypePNG   = "image/png";
constexpr std::string_view kMimeTypeJPG   = "image/jpg";
constexpr std::string_view kMimeTypeJPEG  = "image/jpeg";
constexpr std::string_view kMimeTypeGIF   = "image/gif";
constexpr std::string_view kMimeTypeOCTET = "application/octet-stream";

// Response status
constexpr std::string_view kResponseStatusOK         = "200 OK";
constexpr std::string_view kResponseStatusBadRequest = "400 Bad Request";
constexpr std::string_view kResponseStatusNotFound   = "404 Not Found";
constexpr std::string_view kResponseStatusServiceUnavailable =
  "503 Service Unavailable";

// only support GET/HEAD method now
enum class Method {
  kGET,
  kHEAD,
  kUnsupported
};

// only support HTTP 1.1 now
enum class Version {
  kHTTP_1_1,
  kUnsupported
};

// Content Extension enum
enum class Extension {
  kHTML,
  kCSS,
  kPNG,
  kJPG,
  kJPEG,
  kGIF,
  kOCTET
};

}    // namespace longlp::http

#endif    // SRC_HTTP_CONSTANTS_H_
