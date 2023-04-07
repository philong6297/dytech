// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_CONNECTION_H_
#define CORE_CONNECTION_H_

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "core/utils.h"

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

  void SetCallback(const std::function<void(Connection*)>& callback) {
    callback_ = [callback, this] {
      return callback(this);
    };
  }

  [[nodiscard]] auto GetCallback() noexcept -> std::function<void()> {
    return callback_;
  }

  // for Buffer
  [[nodiscard]] auto
  FindAndPopTill(const std::string& target) -> std::optional<std::string>;
  [[nodiscard]] auto GetReadBufferSize() const noexcept -> size_t;
  [[nodiscard]] auto GetWriteBufferSize() const noexcept -> size_t;
  void WriteToReadBuffer(const uint8_t* buf, size_t size);
  void WriteToWriteBuffer(const uint8_t* buf, size_t size);
  void WriteToReadBuffer(const std::string& str);
  void WriteToWriteBuffer(const std::string& str);
  void WriteToWriteBuffer(std::vector<uint8_t>&& other_buf);

  [[nodiscard]] auto Read() const noexcept -> const uint8_t*;
  [[nodiscard]] auto ReadAsString() const noexcept -> std::string;

  // return std::pair<How many bytes read, whether the client exists>
  [[nodiscard]] auto Recv() -> std::pair<ssize_t, bool>;
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
  std::function<void()> callback_{nullptr};
};

}    // namespace longlp
#endif    // CORE_CONNECTION_H_
