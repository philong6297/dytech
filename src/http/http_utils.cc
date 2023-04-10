// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/http_utils.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>

#include "base/no_destructor.h"
#include "base/utils.h"

namespace longlp::http {
using std::boyer_moore_horspool_searcher;

auto ToMethod(const std::string_view method_str) noexcept -> Method {
  static const NoDestructor<std::map<std::string, Method>> kStringToMethod{
    {{"GET", Method::kGET},
     {"HEAD", Method::kHEAD},
     {"UNSUPPORTED", Method::kUnsupported}}
  };

  const auto method_str_formatted = Format(method_str);

  if (const auto found = kStringToMethod->find(method_str_formatted);
      found != kStringToMethod->end()) {
    return found->second;
  }
  return Method::kUnsupported;
}

auto ToVersion(const std::string_view version_str) noexcept -> Version {
  static const base::NoDestructor<std::map<std::string, Version>>
    kStringToVersion{
      {{"HTTP/1.1", Version::kHTTP_1_1},
       {
       "UNSUPPORTED",
       Version::kUnsupported,
       }}
  };
  const auto version_str_formatted = Format(version_str);
  if (const auto found = kStringToVersion->find(version_str_formatted);
      found != kStringToVersion->end()) {
    return found->second;
  }
  return Version::kUnsupported;
}

auto ToExtension(const std::string_view extension_str) noexcept -> Extension {
  static const NoDestructor<std::map<std::string, Extension>> kStringToExtension{
    {{"HTML", Extension::kHTML},
     {"CSS", Extension::kCSS},
     {"PNG", Extension::kPNG},
     {"JPG", Extension::kJPG},
     {"JPEG", Extension::kJPEG},
     {"GIF", Extension::kGIF},
     {"OCTET", Extension::kOCTET}}
  };
  const auto extension_str_formatted = Format(extension_str);
  if (const auto found = kStringToExtension->find(extension_str_formatted);
      found != kStringToExtension->end()) {
    return found->second;
  }
  return Extension::kOCTET;
}

auto ExtensionToMime(Extension extension) noexcept -> std::string {
  switch (extension) {
    case Extension::kHTML:
      return kMimeTypeHTML.data();
    case Extension::kCSS:
      return kMimeTypeCSS.data();
    case Extension::kPNG:
      return kMimeTypePNG.data();
    case Extension::kJPG:
      return kMimeTypeJPG.data();
    case Extension::kJPEG:
      return kMimeTypeJPEG.data();
    case Extension::kGIF:
      return kMimeTypeGIF.data();
    case Extension::kOCTET:
      return kMimeTypeOCTET.data();
  }
  return kMimeTypeOCTET.data();
}

auto Split(const std::string_view str, const std::string_view delim) noexcept
  -> std::vector<std::string> {
  std::vector<std::string> tokens;
  if (str.empty()) {
    return tokens;
  }
  size_t curr = 0;
  size_t next{};
  while ((next = str.find(delim, curr)) != std::string::npos) {
    tokens.emplace_back(str.substr(curr, next - curr));
    curr = next + delim.size();
  }
  if (curr != str.size()) {
    // one last word
    tokens.emplace_back(str.substr(curr, str.size() - curr));
  }
  return tokens;
}

auto Join(
  const std::vector<std::string>& tokens,
  const std::string_view delim) noexcept -> std::string {
  if (tokens.empty()) {
    return {};
  }
  if (tokens.size() == 1) {
    return tokens[0];
  }
  std::stringstream str_stream;
  for (size_t i = 0; i < tokens.size() - 1; ++i) {
    str_stream << tokens[i] << delim;
  }
  str_stream << tokens[tokens.size() - 1];
  return str_stream.str();
}

auto Trim(const std::string_view str, const std::string_view delim) noexcept
  -> std::string {
  size_t r_found = str.find_last_not_of(delim);
  if (r_found == std::string::npos) {
    return {};
  }
  size_t l_found = str.find_first_not_of(delim);
  return str.substr(l_found, r_found - l_found + 1).data();
}

auto ToUpper(const std::string_view str) noexcept -> std::string {
  std::string result(str.data());
  for (auto& c : result) {
    c = narrow_cast<char>(std::toupper(c));
  }

  return result;
}

auto Format(const std::string_view str) noexcept -> std::string {
  return ToUpper(Trim(str, kSpace));
}

auto IsDirectoryExists(const std::string_view directory_path) noexcept -> bool {
  return std::filesystem::is_directory(directory_path);
}

auto IsCGIRequest(const std::string_view resource_url) noexcept -> bool {
  return std::search(
           resource_url.begin(),
           resource_url.end(),
           std::boyer_moore_horspool_searcher<std::string_view::iterator>(
             kCGIFolderName.begin(),
             kCGIFolderName.end())) != resource_url.end();
}

auto IsFileExists(const std::string_view file_path) noexcept -> bool {
  return std::filesystem::exists(file_path);
}

auto DeleteFile(const std::string_view file_path) noexcept -> bool {
  return std::filesystem::remove(file_path);
}

auto CheckFileSize(const std::string_view file_path) noexcept -> size_t {
  assert(IsFileExists(file_path));
  return std::filesystem::file_size(file_path);
}

void LoadFile(
  const std::string_view file_path,
  DynamicByteArray& buffer) noexcept {
  size_t file_size       = CheckFileSize(file_path);
  size_t buffer_old_size = buffer.size();

  std::ifstream file(file_path.data());
  buffer.resize(buffer_old_size + file_size);
  assert(file.is_open());
  file.read(
    bit_cast<char*>(&buffer[buffer_old_size]),
    narrow_cast<std::streamsize>(file_size));
}

}    // namespace longlp::http
