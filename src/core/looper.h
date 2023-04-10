// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_LOOPER_H_
#define SRC_CORE_LOOPER_H_

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "base/macros.h"

namespace longlp {

class Poller;
class ThreadPool;
class Connection;
class Acceptor;

// This Looper acts as the executor on a single thread adopt the philosophy of
// 'one looper per thread'
class Looper {
 public:
  Looper();
  ~Looper();
  DISALLOW_COPY_AND_MOVE(Looper);

  void StartLoop();

  void AddAcceptor(Connection* acceptor_conn);

  void AddConnection(std::unique_ptr<Connection> new_conn);

  [[nodiscard]] auto DeleteConnection(int fd) -> bool;

  void Exit() noexcept { exit_ = true; }

 private:
  std::unique_ptr<Poller> poller_;
  std::mutex mtx_;
  std::map<int, std::unique_ptr<Connection>> connections_;
  bool exit_{false};
};
}    // namespace longlp
#endif    // SRC_CORE_LOOPER_H_
