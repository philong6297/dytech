// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/distribution_agent.h"

#include <atomic>
#include <random>
#include <thread>

namespace longlp {
namespace {
// lock-free with atomic thread_local
auto GenerateRandomNumber(size_t upper_bound) -> size_t {
  // Create thread-local seeds
  const thread_local std::atomic<std::random_device::result_type>
    seed(std::random_device{}());
  thread_local std::mt19937 gen(seed);

  // Create a uniform integer distribution within the range
  std::uniform_int_distribution<size_t> distr(0, upper_bound);

  // Generate a random number within the range
  return distr(gen);
}
}    // namespace

void DistributionAgent::AddCandidate(not_null<Looper*> candidate) {
  candidates_.emplace_back(candidate);
}

auto DistributionAgent::SelectCandidate() const
  -> std::pair<not_null<Looper*> /* candidate */, size_t /* candidate id */> {
  const auto idx = GenerateRandomNumber(candidates_.size() - 1);
  return {candidates_[idx], idx};
}

}    // namespace longlp
