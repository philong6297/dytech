/**
 * @file request.h
 * @author Yukun J
 * @expectation this header file

 *
 * *



 * * * *
 * should be compatible to compile in C++
 * program on Linux


 * * *
 *
 *
 * @init_date
 * Dec

 * * 30 2022
 *
 * This is a header file
 *
 * implementing
 * the

 * * HTTP request

 */

#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_
#include <iostream>
#include <string>
#include <vector>

#include "base/macros.h"

namespace longlp::http {

class Header;
enum class Method;
enum class Version;

/**
 * The (limited GET/HEAD-only HTTP 1.1) HTTP Request class
 * it
 *
 *
 *
 * contains



 * * * * necessary request line features including method,
 *
 *

 * * resource url,
 *
 * http


 * * * version and since we supports http 1.1,

 * * it

 * * also cares if the
 * client
 *


 * * * connection should be kept

 * * alive
 */
class Request {
 public:
  Request(
    Method method,
    Version version,
    std::string resource_url,
    const std::vector<Header>& headers) noexcept;
  explicit Request(const std::string& request_str) noexcept;    // deserialize
                                                                // method
  DISALLOW_COPY(Request);
  [[nodiscard]] auto IsValid() const noexcept -> bool;
  [[nodiscard]] auto ShouldClose() const noexcept -> bool;
  [[nodiscard]] auto GetInvalidReason() const noexcept -> std::string;
  [[nodiscard]] auto GetMethod() const noexcept -> Method;
  [[nodiscard]] auto GetVersion() const noexcept -> Version;
  [[nodiscard]] auto GetResourceUrl() const noexcept -> std::string;
  [[nodiscard]] auto GetHeaders() const noexcept -> std::vector<Header>;
  friend auto
  operator<<(std::ostream& os, const Request& request) -> std::ostream&;

 private:
  [[nodiscard]] auto ParseRequestLine(const std::string& request_line) -> bool;
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

#endif    // HTTP_REQUEST_H_
