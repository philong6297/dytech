// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_POINTERS_H_
#define SRC_BASE_POINTERS_H_

#include <gsl/pointers>

namespace longlp {
using gsl::make_not_null;
using gsl::not_null;
using gsl::owner;
}    // namespace longlp

#endif    // SRC_BASE_POINTERS_H_
