// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/socket.h"

#include <fcntl.h>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "core/net_address.h"

namespace {
using longlp::NetAddress;
using longlp::Protocol;
using longlp::Socket;
}    // namespace

TEST_CASE("[core/socket]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);
  Socket server_socket;

  // build the server socket
  server_socket.BindWithServerAddress(local_host, true);
  REQUIRE(server_socket.GetFd() != -1);
  server_socket.StartListeningIncomingConnection();

  SECTION("move operations will switch fd") {
    const auto orig_fd = server_socket.GetFd();
    Socket another_sock(123);
    server_socket = std::move(another_sock);
    CHECK(server_socket.GetFd() == 123);
    CHECK(another_sock.GetFd() == orig_fd);

    Socket second_sock = std::move(another_sock);
    CHECK(second_sock.GetFd() == orig_fd);
    CHECK(another_sock.GetFd() == -1);
  }

  SECTION("non-blocking mode setting for socket") {
    Socket sock;
    sock.BindWithServerAddress(local_host, true);
    CHECK((sock.GetAttrs() & O_NONBLOCK) == 0);
    sock.SetNonBlocking();
    CHECK((sock.GetAttrs() & O_NONBLOCK) != 0);
  }

  SECTION("interaction between server and client socket") {
    NetAddress client_address;
    std::thread client_thread = std::thread([&]() {
      Socket client_socket;
      client_socket.ConnectToServer(local_host);
    });
    client_thread.detach();
    CHECK(server_socket.AcceptClientAddress(client_address) != -1);
  }
}
