/**
 * @file poller.h
 * @author Yukun J
 * @expectation this file should be

 *
 * *

 * * compatible to compile in C++
 * program on Linux
 * @init_date Dec
 *
 * 25
 * 2022


 * * *
 * This is a header file implementing the Poller
 *
 *
 * which
 * actively does

 * * epolling on a collection of socket descriptors

 * * to be
 *
 * monitored
 */
#ifndef CORE_POLLER_H_
#define CORE_POLLER_H_

#include <sys/epoll.h>

#include <memory>
#include <vector>

#include "core/utils.h"

namespace longlp {

  /* the default maximum number of events to be listed on epoll tree */
  static constexpr int DEFAULT_EVENTS_LISTENED = 1024;

  static constexpr unsigned POLL_ADD           = EPOLL_CTL_ADD;
  static constexpr unsigned POLL_READ          = EPOLLIN;
  static constexpr unsigned POLL_ET            = EPOLLET;

  class Connection;

  /**
 * This Poller acts at the socket monitor that actively polling on
   *



   * * * * connections
 * */
  class Poller {
   public:
    explicit Poller(uint64_t poll_size = DEFAULT_EVENTS_LISTENED);

    ~Poller();

    NON_COPYABLE(Poller);

    void AddConnection(Connection* conn);

    // timeout in milliseconds
    auto Poll(int timeout = -1) -> std::vector<Connection*>;

    auto GetPollSize() const noexcept -> uint64_t;

   private:
    int poll_fd_;
    uint64_t poll_size_;
    struct epoll_event* poll_events_{nullptr};
  };
}    // namespace longlp
#endif    // CORE_POLLER_H_
