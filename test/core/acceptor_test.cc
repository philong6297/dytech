// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/acceptor.h"

#include <unistd.h>

#include <future>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "core/connection.h"
#include "core/distribution_agent.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"
#include "core/thread_pool.h"

using longlp::Acceptor;
using longlp::Connection;
using longlp::DistributionAgent;
using longlp::Looper;
using longlp::NetAddress;
using longlp::not_null;
using longlp::Protocol;
using longlp::Socket;
using longlp::ThreadPool;

TEST_CASE("[core/acceptor]") {
  NetAddress local_host("127.0.0.1", 20080, Protocol::Ipv4);

  ThreadPool pool;

  // built an acceptor will one listener looper and one reactor together
  auto single_reactor = std::make_unique<Looper>();

  std::vector<std::unique_ptr<Looper>> reactors;
  reactors.emplace_back(std::make_unique<Looper>());

  auto agent = std::make_unique<DistributionAgent>();
  for (auto& reactor : reactors) {
    agent->AddCandidate(reactor.get());
  }
  Acceptor acceptor(single_reactor.get(), agent.get(), local_host);

  REQUIRE(acceptor.GetAcceptorConnection()->GetFd() != -1);

  SECTION(
    "Acceptor should be able to accept new clients and set callback for "
    "them") {
    int client_num                  = 3;
    std::atomic<int> accept_trigger = 0;
    std::atomic<int> handle_trigger = 0;

    // set acceptor customize functions
    acceptor.SetOnAccept([&](not_null<Connection*>) { accept_trigger++; });
    acceptor.SetOnHandle([&](not_null<Connection*>) { handle_trigger++; });

    // start three clients and connect with server
    const char* msg = "Hello from client!";
    std::vector<std::future<void>> futs;
    for (int i = 0; i < client_num; i++) {
      auto fut = std::async(std::launch::async, [&]() {
        Socket client_sock;
        client_sock.Connect(local_host);
        CHECK(client_sock.GetFd() != -1);
        send(client_sock.GetFd(), msg, strlen(msg), 0);
      });
      futs.push_back(std::move(fut));
    }

    auto runner = std::async(std::launch::async, [&]() {
      single_reactor->Loop();
    });
    futs.push_back(std::move(runner));
    sleep(2);
    single_reactor->SetExit();    // terminate the looper

    // accept & handle should be triggered exactly 3 times
    CHECK(accept_trigger == client_num);
    CHECK(handle_trigger >= client_num);

    for (auto& f : futs) {
      f.wait();
    }
  }
}
