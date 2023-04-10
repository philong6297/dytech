// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/server.h"

#include "core/acceptor.h"
#include "core/distribution_agent.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/thread_pool.h"

namespace longlp {

Server::Server(const NetAddress& server_address, int64_t num_threads) :
  agent_{std::make_unique<DistributionAgent>()},
  pool_(std::make_unique<ThreadPool>(num_threads)),
  listener_(std::make_unique<Looper>()) {
  reactors_.reserve(pool_->GetSize());
  for (auto i = 0U; i < pool_->GetSize(); ++i) {
    auto& reactor = reactors_.emplace_back(std::make_unique<Looper>());
    pool_->SubmitTask([&reactor] { reactor->StartLoop(); });
    agent_->AddCandidate(reactor.get());
  }

  acceptor_ =
    std::make_unique<Acceptor>(listener_.get(), agent_.get(), server_address);
}

Server::~Server() = default;

auto Server::OnAccept(ConnectionCallback on_accept) -> Server& {
  acceptor_->SetOnAccept(std::move(on_accept));
  return *this;
}

auto Server::OnHandle(ConnectionCallback on_handle) -> Server& {
  acceptor_->SetOnHandle(std::move(on_handle));
  on_handle_set_ = true;
  return *this;
}

void Server::Begin() {
  if (!on_handle_set_) {
    throw std::logic_error(
      "Please specify OnHandle callback function before starts");
  }
  listener_->StartLoop();
}

}    // namespace longlp
