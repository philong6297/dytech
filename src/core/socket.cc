// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/socket.h"

#include <fcntl.h>
#include <sys/socket.h>

#include <cassert>
#include <stdexcept>

#include "base/utils.h"
#include "core/net_address.h"
#include "log/logger.h"

#include <fmt/format.h>

namespace longlp {

namespace {
constexpr int kBackLog = 128;

auto CreateSocket(Protocol protocol) -> int {
  int fd_{};
  switch (protocol) {
    case Protocol::Ipv4:
      fd_ = socket(AF_INET, SOCK_STREAM, 0);
      break;
    case Protocol::Ipv6:
      fd_ = socket(AF_INET6, SOCK_STREAM, 0);
      break;
  }

  if (fd_ == -1) {
    Log<LogLevel::kError>("Socket: socket() error");
    throw std::logic_error("Socket: socket() error");
  }
  return fd_;
}
}    // namespace

Socket::Socket() noexcept = default;

Socket::Socket(int fd) noexcept :
  fd_(fd) {}

Socket::Socket(Socket&& other) noexcept :
  fd_(other.fd_) {
  other.fd_ = -1;
}

auto Socket::operator=(Socket&& other) noexcept -> Socket& {
  if (fd_ != -1) {
    close(fd_);
  }
  std::swap(fd_, other.fd_);
  return *this;
}

Socket::~Socket() {
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

auto Socket::GetFd() const noexcept -> int {
  return fd_;
}

void Socket::Connect(const NetAddress& server_address) {
  if (fd_ == -1) {
    fd_ = CreateSocket(server_address.GetProtocol());
  }
  if (
    connect(
      fd_,
      server_address.address_data(),
      *server_address.address_data_length()) == -1) {
    Log<LogLevel::kError>("Socket: Connect() error");
    throw std::logic_error("Socket: Connect() error");
  }
}

void Socket::Bind(const NetAddress& server_address, bool is_reusable) {
  if (fd_ == -1) {
    fd_ = CreateSocket(server_address.GetProtocol());
  }
  if (is_reusable) {
    SetReusable();
  }
  if (
    bind(
      fd_,
      server_address.address_data(),
      *server_address.address_data_length()) == -1) {
    Log<LogLevel::kError>("Socket: Bind() error");
    throw std::logic_error("Socket: Bind() error");
  }
}

void Socket::Listen() const {
  assert(fd_ != -1 && "cannot Listen() with an invalid fd");
  if (listen(fd_, kBackLog) == -1) {
    Log<LogLevel::kError>("Socket: Listen() error");
    throw std::logic_error("Socket: Listen() error");
  }
}

auto Socket::Accept(NetAddress& client_address) const -> int {
  assert(fd_ != -1 && "cannot Accept() with an invalid fd");
  // use accept4() with the SOCK_NONBLOCK flag, it sets the accepted socket file
  // descriptor to non-blocking mode for its future operations. However, it does
  // not affect the original listening socket file descriptor, which can still
  // be configured to block or non-block mode using fcntl().
  // TODO(longlp): Try to reduce fcntl calls and Check if socket is the same in
  // acceptor.cc
  int client_fd = accept4(
    fd_,
    client_address.address_data(),
    client_address.address_data_length(),
    SOCK_CLOEXEC | SOCK_NONBLOCK);
  if (client_fd == -1) {
    // under high pressure, accept might fail.
    // but server should not fail at this time
    Log<LogLevel::kWarning>("Socket: Accept() error");
  }
  return client_fd;
}

void Socket::SetReusable() const {
  assert(fd_ != -1 && "cannot SetReusable() with an invalid fd");
  int yes = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1 ||
      setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes) == -1) {
    Log<LogLevel::kError>("Socket: SetReusable() error");
    throw std::logic_error("Socket: SetReusable() error");
  }
}

void Socket::SetNonBlocking() const {
  assert(fd_ != -1 && "cannot SetNonBlocking() with an invalid fd");

  // don't try to call fcntl if it is already non-blocking
  const auto current_attributes = GetAttrs();
  if ((current_attributes & O_NONBLOCK) != 0) {
    return;
  }

  if (fcntl(fd_, F_SETFL, current_attributes | O_NONBLOCK) == -1) {
    Log<LogLevel::kError>("Socket: SetNonBlocking() error");
    throw std::logic_error("Socket: SetNonBlocking() error");
  }
}

auto Socket::GetAttrs() const -> uint64_t {
  assert(fd_ != -1 && "cannot GetAttrs() with an invalid fd");

  // TODO(longlp): in rare cases, fcntl() with F_GETFL may return a negative
  // value, which indicates an error in the system call. The value could be -1.
  const auto result = fcntl(fd_, F_GETFL);
  if (result == -1) {
    Log<LogLevel::kError>("Socket: GetAttrs() error");
    throw std::logic_error("Socket: GetAttrs() error");
  }

  return narrow_cast<uint64_t>(result);
}

}    // namespace longlp
