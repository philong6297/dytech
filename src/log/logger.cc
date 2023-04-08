// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "log/logger.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include "base/chrono.h"
#include "base/no_destructor.h"

namespace longlp {

namespace {
constexpr auto kThreshold                = 1000;
constexpr auto kRefreshThresholdDuration = microseconds(3000);

inline constexpr auto ToString(LogLevel level) -> std::string_view {
  switch (level) {
    case LogLevel::kError:
      return "Error";
    case LogLevel::kFatal:
      return "Fatal";
    case LogLevel::kInfo:
      return "Info";
    case LogLevel::kWarning:
      return "Warning";
  }
}

}    // namespace

// Each individual Log message upon construction, the date time is prepended
class Logger::Log {
 public:
  // stamp datetime and log level not guaranteed to be output in the stamped
  // time order, best effort approach
  Log(LogLevel log_level, const std::string_view log_msg) noexcept {
    auto current_time = std::time(nullptr);
    std::ostringstream stream;
    stream << fmt::format(
      "{time_stamp:%d-%m-%Y %H:%M:%S}{log_level}:{log_msg}\n",
      fmt::arg("time_stamp", fmt::localtime(current_time)),
      fmt::arg("log_level", ToString(log_level)),
      fmt::arg("log_msg", log_msg));
    stamped_msg_ = stream.str();
  }

  friend auto operator<<(std::ostream& os, const Log& log) -> std::ostream& {
    os << log.stamped_msg_;
    return os;
  }

 private:
  std::string stamped_msg_;
};

// opened log stream during the lifetime of the entire server
class Logger::StreamWriter {
 public:
  explicit StreamWriter() {
    const auto current_time = std::time(nullptr);

    // format DD-MM-YYYY
    f_.open(
      fmt::format("longlp_log_{:%d-%m-%Y}", fmt::localtime(current_time)),
      std::fstream::out | std::fstream::trunc);
  }

  ~StreamWriter() {
    if (f_.is_open()) {
      f_.flush();
      f_.close();
    }
  }

  DISALLOW_COPY_AND_MOVE(StreamWriter);

  void WriteLogs(const std::deque<Logger::Log>& logs) {
    std::for_each(logs.begin(), logs.end(), [this](auto& log) { f_ << log; });
    f_.flush();
  }

 private:
  std::fstream f_;
};

// static
void Logger::LogMsg(LogLevel log_level, const std::string_view msg) noexcept {
  auto log = Logger::Log(log_level, msg);
  GetInstance().PushLog(std::move(log));
}

// static
auto Logger::GetInstance() noexcept -> Logger& {
  static NoDestructor<Logger> single_logger{};
  return *single_logger;
}

Logger::Logger() {
  last_flush_ = GetCurrentTimeMs();
  log_writer_ = std::thread(&Logger::LogWriting, this);
}

Logger::~Logger() {
  // signal and harvest backend thread
  done_ = true;
  cv_.notify_one();
  if (log_writer_.joinable()) {
    log_writer_.join();
  }
}

void Logger::PushLog(Logger::Log&& log) {
  bool should_notify = false;
  {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_.push_back(std::move(log));
    milliseconds now = GetCurrentTimeMs();
    if ((now - last_flush_) > kRefreshThresholdDuration ||
        queue_.size() > kThreshold) {
      should_notify = true;
    }
  }
  // a best effort notification to worker thread, not guarantee flush soon
  if (should_notify) {
    cv_.notify_one();
  }
}

void Logger::LogWriting() {
  std::deque<Logger::Log> writer_queue;

  auto print_to_file = [](const std::deque<Logger::Log>& logs) {
    static NoDestructor<StreamWriter> stream_writer{};
    (*stream_writer).WriteLogs(logs);
  };

  while (true) {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() {
      return done_ || queue_.size() > kThreshold ||
             GetCurrentTimeMs() - last_flush_ > kRefreshThresholdDuration;
    });
    // either the flush criteria is met or is about to exit
    // need to record the remaining log in either case
    if (!queue_.empty()) {
      writer_queue.swap(queue_);
      lock.unlock();    // producer may continue
      print_to_file(writer_queue);
      last_flush_ = GetCurrentTimeMs();
      writer_queue.clear();
    }
    if (done_) {
      // exit this background thread
      return;
    }
  }
}

}    // namespace longlp
