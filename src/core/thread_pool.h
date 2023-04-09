// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_THREAD_POOL_H_
#define SRC_CORE_THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "base/macros.h"

namespace longlp {

class ThreadPool {
 public:
  explicit ThreadPool(size_t thread_count);

  DISALLOW_COPY_AND_MOVE(ThreadPool);
  ~ThreadPool();

  // add new work item to the pool
  template <class F, class... Args>
  auto SubmitTask(F&& new_task, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>;

  auto GetSize() -> size_t { return workers_.size(); }

 private:
  // need to keep track of threads so we can join them
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_queue_;

  // synchronization
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> stop_{false};
};

template <class F, class... Args>
auto ThreadPool::SubmitTask(F&& new_task, Args&&... args)
  -> std::future<std::invoke_result_t<F, Args...>> {
  using return_type = std::invoke_result_t<F, Args...>;

  // create a packaged task async
  auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(new_task), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();

  // assign task to queue
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // don't allow pushing after stopping the pool
    if (stop_) {
      throw std::runtime_error("Submitting on a stopped ThreadPool");
    }

    tasks_queue_.emplace([task]() { (*task)(); });
  }

  // notify one waiting thread ~ 1 worker is relaxing to assign the task
  condition_.notify_one();

  return res;
}

}    // namespace longlp

#endif    // SRC_CORE_THREAD_POOL_H_
