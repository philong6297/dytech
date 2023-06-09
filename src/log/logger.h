// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef LOG_LOGGER_H_
#define LOG_LOGGER_H_

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "base/chrono.h"
#include "base/macros.h"
#include "base/no_destructor.h"

namespace longlp {

enum class LogLevel : size_t {
  kInfo    = 0,
  kWarning = 1,
  kError   = 2,
  kFatal   = 3,
};

// A simple asynchronous logger
// All callers counts as frontend-producer and is non-blocking a backend worker
// thread periodically flush the log to persistent storage
class Logger {
 public:
  static void LogMsg(LogLevel log_level, std::string_view msg) noexcept;

  [[nodiscard]] static auto GetInstance() noexcept -> Logger&;

  DISALLOW_COPY_AND_MOVE(Logger);

 private:
  class Log;
  class StreamWriter;
  friend class NoDestructor<Logger>;

  explicit Logger();

  ~Logger();

  // internal helper to push a log into the FIFO queue potentially notify the
  // backend worker to swap and flush if threshold criteria is met
  void PushLog(Log&& log);

  // The thread routine for the backend log writer
  void LogWriting();

  std::atomic<bool> done_ = false;
  std::mutex mtx_;
  std::condition_variable cv_;
  std::deque<Log> queue_;
  std::thread log_writer_;
  microseconds last_flush_;
};

template <LogLevel level>
inline void Log(std::string_view msg) {
  Logger::LogMsg(level, msg);
}

}    // namespace longlp

#endif    // LOG_LOGGER_H_
