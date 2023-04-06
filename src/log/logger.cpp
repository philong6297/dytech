// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "log/logger.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>

#include <fmt/chrono.h>
#include <fmt/format.h>

namespace longlp {

namespace {
constexpr int kThreshold = 1000;
constexpr std::chrono::duration kRefreshThresholdDuration =
  std::chrono::microseconds(3000);

auto GetCurrentTime() -> std::chrono::milliseconds {
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;
  using std::chrono::system_clock;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

}    // namespace

// Each individual Log message upon construction, the date time is prepended
struct Logger::Log {
  std::string stamped_msg_;

  // stamp datetime and log level not guaranteed to be output in the stamped
  // time order, best effort approach
  Log(LogLevel log_level, const std::string_view log_msg) noexcept {
    static constexpr std::array<std::string_view, 4> log_level_names =
      {"INFO: ", "WARNING: ", "ERROR: ", "FATAL: "};

    auto current_time = std::time(nullptr);
    std::ostringstream stream;
    stream << fmt::format(
      "{time_stamp:%d-%m-%Y %H:%M:%S}{log_level}:{log_msg}\n",
      fmt::arg("time_stamp", fmt::localtime(current_time)),
      fmt::arg("log_level", log_level_names[static_cast<size_t>(log_level)]),
      fmt::arg("log_msg", log_msg));
    stamped_msg_ = stream.str();
  }

  friend auto operator<<(std::ostream& os, const Log& log) -> std::ostream& {
    os << log.stamped_msg_;
    return os;
  }
};

// opened log stream during the lifetime of the entire server
struct Logger::StreamWriter {
  std::fstream f_;

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

  void WriteLogs(const std::deque<Logger::Log>& logs) {
    std::for_each(logs.begin(), logs.end(), [this](auto& log) { f_ << log; });
    f_.flush();
  }
};

// static
void Logger::LogMsg(LogLevel log_level, const std::string_view msg) noexcept {
  auto log = Logger::Log(log_level, msg);
  GetInstance().PushLog(std::move(log));
}

// static
auto Logger::GetInstance() noexcept -> Logger& {
  static Logger single_logger{};
  return single_logger;
}

Logger::Logger() {
  last_flush_ = GetCurrentTime();
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
  using std::chrono::milliseconds;
  bool should_notify = false;
  {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_.push_back(std::move(log));
    milliseconds now = GetCurrentTime();
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
    static StreamWriter stream_writer;
    stream_writer.WriteLogs(logs);
  };

  while (true) {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() {
      return done_ || queue_.size() > kThreshold ||
             GetCurrentTime() - last_flush_ > kRefreshThresholdDuration;
    });
    // either the flush criteria is met or is about to exit
    // need to record the remaining log in either case
    if (!queue_.empty()) {
      writer_queue.swap(queue_);
      lock.unlock();    // producer may continue
      print_to_file(writer_queue);
      last_flush_ = GetCurrentTime();
      writer_queue.clear();
    }
    if (done_) {
      // exit this background thread
      return;
    }
  }
}

}    // namespace longlp
