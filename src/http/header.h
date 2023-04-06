/**
 * @file header.h
 * @author Yukun J
 * @expectation this header file
 *
 *
 * should
 * be compatible to compile in C++
 * program on Linux
 * @init_date


 * * * Dec 29
 * 2022
 *
 * This is a header file implementing the HTTP Header

 * *
 * class, which
 * is
 * essentially a key-value pair
 */

#ifndef HTTP_HEADER_H_
#define HTTP_HEADER_H_
#include <iostream>
#include <sstream>
#include <string>

namespace longlp::HTTP {

/**
 * The HTTP Header in the form of string "key : value"
 */
class Header {
 public:
  Header(const std::string& key, const std::string& value);
  Header(std::string&& key, std::string&& value);
  explicit Header(const std::string& line);    // deserialize method
  Header(const Header& other) = default;
  Header(Header&& other) noexcept;
  [[nodiscard]] auto operator=(const Header& other) -> Header& = default;
  [[nodiscard]] auto operator=(Header&& other) noexcept -> Header&;
  ~Header() = default;

  [[nodiscard]] auto IsValid() const -> bool;
  [[nodiscard]] auto GetKey() const -> std::string;
  [[nodiscard]] auto GetValue() const -> std::string;
  void SetValue(const std::string& new_value) noexcept;
  [[nodiscard]] auto Serialize() const -> std::string;

  friend auto
  operator<<(std::ostream& os, const Header& header) -> std::ostream&;

 private:
  std::string key_;
  std::string value_;
  bool valid_{true};
};

}    // namespace longlp::HTTP

#endif    // HTTP_HEADER_H_
