// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "base/chrono.h"

namespace longlp {
auto GetCurrentTimeMs() -> milliseconds {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}
}    // namespace longlp
