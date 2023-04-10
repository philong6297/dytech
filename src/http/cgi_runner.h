// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_HTTP_CGI_RUNNER_H_
#define SRC_HTTP_CGI_RUNNER_H_

#include <string>
#include <string_view>
#include <vector>

#include "base/macros.h"
#include "core/typedefs.h"

namespace longlp::http {

// This CGIRunner runs a client commanded program through traditional 'fork' +
// 'execve'
// All the cgi program should reside in a '/cgi-bin' folder in the root
// directory of the http serving directory parent and child process communicate
// through a file, where child writes the output to it and parent read it out
// afterwards
class CGIRunner {
 public:
  [[nodiscard]] static auto
  ParseCGIRunner(std::string_view resource_url) noexcept -> CGIRunner;
  [[nodiscard]] static auto MakeInvalidCGIRunner() noexcept -> CGIRunner;

  explicit CGIRunner(
    std::string_view path,
    const std::vector<std::string>& arguments) noexcept;

  [[nodiscard]] auto Run() -> DynamicByteArray;

  [[nodiscard]] auto IsValid() const noexcept -> bool { return valid_; }

  [[nodiscard]] auto GetPath() const noexcept -> std::string {
    return cgi_program_path_;
  }

 private:
  std::string cgi_program_path_;
  std::vector<std::string> cgi_arguments_;
  bool valid_{true};
};

}    // namespace longlp::http

#endif    // SRC_HTTP_CGI_RUNNER_H_
