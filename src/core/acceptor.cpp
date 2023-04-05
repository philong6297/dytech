// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/acceptor.h"

#include <cstdlib>
#include <utility>

#include "core/connection.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"
#include "log/logger.h"

namespace longlp {

  Acceptor::Acceptor(
    Looper* listener,
    std::vector<Looper*> reactors,
    NetAddress server_address) :
    reactors_(std::move(reactors)) {
    auto acceptor_sock = std::make_unique<Socket>();
    acceptor_sock->Bind(server_address, true);
    acceptor_sock->Listen();
    acceptor_conn = std::make_unique<Connection>(std::move(acceptor_sock));
    acceptor_conn->SetEvents(POLL_READ);    // not edge-trigger for listener
    acceptor_conn->SetLooper(listener);
    listener->AddAcceptor(acceptor_conn.get());
    SetCustomAcceptCallback([](Connection*) {});
    SetCustomHandleCallback([](Connection*) {});
  }

  void Acceptor::BaseAcceptCallback(Connection* server_conn) {
    NetAddress client_address;
    int accept_fd = server_conn->GetSocket()->Accept(client_address);
    if (accept_fd == -1) {
      return;
    }
    auto client_sock = std::make_unique<Socket>(accept_fd);
    client_sock->SetNonBlocking();
    auto client_connection =
      std::make_unique<Connection>(std::move(client_sock));
    client_connection->SetEvents(POLL_READ | POLL_ET);    // edge-trigger for
                                                          // client
    client_connection->SetCallback(GetCustomHandleCallback());
    // randomized distribution. uniform in long term.
    size_t idx = static_cast<size_t>(rand()) % reactors_.size();
    LOG_INFO(
      "new client fd=" + std::to_string(client_connection->GetFd()) +
      " maps to reactor " + std::to_string(idx));
    client_connection->SetLooper(reactors_[idx]);
    reactors_[idx]->AddConnection(std::move(client_connection));
  }

  void Acceptor::SetCustomAcceptCallback(
    std::function<void(Connection*)> custom_accept_callback) {
    custom_accept_callback_ = std::move(custom_accept_callback);
    acceptor_conn->SetCallback([this](auto&& PH1) {
      BaseAcceptCallback(std::forward<decltype(PH1)>(PH1));
      custom_accept_callback_(std::forward<decltype(PH1)>(PH1));
    });
  }

  void Acceptor::SetCustomHandleCallback(
    std::function<void(Connection*)> custom_handle_callback) {
    custom_handle_callback_ = std::move(custom_handle_callback);
  }

  auto Acceptor::GetCustomAcceptCallback() const noexcept
    -> std::function<void(Connection*)> {
    return custom_accept_callback_;
  }

  auto Acceptor::GetCustomHandleCallback() const noexcept
    -> std::function<void(Connection*)> {
    return custom_handle_callback_;
  }

  auto Acceptor::GetAcceptorConnection() noexcept -> Connection* {
    return acceptor_conn.get();
  }

}    // namespace longlp
