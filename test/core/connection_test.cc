// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/connection.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <fmt/format.h>
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
  NetAddress server_address("127.0.0.1", 20080, Protocol::Ipv4);
  auto server_socket = std::make_unique<Socket>();
  server_socket->BindWithServerAddress(server_address, true);
  server_socket->StartListeningIncomingConnection();
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

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> client_connection_exists{false};
    std::atomic<bool> client_msg_sent{false};
    std::atomic<bool> server_msg_sent{false};
    std::atomic<bool> client_finished{false};

    std::thread client_thread([&]() {
      // build a client connecting with server
      auto client_socket = std::make_unique<Socket>();
      client_socket->ConnectToServer(server_address);
      client_socket->SetNonBlocking();
      Connection client_connection(std::move(client_socket));

      {
        client_connection_exists = true;
        cv.notify_all();
      }

      // send a message to server
      client_connection.WriteToWriteBuffer(client_message);
      CHECK(client_connection.GetWriteBufferSize() == client_message.size());
      client_connection.Send();
      fmt::print("client: client msg sent\n");
      {
        client_msg_sent = true;
        cv.notify_all();
      }

      // wait for server msg
      {
        std::unique_lock<std::mutex> lock(mtx);
        fmt::print("client: wait for server msg\n");
        cv.wait(lock, [&]() -> bool { return server_msg_sent; });
      }

      // recv a message from server
      auto [read, exit] = client_connection.Recv();
      CHECK(read == std::ssize(server_message));
      // the server should not exit yet due to sleep
      CHECK(!exit);
      CHECK(client_connection.ReadAsString() == std::string(server_message));
      fmt::print("client: client finished\n");
      {
        client_finished = true;
        cv.notify_all();
      }
    });

    // make sure client connection exists so the accept operation wont block
    {
      std::unique_lock<std::mutex> lock(mtx);
      fmt::print("server: wait for client connection\n");
      cv.wait(lock, [&]() -> bool { return client_connection_exists; });
    }

    NetAddress client_address;
    auto connected_sock = std::make_unique<Socket>(
      server_connection.GetSocket()->AcceptClientAddress(client_address));
    connected_sock->SetNonBlocking();
    CHECK(connected_sock->GetFd() != -1);
    Connection connected_conn(std::move(connected_sock));

    // wait for client msg
    {
      std::unique_lock<std::mutex> lock(mtx);
      fmt::print("server: wait for client msg\n");
      cv.wait(lock, [&]() -> bool { return client_msg_sent; });
    }

    fmt::print("server: client msg sent\n");

    // recv a message from client
    auto [read, exit] = connected_conn.Recv();
    CHECK(read == std::ssize(client_message));
    // the client is still wait for listening the message from server
    // it should not be exited yet
    CHECK(!exit);
    CHECK(connected_conn.GetReadBufferSize() == client_message.size());

    // send a message to client
    connected_conn.WriteToWriteBuffer(server_message);
    connected_conn.Send();
    fmt::print("server: server msg sent\n");
    {
      server_msg_sent = true;
      cv.notify_all();
    }

    // wait for client finish before exit
    {
      std::unique_lock<std::mutex> lock(mtx);
      fmt::print("server: wait for client finish\n");
      cv.wait(lock, [&]() -> bool { return client_finished; });
    }

    client_thread.join();
  }
}
