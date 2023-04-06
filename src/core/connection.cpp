// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/connection.h"

#include <sys/socket.h>
#include <cstring>

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

auto Connection::GetReadBufferSize() const noexcept -> size_t {
  return read_buffer_->Size();
}

auto Connection::GetWriteBufferSize() const noexcept -> size_t {
  return write_buffer_->Size();
}

void Connection::WriteToReadBuffer(const uint8_t* buf, size_t size) {
  read_buffer_->AppendUnsafe(buf, size);
}

void Connection::WriteToWriteBuffer(const uint8_t* buf, size_t size) {
  write_buffer_->AppendUnsafe(buf, size);
}

void Connection::WriteToReadBuffer(const std::string& str) {
  read_buffer_->Append(str);
}

void Connection::WriteToWriteBuffer(const std::string& str) {
  write_buffer_->Append(str);
}

void Connection::WriteToWriteBuffer(std::vector<uint8_t>&& other_buf) {
  write_buffer_->Append(std::move(other_buf));
}

auto Connection::Read() const noexcept -> const uint8_t* {
  return read_buffer_->Data();
}

auto Connection::ReadAsString() const noexcept -> std::string {
  auto str_view = read_buffer_->ToStringView();
  return {str_view.begin(), str_view.end()};
}

auto Connection::Recv() -> std::pair<ssize_t, bool> {
  // read all available bytes, since Edge-trigger
  int from_fd  = GetFd();
  ssize_t read = 0;
  uint8_t buf[kBufferSize + 1];
  memset(buf, 0, sizeof(buf));
  while (true) {
    ssize_t curr_read = recv(from_fd, buf, kBufferSize, 0);
    if (curr_read > 0) {
      read += curr_read;
      WriteToReadBuffer(buf, curr_read);
      memset(buf, 0, sizeof(buf));
    }
    else if (curr_read == 0) {
      // the client has exit
      return {read, true};
    }
    else if (curr_read == -1 && errno == EINTR) {
      // normal interrupt
      continue;
    }
    else if (curr_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // all data read
      break;
    }
    else {
      Log<LogLevel::ERROR>("HandleConnection: recv() error");
      return {read, true};
    }
  }
  return {read, false};
}

void Connection::Send() {
  // robust write
  ssize_t curr_write = 0;
  ssize_t write;
  const ssize_t to_write = GetWriteBufferSize();
  const uint8_t* buf     = write_buffer_->Data();
  while (curr_write < to_write) {
    write = send(GetFd(), buf + curr_write, to_write - curr_write, 0);
    if (write <= 0) {
      if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
        Log<LogLevel::ERROR>("Error in Connection::Send()");
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

}    // namespace longlp
