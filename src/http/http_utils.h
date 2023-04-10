// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef HTTP_HTTP_UTILS_H_
#define HTTP_HTTP_UTILS_H_

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "core/typedefs.h"
#include "http/constants.h"

namespace longlp::http {

// space and case insensitive
[[nodiscard]] auto ToMethod(std::string_view method_str) noexcept -> Method;

// space and case insensitive
[[nodiscard]] auto ToVersion(std::string_view version_str) noexcept -> Version;

// space and case insensitive
[[nodiscard]] auto
ToExtension(std::string_view extension_str) noexcept -> Extension;

// space and case insensitive
[[nodiscard]] auto ExtensionToMime(Extension extension) noexcept -> std::string;

[[nodiscard]] auto Split(std::string_view str, std::string_view delim) noexcept
  -> std::vector<std::string>;

// concatenate a collection of strings using the specified delimiter
[[nodiscard]] auto
Join(const std::vector<std::string>& tokens, std::string_view delim) noexcept
  -> std::string;

// Remove the leading and trailing specified delimiter (not inplace)
[[nodiscard]] auto
Trim(std::string_view str, std::string_view delim) noexcept -> std::string;

[[nodiscard]] auto ToUpper(std::string_view str) noexcept -> std::string;

// Apply Trim + ToUpper to a string and return the formatted version
[[nodiscard]] auto Format(std::string_view str) noexcept -> std::string;

[[nodiscard]] auto
IsDirectoryExists(std::string_view directory_path) noexcept -> bool;

[[nodiscard]] auto IsCgiRequest(std::string_view resource_url) noexcept -> bool;

[[nodiscard]] auto IsFileExists(std::string_view file_path) noexcept -> bool;

[[nodiscard]] auto DeleteFile(std::string_view file_path) noexcept -> bool;

[[nodiscard]] auto CheckFileSize(std::string_view file_path) noexcept -> size_t;

void LoadFile(std::string_view file_path, DynamicByteArray& buffer) noexcept;

}    // namespace longlp::http

#endif    // HTTP_HTTP_UTILS_H_
