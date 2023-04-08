// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_CORE_TYPEDEFS_H_
#define SRC_CORE_TYPEDEFS_H_

#include <cstdint>
#include <functional>
#include <vector>

#include "base/pointers.h"

namespace longlp {
class Connection;
using ConnectionCallback = std::function<void(not_null<Connection*>)>;

using Byte               = uint8_t;
using ByteData           = std::vector<uint8_t>;
}    // namespace longlp

#endif    // SRC_CORE_TYPEDEFS_H_
