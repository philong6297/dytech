// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/acceptor.h"

#include <utility>

#include <fmt/format.h>

#include "core/connection.h"
#include "core/distribution_agent.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"
#include "log/logger.h"

namespace longlp {

Acceptor::Acceptor(
  not_null<Looper*> listener,
  not_null<DistributionAgent*> agent,
  const NetAddress& server_address) :
  agent_{agent} {
  auto acceptor_socket = std::make_unique<Socket>();
  acceptor_socket->Bind(server_address, true);
  acceptor_socket->Listen();

  acceptor_connection_ =
    std::make_unique<Connection>(std::move(acceptor_socket));
  acceptor_connection_->SetEvents(Poller::Event::kRead);    // not edge-trigger
                                                            // for listener
  acceptor_connection_->SetLooper(listener);
  listener->AddAcceptor(acceptor_connection_.get());

  SetOnAccept([](Connection*) {});
  SetOnHandle([](Connection*) {});
}

Acceptor::~Acceptor() = default;

void Acceptor::SetOnAccept(ConnectionCallback on_accept_callback) {
  on_accept_cb_ = std::move(on_accept_callback);
  acceptor_connection_->SetCallback([this](not_null<Connection*> connection) {
    NetAddress client_address{};
    const auto accept_fd = connection->GetSocket()->Accept(client_address);
    if (accept_fd == -1) {
      return;
    }
    auto client_sock = std::make_unique<Socket>(accept_fd);
    client_sock->SetNonBlocking();
    auto client_connection =
      std::make_unique<Connection>(std::move(client_sock));
    client_connection
      ->SetEvents(Poller::Event::kRead | Poller::Event::kET);    // edge-trigger
                                                                 // for client
    client_connection->SetCallback(on_handle_cb_);

    auto [looper, idx] = agent_->SelectCandidate();

    Log<LogLevel::kInfo>(fmt::format(
      "new client fd={client} maps to reactor={reactor}",
      fmt::arg("client", client_connection->GetFd()),
      fmt::arg("reactor", idx)));

    client_connection->SetLooper(looper);
    looper->AddConnection(std::move(client_connection));
    on_accept_cb_(connection);
  });
}

void Acceptor::SetOnHandle(ConnectionCallback on_handle_callback) {
  on_handle_cb_ = std::move(on_handle_callback);
}

auto Acceptor::GetAcceptorConnection() noexcept -> Connection* {
  return acceptor_connection_.get();
}

}    // namespace longlp
