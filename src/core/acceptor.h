// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_ACCEPTOR_H_
#define CORE_ACCEPTOR_H_

#include <functional>
#include <memory>
#include <vector>

#include "base/pointers.h"
#include "core/utils.h"

namespace longlp {

class NetAddress;
class Looper;
class Connection;

// Acceptor that accepts all the incoming new client connections and set up
// customer handle functions for new clients.
// Comes with basic functionality for accepting new client connections and
// distribute its into the different Poller.
class Acceptor {
 public:
  Acceptor(
    not_null<Looper*> listener,
    std::vector<not_null<Looper*>> reactors,
    NetAddress server_address);

  ~Acceptor();

  using ConnectionCallback = std::function<void(not_null<Connection*>)>;

  DISALLOW_COPY(Acceptor);
  DEFAULT_MOVE(Acceptor);

  void SetAcceptCallback(ConnectionCallback custom_accept_callback);

  void SetHandleCallback(ConnectionCallback custom_handle_callback);

  [[nodiscard]] auto
  GetCustomAcceptCallback() const noexcept -> ConnectionCallback;

  [[nodiscard]] auto
  GetCustomHandleCallback() const noexcept -> ConnectionCallback;

  [[nodiscard]] auto GetAcceptorConnection() noexcept -> Connection*;

 private:
  // basic functionality for accepting new connection provided to the acceptor
  // by default
  void BaseAcceptCallback(Connection* server_conn);

  std::vector<not_null<Looper*>> reactors_;
  std::unique_ptr<Connection> acceptor_conn_;
  ConnectionCallback custom_accept_callback_{};
  ConnectionCallback custom_handle_callback_{};
};

}    // namespace longlp

#endif    // CORE_ACCEPTOR_H_
