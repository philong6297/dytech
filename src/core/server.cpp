// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/server.h"

namespace longlp {

Server::Server(NetAddress server_address, int concurrency) :
  pool_(std::make_unique<ThreadPool>(concurrency)),
  listener_(std::make_unique<Looper>()) {
  for (size_t i = 0; i < pool_->GetSize(); i++) {
    reactors_.push_back(std::make_unique<Looper>());
  }
  for (auto& reactor : reactors_) {
    pool_->SubmitTask([capture0 = reactor.get()] { capture0->Loop(); });
  }
  std::vector<Looper*> raw_reactors;
  raw_reactors.reserve(reactors_.size());
  std::transform(
    reactors_.begin(),
    reactors_.end(),
    std::back_inserter(raw_reactors),
    [](auto& uni_ptr) { return uni_ptr.get(); });
  acceptor_ =
    std::make_unique<Acceptor>(listener_.get(), raw_reactors, server_address);
}

Server::~Server() = default;

auto Server::OnAccept(std::function<void(Connection*)> on_accept) -> Server& {
  acceptor_->SetCustomAcceptCallback(std::move(on_accept));
  return *this;
}

auto Server::OnHandle(std::function<void(Connection*)> on_handle) -> Server& {
  acceptor_->SetCustomHandleCallback(std::move(on_handle));
  on_handle_set_ = true;
  return *this;
}

void Server::Begin() {
  if (!on_handle_set_) {
    throw std::logic_error(
      "Please specify OnHandle callback function before starts");
  }
  listener_->Loop();
}

}    // namespace longlp
