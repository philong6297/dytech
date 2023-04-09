// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_SOCKET_H_
#define SRC_CORE_SOCKET_H_

#include "base/macros.h"

#include <cstdint>

namespace longlp {

class NetAddress;

// Socket class encapsulates a socket descriptor which can act as either
// listener or client.
// IPv4/6-Compatible
class Socket {
 public:
  Socket() noexcept;

  explicit Socket(int fd) noexcept;

  DISALLOW_COPY(Socket);

  Socket(Socket&& other) noexcept;

  auto operator=(Socket&& other) noexcept -> Socket&;

  ~Socket();

  [[nodiscard]] auto GetFd() const noexcept -> int;

  // for client, one step: directly connect
  void Connect(const NetAddress& server_address);

  // for server, three steps: bind + listen + accept
  void Bind(const NetAddress& server_address, bool is_reusable);

  void Listen() const;

  [[nodiscard]] auto Accept(NetAddress& client_address) const -> int;

  void SetReusable() const;

  void SetNonBlocking() const;

  [[nodiscard]] auto GetAttrs() const -> uint64_t;

 private:
  int fd_{-1};
};
}    // namespace longlp

#endif    // SRC_CORE_SOCKET_H_
