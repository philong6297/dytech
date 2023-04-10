// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/looper.h"

#include <fmt/format.h>
#include "core/acceptor.h"
#include "core/connection.h"
#include "core/poller.h"
#include "core/thread_pool.h"

namespace longlp {

namespace {
// the epoll_wait time in milliseconds
constexpr int kTimeoutMs = 3000;
}    // namespace

Looper::Looper() :
  poller_(std::make_unique<Poller>(Poller::kDefaultListenedEvents)) {}

Looper::~Looper() = default;

void Looper::StartLoop() {
  while (!exit_) {
    auto ready_connections = poller_->Poll(kTimeoutMs);
    // fmt::print("ready connection size: {}\n", ready_connections.size());
    for (auto& connection : ready_connections) {
      connection->Start();
    }
  }
}

void Looper::AddAcceptor(Connection* acceptor_conn) {
  std::unique_lock<std::mutex> lock(mtx_);
  poller_->AddConnection(acceptor_conn);
}

void Looper::AddConnection(std::unique_ptr<Connection> new_conn) {
  std::unique_lock<std::mutex> lock(mtx_);
  poller_->AddConnection(new_conn.get());
  int fd = new_conn->GetFd();
  connections_.insert({fd, std::move(new_conn)});
}

auto Looper::DeleteConnection(int fd) -> bool {
  std::unique_lock<std::mutex> lock(mtx_);
  auto it = connections_.find(fd);
  if (it == connections_.end()) {
    return false;
  }
  connections_.erase(it);
  return true;
}

}    // namespace longlp
