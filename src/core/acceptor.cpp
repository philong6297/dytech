// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/acceptor.h"

#include <atomic>
#include <cstdlib>
#include <random>
#include <thread>
#include <utility>

#include <fmt/format.h>

#include "core/connection.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"
#include "log/logger.h"

namespace longlp {

namespace {
// lock-free with atomic thread_local
auto GenerateRandomNumber(size_t upper_bound) -> size_t {
  // Create thread-local seeds
  const thread_local std::atomic<std::random_device::result_type>
    seed(std::random_device{}());
  thread_local std::mt19937 gen(seed);

  // Create a uniform integer distribution within the range
  std::uniform_int_distribution<size_t> distr(0, upper_bound);

  // Generate a random number within the range
  return distr(gen);
}
}    // namespace

Acceptor::Acceptor(
  not_null<Looper*> listener,
  std::vector<not_null<Looper*>> reactors,
  NetAddress server_address) :
  reactors_(std::move(reactors)) {
  auto acceptor_sock = std::make_unique<Socket>();
  acceptor_sock->Bind(server_address, true);
  acceptor_sock->Listen();

  acceptor_conn_ = std::make_unique<Connection>(std::move(acceptor_sock));
  acceptor_conn_->SetEvents(Poller::Event::kRead);    // not edge-trigger for
                                                      // listener
  acceptor_conn_->SetLooper(listener);
  listener->AddAcceptor(acceptor_conn_.get());
  SetAcceptCallback([](Connection*) {});
  SetHandleCallback([](Connection*) {});
}

Acceptor::~Acceptor() = default;

void Acceptor::BaseAcceptCallback(Connection* server_conn) {
  NetAddress client_address{};
  int accept_fd = server_conn->GetSocket()->Accept(client_address);
  if (accept_fd == -1) {
    return;
  }
  auto client_sock = std::make_unique<Socket>(accept_fd);
  client_sock->SetNonBlocking();
  auto client_connection = std::make_unique<Connection>(std::move(client_sock));
  client_connection
    ->SetEvents(Poller::Event::kRead | Poller::Event::kET);    // edge-trigger
                                                               // for client
  client_connection->SetCallback(GetCustomHandleCallback());
  // randomized distribution. uniform in long term.
  const auto idx = GenerateRandomNumber(reactors_.size() - 1);
  Log<LogLevel::kInfo>(fmt::format(
    "new client fd={client} maps to reactor={reactor}",
    fmt::arg("client", client_connection->GetFd()),
    fmt::arg("reactor", idx)));

  client_connection->SetLooper(reactors_[idx]);
  reactors_[idx]->AddConnection(std::move(client_connection));
}

void Acceptor::SetAcceptCallback(ConnectionCallback custom_accept_callback) {
  custom_accept_callback_ = std::move(custom_accept_callback);
  acceptor_conn_->SetCallback([this](not_null<Connection*> connection) {
    BaseAcceptCallback(connection);
    custom_accept_callback_(connection);
  });
}

void Acceptor::SetHandleCallback(ConnectionCallback custom_handle_callback) {
  custom_handle_callback_ = std::move(custom_handle_callback);
}

auto Acceptor::GetCustomAcceptCallback() const noexcept -> ConnectionCallback {
  return custom_accept_callback_;
}

auto Acceptor::GetCustomHandleCallback() const noexcept -> ConnectionCallback {
  return custom_handle_callback_;
}

auto Acceptor::GetAcceptorConnection() noexcept -> Connection* {
  return acceptor_conn_.get();
}

}    // namespace longlp
