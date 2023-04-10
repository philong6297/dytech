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
#include <fmt/std.h>

#include "base/pointers.h"
#include "http/constants.h"
#include "http/http_utils.h"

namespace longlp::http {

namespace {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"

auto BuildArgv(
  const std::vector<std::string>& cgi_arguments,
  const std::string_view cgi_program_path) -> owner<char**> {
  // build argument lists for the cgi program
  assert(!cgi_program_path.empty());

  // argv = path + program args + null indicated arg
  auto** argv =
    static_cast<owner<char**>>(new char*[cgi_arguments.size() + 2U]);

  argv[0] = static_cast<owner<char*>>(new char[cgi_program_path.size() + 1U]);
  std::memcpy(argv[0], cgi_program_path.data(), cgi_program_path.size());
  argv[0][cgi_program_path.size()] = 0;

  for (auto i = 0U; i < cgi_arguments.size(); ++i) {
    argv[i + 1] =
      static_cast<owner<char*>>(new char[cgi_arguments[i].size() + 1]);
    std::memcpy(argv[i + 1], cgi_arguments[i].c_str(), cgi_arguments[i].size());
    argv[i + 1][cgi_arguments[i].size()] = 0;
  }

  // indicate the end of arg list
  argv[cgi_arguments.size() + 1] = nullptr;

  return argv;
}

void DeleteArgv(owner<char**> argv, size_t size) {
  for (auto i = 0U; i < size; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete argv[i];
  }
  delete argv;
}

#pragma GCC diagnostic pop

}    // namespace

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

    owner<char**> argv = BuildArgv(cgi_arguments_, cgi_program_path_);

    // walk into cgi program
    if (execve(cgi_program_path_.c_str(), argv, nullptr) < 0) {
      // only reach here when execve fails
      perror("fail to execve()");
      DeleteArgv(argv, cgi_arguments_.size() + 2U);
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

}    // namespace longlp::http
