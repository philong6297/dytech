// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_UTILS_H_
#define CORE_UTILS_H_

#define DISALLOW_COPY(class_name)                                        \
  class_name(const class_name&)                                = delete; \
  [[nodiscard]] auto operator=(const class_name&)->class_name& = delete

#define DISALLOW_MOVE(class_name)                                   \
  class_name(class_name&&)                                = delete; \
  [[nodiscard]] auto operator=(class_name&&)->class_name& = delete

#define DEFAULT_COPY(class_name)                                          \
  class_name(const class_name&)                                = default; \
  [[nodiscard]] auto operator=(const class_name&)->class_name& = default

#define DEFAULT_MOVE(class_name)                                     \
  class_name(class_name&&)                                = default; \
  [[nodiscard]] auto operator=(class_name&&)->class_name& = default

#define DISALLOW_COPY_AND_MOVE(class_name) \
  DISALLOW_COPY(class_name);               \
  DISALLOW_MOVE(class_name)

#endif    // CORE_UTILS_H_
