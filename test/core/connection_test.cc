// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/connection.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"

namespace {
using longlp::Connection;
using longlp::NetAddress;
using longlp::Poller;
using longlp::Protocol;
using longlp::Socket;
using namespace std::chrono_literals;
}    // namespace

TEST_CASE("[core/connection]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);
  auto server_socket = std::make_unique<Socket>();
  server_socket->Bind(local_host, true);
  server_socket->Listen();
  Connection server_connection(std::move(server_socket));
  REQUIRE(server_connection.GetSocket() != nullptr);

  SECTION("connection set events and return events") {
    server_connection.SetEvents(Poller::Event::kAdd | Poller::Event::kET);
    CHECK((server_connection.GetEvents() & Poller::Event::kAdd));
    CHECK((server_connection.GetEvents() & Poller::Event::kET));
    server_connection.SetRevents(Poller::Event::kRead);
    CHECK((server_connection.GetRevents() & Poller::Event::kRead));
  }

  SECTION("connection's callback setup and invoke") {
    server_connection.SetCallback([](Connection*) -> void {});
    int32_t i = 0;
    server_connection.SetCallback([&target = i](Connection*) -> void {
      target += 1;
    });
    server_connection.Start();
    CHECK(i == 1);
  }

  SECTION("through connection to send and recv messages") {
    const std::string client_message = "hello from client";
    const std::string server_message = "hello from server";
    std::thread client_thread([&]() {
      // build a client connecting with server
      auto client_sock = std::make_unique<Socket>();
      client_sock->Connect(local_host);
      Connection client_conn(std::move(client_sock));
      // send a message to server
      client_conn.WriteToWriteBuffer(client_message);
      CHECK(client_conn.GetWriteBufferSize() == client_message.size());
      client_conn.Send();
      // recv a message from server
      // 5s is long enough to wait for server setup and send message
      std::this_thread::sleep_for(5s);
      auto [read, exit] = client_conn.Recv();
      CHECK((read == std::ssize(server_message) && exit));
      CHECK(client_conn.ReadAsString() == std::string(server_message));
    });

    client_thread.detach();

    NetAddress client_address;
    auto connected_sock = std::make_unique<Socket>(
      server_connection.GetSocket()->Accept(client_address));
    connected_sock->SetNonBlocking();
    CHECK(connected_sock->GetFd() != -1);
    Connection connected_conn(std::move(connected_sock));
    std::this_thread::sleep_for(1s);
    // recv a message from client
    auto [read, exit] = connected_conn.Recv();
    CHECK((read == std::ssize(client_message) && !exit));
    CHECK(connected_conn.GetReadBufferSize() == client_message.size());
    // send a message to client
    connected_conn.WriteToWriteBuffer(server_message);
    connected_conn.Send();
    std::this_thread::sleep_for(1s);
  }
}
