// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/acceptor.h"

#include <chrono>
#include <future>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "core/connection.h"
#include "core/distribution_agent.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"
#include "core/thread_pool.h"

namespace {
using longlp::Acceptor;
using longlp::Connection;
using longlp::DistributionAgent;
using longlp::Looper;
using longlp::NetAddress;
using longlp::not_null;
using longlp::Protocol;
using longlp::Socket;
using longlp::ThreadPool;
using namespace std::chrono_literals;
}    // namespace

TEST_CASE("[core/acceptor]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);

  ThreadPool pool(std::thread::hardware_concurrency());

  // built an acceptor will one listener looper and one reactor together
  auto single_reactor = std::make_unique<Looper>();
  auto single_looper  = std::make_unique<Looper>();
  auto agent          = std::make_unique<DistributionAgent>();
  agent->AddCandidate(single_looper.get());

  Acceptor acceptor(single_reactor.get(), agent.get(), local_host);

  REQUIRE(acceptor.GetAcceptorConnection()->GetFd() != -1);

  SECTION(
    "Acceptor should be able to accept new clients and set callback for "
    "them") {
    static constexpr auto kClientNum           = 3U;
    static constexpr std::string_view kMessage = "Hello from client!";
    std::atomic<size_t> accept_trigger         = 0;
    std::atomic<size_t> handle_trigger         = 0;

    // set acceptor customize functions
    acceptor.SetOnAccept([&](not_null<Connection*>) { ++accept_trigger; });
    acceptor.SetOnHandle([&](not_null<Connection*>) { ++handle_trigger; });

    // start three clients and connect with server
    std::vector<std::future<void>> futs;
    for (auto i = 0U; i < kClientNum; ++i) {
      auto fut = std::async(std::launch::async, [&]() {
        Socket client_socket;
        client_socket.ConnectToServer(local_host);
        CHECK(client_socket.GetFd() != -1);
        send(client_socket.GetFd(), kMessage.data(), kMessage.size(), 0);
      });
      futs.push_back(std::move(fut));
    }

    auto runner = std::async(std::launch::async, [&]() {
      single_reactor->StartLoop();
    });
    futs.push_back(std::move(runner));
    std::this_thread::sleep_for(2s);
    single_reactor->Exit();    // terminate the looper

    // accept & handle should be triggered exactly 3 times
    CHECK(accept_trigger == kClientNum);
    CHECK(handle_trigger >= kClientNum);

    for (auto& f : futs) {
      f.wait();
    }
  }
}
