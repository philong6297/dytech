// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/thread_pool.h"

#include <atomic>

#include <catch2/catch_test_macros.hpp>

using longlp::ThreadPool;

TEST_CASE("[core/thread_pool]") {
  constexpr auto thread_pool_size = 8U;
  constexpr auto task_num         = 24U;
  ThreadPool pool(thread_pool_size);
  REQUIRE(pool.GetSize() == thread_pool_size);

  SECTION("launch multiple tasks and wait for exit") {
    std::atomic<size_t> counter{0};
    {
      ThreadPool local_pool(thread_pool_size);
      for (auto i = 0U; i < task_num; ++i) {
        local_pool.SubmitTask([&]() { ++counter; });
      }
      // here thread_pool's dtor should finish all the tasks
    }
    CHECK(counter == task_num);
  }
}
