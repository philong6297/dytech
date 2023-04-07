// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/connection.h"

#include <unistd.h>
#include <cstring>
#include <memory>
#include <string>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"

using longlp::Connection;
using longlp::NetAddress;
using longlp::Poller;
using longlp::Protocol;
using longlp::Socket;

TEST_CASE("[core/connection]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);
  auto server_sock = std::make_unique<Socket>();
  server_sock->Bind(local_host);
  server_sock->Listen();
  Connection server_conn(std::move(server_sock));
  REQUIRE(server_conn.GetSocket() != nullptr);

  SECTION("connection set events and return events") {
    server_conn.SetEvents(Poller::Event::kAdd | Poller::Event::kET);
    CHECK((server_conn.GetEvents() & Poller::Event::kAdd));
    CHECK((server_conn.GetEvents() & Poller::Event::kET));
    server_conn.SetRevents(Poller::Event::kRead);
    CHECK((server_conn.GetRevents() & Poller::Event::kRead));
  }

  SECTION("connection's callback setup and invoke") {
    server_conn.SetCallback([](Connection*) -> void {});
    int i = 0;
    server_conn.SetCallback([&target = i](Connection*) -> void {
      target += 1;
    });
    server_conn.GetCallback()();
    CHECK(i == 1);
  }

  SECTION("through connection to send and recv messages") {
    const char* client_message = "hello from client";
    const char* server_message = "hello from server";
    std::thread client_thread([&]() {
      // build a client connecting with server
      auto client_sock = std::make_unique<Socket>();
      client_sock->Connect(local_host);
      Connection client_conn(std::move(client_sock));
      // send a message to server
      client_conn.WriteToWriteBuffer(client_message);
      CHECK(client_conn.GetWriteBufferSize() == strlen(client_message));
      client_conn.Send();
      // recv a message from server
      sleep(1);
      auto [read, exit] = client_conn.Recv();
      CHECK((read == strlen(server_message) && exit));
      CHECK(client_conn.ReadAsString() == std::string(server_message));
    });

    client_thread.detach();
    NetAddress client_address;
    auto connected_sock =
      std::make_unique<Socket>(server_conn.GetSocket()->Accept(client_address));
    connected_sock->SetNonBlocking();
    CHECK(connected_sock->GetFd() != -1);
    Connection connected_conn(std::move(connected_sock));
    sleep(1);
    // recv a message from client
    auto [read, exit] = connected_conn.Recv();
    CHECK((read == strlen(client_message) && !exit));
    CHECK(connected_conn.GetReadBufferSize() == strlen(client_message));
    // send a message to client
    connected_conn.WriteToWriteBuffer(server_message);
    connected_conn.Send();
    sleep(1);
  }
}
