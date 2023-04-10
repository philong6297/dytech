// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_SERVER_H_
#define SRC_CORE_SERVER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "core/typedefs.h"

namespace longlp {

class NetAddress;
class Connection;
class Acceptor;
class Looper;
class ThreadPool;
class DistributionAgent;

// The class for setting up a web server using the framework
// User should provide the callback functions in OnAccept() and OnHandle()
// The rest is already taken care of and in most cases users don't need to touch
// upon
class Server {
 public:
  Server(const NetAddress& server_address, int64_t num_threads);

  virtual ~Server();

  DISALLOW_COPY(Server);
  DEFAULT_MOVE(Server);

  // Not Edge trigger
  // Given the acceptor connection, when the Poller tells us there is new
  // incoming client connection basic step of socket accept and build connection
  // and add into the Poller are already taken care of in the
  // Acceptor::BaseAcceptCallback. This OnAccept() functionality is appended to
  // that base BaseAcceptCallback and called after that base, to support any
  // custom business logic upon receiving new client connection
  [[nodiscard]] auto OnAccept(ConnectionCallback on_accept) -> Server&;

  // Edge trigger! Read all bytes please
  // OnHandle(): No base version exists. Users should implement provide a
  // function to achieve the expected behavior
  [[nodiscard]] auto OnHandle(ConnectionCallback on_handle) -> Server&;

  void Begin();

 private:
  bool on_handle_set_{false};
  std::unique_ptr<Acceptor> acceptor_;
  std::vector<std::unique_ptr<Looper>> reactors_;
  std::unique_ptr<DistributionAgent> agent_;
  std::unique_ptr<ThreadPool> pool_;
  std::unique_ptr<Looper> listener_;
};
}    // namespace longlp

#endif    // SRC_CORE_SERVER_H_
