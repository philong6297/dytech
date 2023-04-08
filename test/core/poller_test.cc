// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/poller.h"

#include <unistd.h>

#include <memory>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include "core/connection.h"
#include "core/net_address.h"
#include "core/socket.h"

using longlp::Connection;
using longlp::NetAddress;
using longlp::Poller;
using longlp::Protocol;
using longlp::Socket;
using longlp::Poller::Event::kAdd;
using longlp::Poller::Event::kET;
using longlp::Poller::Event::kRead;

TEST_CASE("[core/poller]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);
  Socket server_sock;

  // build the server socket
  server_sock.Bind(local_host);
  server_sock.Listen();
  REQUIRE(server_sock.GetFd() != -1);

  int client_num = 3;
  // build the empty poller
  Poller poller(client_num);
  REQUIRE(poller.GetPollSize() == client_num);

  SECTION("able to poll out the client's messages sent over") {
    std::vector<std::thread> threads;
    for (int i = 0; i < client_num; i++) {
      threads.emplace_back([&]() {
        auto client_socket = Socket();
        client_socket.Connect(local_host);
        char message[] = "Hello from client!";
        send(client_socket.GetFd(), message, strlen(message), 0);
        sleep(2);
      });
    }

    // server accept clients and build connection for them
    std::vector<std::shared_ptr<Connection>> client_conns;
    for (int i = 0; i < client_num; i++) {
      NetAddress client_address;
      auto client_sock =
        std::make_unique<Socket>(server_sock.Accept(client_address));
      CHECK(client_sock->GetFd() != -1);
      client_conns
        .push_back(std::make_shared<Connection>(std::move(client_sock)));
      client_conns[i]->SetEvents(Poller::Event::kRead);
    }

    // each client connection under poller's monitor
    for (int i = 0; i < client_num; i++) {
      poller.AddConnection(client_conns[i].get());
    }
    sleep(1);
    auto ready_conns = poller.Poll(Poller::kBlockForever);
    CHECK(ready_conns.size() == client_num);

    for (int i = 0; i < client_num; i++) {
      threads[i].join();
    }
  }
}
