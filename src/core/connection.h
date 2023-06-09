// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_CONNECTION_H_
#define SRC_CORE_CONNECTION_H_

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "core/typedefs.h"

namespace longlp {

class Looper;
class Buffer;
class Socket;

// This Connection class encapsulates a TCP client connection
// It could be set a custom callback function when new messages arrive and it
// contains information about the monitoring events and return events so that
// Poller could manipulate and epoll based on this Connection class
class Connection {
 public:
  explicit Connection(std::unique_ptr<Socket> socket);
  ~Connection();

  DISALLOW_COPY(Connection);
  DEFAULT_MOVE(Connection);

  [[nodiscard]] auto GetFd() const noexcept -> int;

  [[nodiscard]] auto GetSocket() noexcept -> Socket*;

  // for Poller

  void SetEvents(uint32_t events) { events_ = events; }

  [[nodiscard]] auto GetEvents() const noexcept -> uint32_t { return events_; }

  void SetRevents(uint32_t revents) { revents_ = revents; }

  [[nodiscard]] auto GetRevents() const noexcept -> uint32_t {
    return revents_;
  }

  void SetCallback(const ConnectionCallback& callback) { callback_ = callback; }

  void Start();

  // for Buffer
  [[nodiscard]] auto
  FindAndPopTill(const std::string& target) -> std::optional<std::string>;
  [[nodiscard]] auto GetReadSize() const noexcept -> size_t;
  [[nodiscard]] auto GetWriteSize() const noexcept -> size_t;
  void ReadUnsafe(const Byte* buf, size_t size);
  void WriteUnsafe(const Byte* buf, size_t size);
  void Read(const std::string& str);
  void Write(const std::string& str);
  void Write(DynamicByteArray&& other_buf);

  [[nodiscard]] auto ReadData() const noexcept -> const Byte*;
  [[nodiscard]] auto ReadDataAsString() const noexcept -> std::string;

  // return std::pair<How many bytes read, whether the client exists>
  [[nodiscard]] auto Receive() -> std::pair<ssize_t, bool>;
  void Send();
  void ClearReadBuffer() noexcept;
  void ClearWriteBuffer() noexcept;

  void SetLooper(Looper* looper) noexcept { owner_looper_ = looper; }

  [[nodiscard]] auto GetLooper() noexcept -> Looper* { return owner_looper_; }

 private:
  Looper* owner_looper_{nullptr};
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Buffer> read_buffer_;
  std::unique_ptr<Buffer> write_buffer_;
  uint32_t events_{0};
  uint32_t revents_{0};
  ConnectionCallback callback_{};
};

}    // namespace longlp
#endif    // SRC_CORE_CONNECTION_H_
