// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/connection.h"

#include <sys/socket.h>
#include <array>
#include <cstring>

#include <fmt/format.h>

#include "base/utils.h"
#include "core/buffer.h"
#include "core/socket.h"
#include "log/logger.h"

namespace {
constexpr auto kBufferSize = 2048U;
}    // namespace

namespace longlp {

Connection::Connection(std::unique_ptr<Socket> socket) :
  socket_(std::move(socket)),
  read_buffer_(std::make_unique<Buffer>()),
  write_buffer_(std::make_unique<Buffer>()) {}

Connection::~Connection() = default;

auto Connection::GetFd() const noexcept -> int {
  return socket_->GetFd();
}

auto Connection::GetSocket() noexcept -> Socket* {
  return socket_.get();
}

auto Connection::FindAndPopTill(const std::string& target)
  -> std::optional<std::string> {
  return read_buffer_->FindAndPopTill(target);
}

auto Connection::GetReadSize() const noexcept -> size_t {
  return read_buffer_->Size();
}

auto Connection::GetWriteSize() const noexcept -> size_t {
  return write_buffer_->Size();
}

void Connection::ReadUnsafe(const Byte* buf, size_t size) {
  read_buffer_->PushBackUnsafe(buf, size);
}

void Connection::WriteUnsafe(const Byte* buf, size_t size) {
  write_buffer_->PushBackUnsafe(buf, size);
}

void Connection::Read(const std::string& str) {
  read_buffer_->PushBack(str);
}

void Connection::Write(const std::string& str) {
  write_buffer_->PushBack(str);
}

void Connection::Write(DynamicByteArray&& other_buf) {
  write_buffer_->PushBack(std::move(other_buf));
}

auto Connection::ReadData() const noexcept -> const Byte* {
  return read_buffer_->Data();
}

auto Connection::ReadDataAsString() const noexcept -> std::string {
  auto str_view = read_buffer_->ToStringView();
  return {str_view.begin(), str_view.end()};
}

auto Connection::Receive() -> std::pair<ssize_t, bool> {
  // read all available bytes, since Edge-trigger
  ssize_t read = 0;
  FixedByteArray<kBufferSize + 1> buf{};
  buf.fill(0);

  while (true) {
    ssize_t curr_read = recv(GetFd(), buf.data(), kBufferSize, 0);
    if (curr_read > 0) {
      read += curr_read;
      ReadUnsafe(buf.data(), narrow_cast<size_t>(curr_read));
      buf.fill(0);
      continue;
    }

    // the client has exit
    if (curr_read == 0) {
      return {read, true};
    }

    // curr_read = -1

    // normal interrupt
    if (errno == EINTR) {
      continue;
    }

    // all data read
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      break;
    }

    Log<LogLevel::kError>(
      fmt::format("HandleConnection: Receive() error code {}", errno));
    return {read, true};
  }
  return {read, false};
}

void Connection::Send() {
  const auto to_write = narrow_cast<ssize_t>(GetWriteSize());
  const Byte* buf     = write_buffer_->Data();
  for (ssize_t curr_write = 0; curr_write < to_write;) {
    auto write = send(
      GetFd(),
      buf + curr_write,
      narrow_cast<size_t>(to_write - curr_write),
      0);
    if (write <= 0) {
      if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
        Log<LogLevel::kError>("Error in Connection::Send()");
        ClearWriteBuffer();
        return;
      }
      write = 0;
    }
    curr_write += write;
  }
  ClearWriteBuffer();
}

void Connection::ClearReadBuffer() noexcept {
  read_buffer_->Clear();
}

void Connection::ClearWriteBuffer() noexcept {
  write_buffer_->Clear();
}

void Connection::Start() {
  callback_(this);
}

}    // namespace longlp
