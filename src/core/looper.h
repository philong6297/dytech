// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

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

  void Loop();

  void AddAcceptor(Connection* acceptor_conn);

  void AddConnection(std::unique_ptr<Connection> new_conn);

  [[nodiscard]] auto DeleteConnection(int fd) -> bool;

  void SetExit() noexcept { exit_ = true; }

 private:
  std::unique_ptr<Poller> poller_;
  std::mutex mtx_;
  std::map<int, std::unique_ptr<Connection>> connections_;
  bool exit_{false};
};
}    // namespace longlp
#endif    // CORE_LOOPER_H_
