// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/poller.h"

#include <cassert>
#include <cstring>

#include "base/utils.h"
#include "core/connection.h"
#include "log/logger.h"

namespace longlp {

namespace {
auto DefaultPollEvent() -> epoll_event {
  epoll_event ret{};
  memset(&ret, 0, sizeof(epoll_event));
  return ret;
}
}    // namespace

Poller::Poller(uint64_t poll_size) :
  poll_fd_(epoll_create1(EPOLL_CLOEXEC)),
  poll_events_(poll_size, DefaultPollEvent()) {
  if (poll_fd_ == -1) {
    perror("Poller: epoll_create1() error");
    // TODO(longlp): It is not thread-safe
    std::exit(EXIT_FAILURE);
  }
}

Poller::~Poller() {
  if (poll_fd_ != -1) {
    close(poll_fd_);
    poll_fd_ = -1;
  }
}

void Poller::AddConnection(Connection* conn) const {
  assert(conn->GetFd() != -1 && "cannot AddConnection() with an invalid fd");

  auto event     = DefaultPollEvent();
  event.data.ptr = conn;
  event.events   = conn->GetEvents();

  const auto ret_val =
    epoll_ctl(poll_fd_, Poller::Event::kAdd, conn->GetFd(), &event);
  if (ret_val == -1) {
    perror("Poller: epoll_ctl add error");
    // TODO(longlp): It is not thread-safe
    std::exit(EXIT_FAILURE);
  }
}

auto Poller::Poll(int timeout) -> std::vector<Connection*> {
  auto ready = epoll_wait(
    poll_fd_,
    poll_events_.data(),
    narrow_cast<int>(poll_events_.size()),
    timeout);
  if (ready == -1) {
    perror("Poller: Poll() error");
    // TODO(longlp): It is not thread-safe
    std::exit(EXIT_FAILURE);
  }

  std::vector<Connection*> events_happen;
  events_happen.reserve(narrow_cast<size_t>(ready));
  for (auto i = 0; i < ready; ++i) {
    const auto& event      = poll_events_[narrow_cast<size_t>(i)];
    auto* ready_connection = bit_cast<Connection*>(event.data.ptr);
    ready_connection->SetRevents(event.events);
    events_happen.emplace_back(ready_connection);
  }
  return events_happen;
}

}    // namespace longlp
