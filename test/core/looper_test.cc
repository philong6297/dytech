// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/looper.h"

#include <array>
#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>
#include <thread>
#include <vector>

#include <fmt/format.h>
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
  NetAddress server_address("127.0.0.1", 20080, Protocol::Ipv4);
  Socket server_socket;
  server_socket.BindWithServerAddress(server_address, true);
  server_socket.StartListeningIncomingConnection();
  REQUIRE(server_socket.GetFd() != -1);

  SECTION("looper execute connection's callback func when triggered") {
    constexpr auto kClientNum = 3U;

    std::mutex mtx;
    std::atomic<bool> looper_exited{false};
    std::condition_variable cv;

    std::vector<std::thread> threads;
    threads.reserve(kClientNum);
    for (auto i = 0U; i < kClientNum; ++i) {
      threads.emplace_back([i, &server_address, &mtx, &cv, &looper_exited]() {
        auto client_socket = Socket();
        client_socket.ConnectToServer(server_address);
        client_socket.SetNonBlocking();
        // fmt::print(
        //   "client {} with fd {} created connection\n",
        //   i,
        //   client_socket.GetFd());
        // // keep the connection alive until looper exit
        // {
        //   std::unique_lock<std::mutex> lock(mtx);
        //   cv.wait(lock, [&]() -> bool { return looper_exited; });
        // }
        // fmt::print("client {} exit\n", i);
      });
    }

    // build 3 connections and add into looper with customized callback function
    std::array<std::atomic<bool>, kClientNum> reached_to_client =
      {false, false, false};

    for (auto i = 0U; i < kClientNum; ++i) {
      NetAddress client_address;
      auto client_socket = std::make_unique<Socket>(
        server_socket.AcceptClientAddress(client_address));
      CHECK(client_socket->GetFd() != -1);
      client_socket->SetNonBlocking();
      auto client_conn = std::make_unique<Connection>(std::move(client_socket));
      client_conn->SetEvents(Poller::Event::kRead);
      client_conn->SetCallback(
        [&reached_to_client, i](longlp::not_null<Connection*> /* conn */) {
          reached_to_client.at(i) = true;
        });
      looper.AddConnection(std::move(client_conn));
    }

    // the looper execute each client's callback once, upon their exit
    std::thread runner([&]() { looper.StartLoop(); });

    // let the looper runs for maximum of 10s
    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait_for(lock, 10s, [&]() -> bool {
        return reached_to_client[0] && reached_to_client[1] &&
               reached_to_client[2];
      });
    }

    looper.Exit();
    {
      looper_exited = true;
      cv.notify_all();
    }

    // each client's callback should have already been executed
    for (auto i = 0U; i < kClientNum; ++i) {
      CHECK(reached_to_client.at(i));
    }
    runner.join();
    for (auto i = 0U; i < kClientNum; ++i) {
      threads[i].join();
    }
  }
}
