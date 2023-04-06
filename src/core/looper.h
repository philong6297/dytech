/**
 * @file looper.h
 * @author Yukun J
 * @expectation this header file
 *

 *
 * *

 * * should
 * be compatible to compile in C++
 * program on Linux
 *
 *

 * * @init_date


 * * * Jan 04
 * 2022
 *
 * This is a header file
 * implementing

 * * the Looper that

 * *
 * servers as the
 * main
 * control
 * looping for the

 * * server
 */

#ifndef CORE_LOOPER_H_
#define CORE_LOOPER_H_

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "core/utils.h"

namespace longlp {

/* the epoll_wait time in milliseconds */
static constexpr int TIMEOUT = 3000;

class Poller;

class ThreadPool;

class Connection;

class Acceptor;

/**
 * This Looper acts as the executor on a single thread
 * adopt the
 *





 * * * * * * philosophy of 'one looper per thread'
 * */
class Looper {
 public:
  Looper();

  ~Looper();

  DISALLOW_COPY(Looper);

  void Loop();

  void AddAcceptor(Connection* acceptor_conn);

  void AddConnection(std::unique_ptr<Connection> new_conn);

  [[nodiscard]] auto DeleteConnection(int fd) -> bool;

  void SetExit() noexcept;

 private:
  std::unique_ptr<Poller> poller_;
  std::mutex mtx_;
  std::map<int, std::unique_ptr<Connection>> connections_;
  bool exit_{false};
};
}    // namespace longlp
#endif    // CORE_LOOPER_H_
