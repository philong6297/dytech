// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/cgi_runner.h"

#include <fcntl.h>
#include <sys/wait.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string_view>
#include <thread>

#include <fmt/format.h>

#include "http/constants.h"
#include "http/http_utils.h"

namespace longlp::http {

// static
auto CGIRunner::ParseCGIRunner(const std::string_view resource_url) noexcept
  -> CGIRunner {
  if (resource_url.empty() || !IsCgiRequest(resource_url)) {
    return MakeInvalidCGIRunner();
  }
  // find the first & after the cgi-bin/ to fetch out cgi program path
  const auto cgi_pos       = resource_url.find(kCGIFolderName);
  const auto cgi_separator = resource_url.find(kSeparator, cgi_pos);
  const auto cgi_path      = resource_url.substr(0, cgi_separator);
  const auto arguments =
    Split(resource_url.substr(cgi_separator + 1), kSeparator);
  return CGIRunner(cgi_path, arguments);
}

// static
auto CGIRunner::MakeInvalidCGIRunner() noexcept -> CGIRunner {
  CGIRunner invalid_cgier{std::string(), std::vector<std::string>()};
  invalid_cgier.valid_ = false;
  return invalid_cgier;
}

CGIRunner::CGIRunner(
  const std::string_view path,
  const std::vector<std::string>& arguments) noexcept :
  cgi_program_path_(path),
  cgi_arguments_(arguments) {}

auto CGIRunner::Run() -> DynamicByteArray {
  assert(valid_);
  DynamicByteArray cgi_result;
  // unique shared filename within one CGIRunner
  // when communicating between parent and child
  const auto shared_file_name = fmt::format(
    "{cgi_prefix}{underscore}{thread_id}.txt",
    fmt::arg("cgi_prefix", kCGIPrefix),
    fmt::arg("underscore", kUnderscore),
    fmt::arg("thread_id", std::this_thread::get_id()));

  int fd = open(
    shared_file_name.c_str(),
    O_RDWR | O_APPEND | O_CREAT | O_CLOEXEC,
    kReadWritePermission);
  if (fd == -1) {
    const auto error =
      fmt::format("fail to create/open the file {}", shared_file_name);
    return {error.begin(), error.end()};
  }
  pid_t pid = fork();
  if (pid == -1) {
    constexpr std::string_view error = "fail to fork()";
    return {error.begin(), error.end()};
  }

  if (pid == 0) {
    // child

    // link cgi program's stdout to the shared file
    dup2(fd, STDOUT_FILENO);
    close(fd);

    // build argument lists for the cgi program
    char** cgi_argv = BuildArgumentList();

    // walk into cig program
    if (execve(cgi_program_path_.c_str(), cgi_argv, nullptr) < 0) {
      // only reach here when execve fails
      perror("fail to execve()");
      FreeArgumentList(cgi_argv);
      // TODO(longlp): not thread-safe
      exit(1);    // exit child process
    }
  }
  else {
    // parent
    close(fd);
    int status{};
    // wait and harvest child process
    if (waitpid(pid, &status, 0) == -1) {
      std::string error = "fail to harvest child by waitpid()";
      return {error.begin(), error.end()};
    }
    // load cgi result from the shared file
    LoadFile(shared_file_name, cgi_result);
    // clean it up by deleting shared file
    // TODO(longlp): check for return value
    std::ignore = DeleteFile(shared_file_name);
  }
  return cgi_result;
}

auto CGIRunner::IsValid() const noexcept -> bool {
  return valid_;
}

auto CGIRunner::GetPath() const noexcept -> std::string {
  return cgi_program_path_;
}

auto CGIRunner::BuildArgumentList() -> char** {
  assert(!cgi_program_path_.empty());
  char** cgi_argv = (char**)calloc(cgi_arguments_.size() + 2, sizeof(char*));
  cgi_argv[0]     = (char*)calloc(cgi_program_path_.size() + 1, sizeof(char));
  memcpy(cgi_argv[0], cgi_program_path_.c_str(), cgi_program_path_.size());
  for (size_t i = 0; i < cgi_arguments_.size(); ++i) {
    cgi_argv[i + 1] = (char*)calloc(cgi_arguments_[i].size() + 1, sizeof(char));
    memcpy(
      cgi_argv[i + 1],
      cgi_arguments_[i].c_str(),
      cgi_arguments_[i].size());
  }
  cgi_argv[cgi_arguments_.size() + 1] = nullptr;    // indicate the end of arg
                                                    // list
  return cgi_argv;
}

void CGIRunner::FreeArgumentList(char** arg_list) {
  for (int i = 0; i < static_cast<int>(cgi_arguments_.size()) + 2; ++i) {
    free(arg_list[i]);
  }
  free(arg_list);
}

}    // namespace longlp::http
