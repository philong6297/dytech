// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_COREACCEPTOR_H_
#define SRC_COREACCEPTOR_H_

#include <functional>
#include <memory>

#include "base/macros.h"
#include "base/pointers.h"
#include "core/typedefs.h"

namespace longlp {

class NetAddress;
class Looper;
class Connection;
class DistributionAgent;

// Acceptor that accepts all the incoming new client connections and set up
// customer handle functions for new clients.
// Comes with basic functionality for accepting new client connections and
// distribute its into the different Poller.
class Acceptor {
 public:
  Acceptor(
    not_null<Looper*> listener,
    not_null<DistributionAgent*> agent,
    const NetAddress& server_address);

  ~Acceptor();

  DISALLOW_COPY(Acceptor);
  DEFAULT_MOVE(Acceptor);

  void SetOnAccept(ConnectionCallback on_accept_callback);

  void SetOnHandle(ConnectionCallback on_handle_callback);

  [[nodiscard]] auto GetAcceptorConnection() noexcept -> Connection*;

 private:
  std::unique_ptr<Connection> acceptor_connection_;
  not_null<DistributionAgent*> agent_;
  ConnectionCallback on_accept_cb_{};
  ConnectionCallback on_handle_cb_{};
};

}    // namespace longlp

#endif    // SRC_COREACCEPTOR_H_
