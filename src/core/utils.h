/**
 * @file utils.h
 * @author Yukun J
 * @expectation this header file
 *
 *

 * * should

 * * be compatible to compile in C++
 * program on Linux
 *
 *
 *

 * * @init_date Dec 25
 *
 * 2022
 *
 * This is a header file only helper for

 * *
 * some
 * macro definitions
 */

#ifndef CORE_UTILS_H_
#define CORE_UTILS_H_

#define NON_COPYABLE(class_name)                           \
  class_name(const class_name&)                  = delete; \
  auto operator=(const class_name&)->class_name& = delete

#define NON_MOVEABLE(class_name)                      \
  class_name(class_name&&)                  = delete; \
  auto operator=(class_name&&)->class_name& = delete

#define DEFAULT_COPY(class_name)                            \
  class_name(const class_name&)                  = default; \
  auto operator=(const class_name&)->class_name& = default

#define DEFAULT_MOVE(class_name)                       \
  class_name(class_name&&)                  = default; \
  auto operator=(class_name&&)->class_name& = default

#define NON_COPYABLE_AND_MOVEABLE(class_name) \
  NON_COPYABLE(class_name);                   \
  NON_MOVEABLE(class_name)

#endif    // CORE_UTILS_H_
