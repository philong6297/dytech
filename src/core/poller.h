// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_POLLER_H_
#define CORE_POLLER_H_

#include <sys/epoll.h>

#include <memory>
#include <vector>

#include "core/utils.h"

namespace longlp {

class Connection;

// the Poller which actively does epolling on a collection of socket descriptors
// to be monitored
class Poller {
 public:
  /* the default maximum number of events to be listed on epoll tree */
  static constexpr auto kDefaultListenedEvents = 1024U;
  static constexpr auto kBlockForever          = -1;

  enum Event {
    kAdd  = EPOLL_CTL_ADD,
    kRead = EPOLLIN,
    kET   = EPOLLET,
  };

  explicit Poller(uint64_t poll_size);
  ~Poller();
  DISALLOW_COPY(Poller);
  DEFAULT_MOVE(Poller);

  void AddConnection(Connection* conn) const;

  // timeout in milliseconds
  [[nodiscard]] auto Poll(int timeout_ms) -> std::vector<Connection*>;

  [[nodiscard]] auto GetPollSize() const noexcept -> uint64_t {
    return poll_events_.size();
  }

 private:
  int poll_fd_{};
  std::vector<epoll_event> poll_events_{};
};
}    // namespace longlp
#endif    // CORE_POLLER_H_
