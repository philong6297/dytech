// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_DISTRIBUTION_AGENT_H_
#define SRC_CORE_DISTRIBUTION_AGENT_H_

#include <utility>
#include <vector>

#include "base/pointers.h"

namespace longlp {
class Looper;

class DistributionAgent {
 public:
  void AddCandidate(not_null<Looper*> candidate);

  // TODO(longlp): randomized distribution. uniform in long term.
  [[nodiscard]] auto SelectCandidate() const
    -> std::pair<not_null<Looper*> /* candidate */, size_t /* candidate id */>;

 private:
  std::vector<not_null<Looper*>> candidates_{};
};
}    // namespace longlp

#endif    // SRC_CORE_DISTRIBUTION_AGENT_H_
