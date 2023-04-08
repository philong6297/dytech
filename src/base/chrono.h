// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_CHRONO_H_
#define SRC_BASE_CHRONO_H_

#include <chrono>

namespace longlp {
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::system_clock;

// it should be inlined to have the most accuracy
[[gnu::always_inline]] auto GetCurrentTimeMs() -> milliseconds;
}    // namespace longlp

#endif    // SRC_BASE_CHRONO_H_
