// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/looper.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "base/utils.h"
#include "core/connection.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"

namespace {
using longlp::Connection;
using longlp::Looper;
using longlp::NetAddress;
using longlp::Poller;
using longlp::Protocol;
using longlp::Socket;
using namespace std::chrono_literals;
}    // namespace

TEST_CASE("[core/looper]") {
  Looper looper;
  // build the server socket
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);
  Socket server_sock;
  server_sock.Bind(local_host, true);
  server_sock.Listen();
  REQUIRE(server_sock.GetFd() != -1);

  SECTION("looper execute connection's callback func when triggered") {
    const auto client_num = 3U;
    std::vector<std::thread> threads;

    threads.reserve(client_num);
    for (auto i = 0U; i < client_num; ++i) {
      threads.emplace_back([&host = local_host]() {
        auto client_socket = Socket();
        client_socket.Connect(host);
        std::this_thread::sleep_for(1s);
      });
    }

    // build 3 connections and add into looper with customized callback function
    std::vector<int> reach(client_num, 0U);
    for (auto i = 0U; i < client_num; ++i) {
      NetAddress client_address;
      auto client_sock =
        std::make_unique<Socket>(server_sock.Accept(client_address));
      CHECK(client_sock->GetFd() != -1);
      client_sock->SetNonBlocking();
      auto client_conn = std::make_unique<Connection>(std::move(client_sock));
      client_conn->SetEvents(Poller::Event::kRead);
      client_conn->SetCallback([&](longlp::not_null<Connection*> /* conn */) {
        // counter.fetch_add(1, std::memory_order_relaxed);
        // counter_dec.fetch_sub(1, std::memory_order_relaxed);
        reach[i] = 1;
      });
      looper.AddConnection(std::move(client_conn));
    }

    /* the looper execute each client's callback once, upon their exit */
    std::thread runner([&]() { looper.Loop(); });
    std::this_thread::sleep_for(2s);
    looper.SetExit();

    // each client's callback should have already been executed
    // CHECK(counter == client_num);
    // CHECK(counter_dec == -3);
    CHECK(std::accumulate(reach.begin(), reach.end(), 0) == client_num);
    runner.join();
    for (auto i = 0U; i < client_num; ++i) {
      threads[i].join();
    }
  }
}
