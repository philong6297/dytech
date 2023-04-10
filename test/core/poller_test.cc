// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/poller.h"

#include <chrono>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "core/connection.h"
#include "core/net_address.h"
#include "core/socket.h"

namespace {
using longlp::Connection;
using longlp::NetAddress;
using longlp::Poller;
using longlp::Protocol;
using longlp::Socket;
using namespace std::chrono_literals;
}    // namespace

TEST_CASE("[core/poller]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);
  Socket server_socket;

  // build the server socket
  server_socket.BindWithServerAddress(local_host, true);
  server_socket.StartListeningIncomingConnection();
  REQUIRE(server_socket.GetFd() != -1);

  constexpr auto client_num = 3U;
  // build the empty poller
  Poller poller(client_num);
  REQUIRE(poller.GetPollSize() == client_num);

  SECTION("able to poll out the client's messages sent over") {
    std::vector<std::thread> threads;
    threads.reserve(client_num);
    for (auto i = 0U; i < client_num; ++i) {
      threads.emplace_back([&]() {
        auto client_socket = Socket();
        client_socket.ConnectToServer(local_host);
        const std::string_view message = "Hello from client!";
        send(client_socket.GetFd(), message.data(), message.size(), 0);
        std::this_thread::sleep_for(2s);
      });
    }

    // server accept clients and build connection for them
    std::vector<std::shared_ptr<Connection>> client_conns;
    for (auto i = 0U; i < client_num; ++i) {
      NetAddress client_address;
      auto client_socket = std::make_unique<Socket>(
        server_socket.AcceptClientAddress(client_address));
      CHECK(client_socket->GetFd() != -1);
      client_conns
        .push_back(std::make_shared<Connection>(std::move(client_socket)));
      client_conns[i]->SetEvents(Poller::Event::kRead);
    }

    // each client connection under poller's monitor
    for (auto i = 0U; i < client_num; ++i) {
      poller.AddConnection(client_conns[i].get());
    }
    std::this_thread::sleep_for(2s);
    auto ready_conns = poller.Poll(Poller::kBlockForever);
    CHECK(ready_conns.size() == client_num);

    for (auto i = 0U; i < client_num; ++i) {
      threads[i].join();
    }
  }
}
