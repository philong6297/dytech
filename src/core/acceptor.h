// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_ACCEPTOR_H_
#define CORE_ACCEPTOR_H_

#include <functional>
#include <memory>
#include <vector>

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
    Looper* listener,
    std::vector<Looper*> reactors,
    NetAddress server_address);

  ~Acceptor();

  DISALLOW_COPY(Acceptor);
  DEFAULT_MOVE(Acceptor);

  // basic functionality for accepting new connection provided to the acceptor
  // by default
  void BaseAcceptCallback(Connection* server_conn);

  void SetCustomAcceptCallback(
    std::function<void(Connection*)> custom_accept_callback);

  void SetCustomHandleCallback(
    std::function<void(Connection*)> custom_handle_callback);

  [[nodiscard]] auto
  GetCustomAcceptCallback() const noexcept -> std::function<void(Connection*)>;

  [[nodiscard]] auto
  GetCustomHandleCallback() const noexcept -> std::function<void(Connection*)>;

  [[nodiscard]] auto GetAcceptorConnection() noexcept -> Connection*;

 private:
  std::vector<Looper*> reactors_;
  std::unique_ptr<Connection> acceptor_conn;
  std::function<void(Connection*)> custom_accept_callback_{};
  std::function<void(Connection*)> custom_handle_callback_{};
};

}    // namespace longlp

#endif    // CORE_ACCEPTOR_H_
