// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/thread_pool.h"

namespace longlp {
ThreadPool::ThreadPool(const size_t thread_count) :
  stop_(false) {
  static constexpr size_t kMinThreadCount = 2U;

  // std::thread::hardware_concurrency() might return 0 if sys info not
  // available
  const auto max_thread = std::max(thread_count, kMinThreadCount);
  // Create |thread_count| workers prepare for incomming tasks
  workers_.reserve(max_thread);
  for (size_t i = 0; i < max_thread; ++i) {
    workers_.emplace_back([this] {
      while (true) {
        std::function<void()> incoming_task;

        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          // create condition for mutex when need to lock, unlock
          condition_.wait(lock, [this] {
            return stop_ || !tasks_queue_.empty();
          });

          // thread life ends
          // if the thread pool needs to stop and there is no task left, any
          // on-going work should be cancelled
          if (stop_ && tasks_queue_.empty()) {
            return;
          }

          // if the thread pool want to stop but it still has tasks, or the
          // thread pool is still running, the worker will assign for new task
          incoming_task = std::move(tasks_queue_.front());

          // remove the task from queue
          tasks_queue_.pop();
        }

        // worker do the task
        incoming_task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  stop_ = true;

  // notify all the workers to stop and join
  condition_.notify_all();

  for (auto& worker : workers_) {
    // TODO(longlp): Should we force join?
    if (worker.joinable()) {
      worker.join();
    }
  }
}
}    // namespace longlp
