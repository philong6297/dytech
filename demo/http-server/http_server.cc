// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string_view>
#include <system_error>
#include <thread>

#include <fmt/format.h>
#include <cxxopts.hpp>

#include "base/pointers.h"
#include "core/cache.h"
#include "core/connection.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/server.h"
#include "http/cgi_runner.h"
#include "http/constants.h"
#include "http/header.h"
#include "http/http_utils.h"
#include "http/request.h"
#include "http/response.h"
#include "log/logger.h"

namespace longlp::http {

namespace {
auto HandleStaticResourceRequest(
  const Request& request,
  const std::string& resource_full_path,
  std::shared_ptr<Cache>& cache,
  DynamicByteArray& response_buf) -> bool /* should_finish */ {
  if (!IsFileExists(resource_full_path)) {
    Log<LogLevel::kInfo>(fmt::format("{} not exist.", resource_full_path));
    auto response = Response::Make404Response();
    response.Serialize(response_buf);
    return true;
  }

  auto response =
    Response::Make200Response(request.ShouldClose(), resource_full_path);
  response.Serialize(response_buf);

  DynamicByteArray cache_buf;
  if (request.GetMethod() == Method::kGET) {
    // only concern about carrying content when GET request
    bool resource_cached = cache->TryLoad(resource_full_path, cache_buf);
    if (!resource_cached) {
      // if content directly from cache, not disk file I/O
      // otherwise content not in cache, load from disk and try cache
      // it
      LoadFile(resource_full_path, cache_buf);
      std::ignore = cache->TryInsert(resource_full_path, cache_buf);
    }
  }
  // now cache_buf contains the file content anyway
  response_buf.insert(response_buf.end(), cache_buf.begin(), cache_buf.end());
  return request.ShouldClose();
}

auto HandleCGIRequest(
  const Request& request,
  const std::string& resource_full_path,
  DynamicByteArray& response_buf) -> bool /* should_finish */ {
  // dynamic CGI request
  CGIRunner cgi_runner = CGIRunner::ParseCGIRunner(resource_full_path);
  if (!cgi_runner.IsValid()) {
    auto response = Response::Make400Response();
    response.Serialize(response_buf);
    return true;
  }

  auto cgi_program_path = cgi_runner.GetPath();
  if (!IsFileExists(cgi_program_path)) {
    auto response = Response::Make404Response();
    response.Serialize(response_buf);
    return true;
  }

  auto cgi_result = cgi_runner.Run();
  auto response =
    Response::Make200Response(request.ShouldClose(), std::nullopt);
  std::ignore = response.ChangeHeader(
    kHeaderContentLength,
    std::to_string(cgi_result.size()));
  response.Serialize(response_buf);
  response_buf.insert(response_buf.end(), cgi_result.begin(), cgi_result.end());
  return request.ShouldClose();
}

void ProcessHttpRequest(
  const std::string_view serving_directory,
  std::shared_ptr<Cache>& cache,
  not_null<Connection*> client_connection) {
  Log<LogLevel::kInfo>("detect request");

  // edge-trigger, first read all available bytes
  int from_fd       = client_connection->GetFd();
  auto [read, exit] = client_connection->Receive();
  if (exit) {
    std::ignore = client_connection->GetLooper()->DeleteConnection(from_fd);

    Log<LogLevel::kInfo>(fmt::format("client fd={} has exited.", from_fd));

    // client_connection ptr is invalid below here, do not touch it again
    return;
  }

  // check if there is any complete http request ready
  bool finished_handle = false;

  for (
    auto request_op = client_connection->FindAndPopTill("\r\n\r\n");
    request_op != std::nullopt;
    request_op = client_connection->FindAndPopTill("\r\n\r\n")) {
    Request request{request_op.value()};
    DynamicByteArray response_buf;
    if (!request.IsValid()) {
      auto response   = Response::Make400Response();
      finished_handle = true;
      response.Serialize(response_buf);
    }
    else {
      std::string resource_full_path =
        fmt::format("{}{}", serving_directory, request.GetResourceUrl());

      Log<LogLevel::kInfo>(resource_full_path);
      if (IsCGIRequest(resource_full_path)) {
        finished_handle =
          HandleCGIRequest(request, resource_full_path, response_buf);
      }
      // normal http request - static resource request
      else {
        finished_handle = HandleStaticResourceRequest(
          request,
          resource_full_path,
          cache,
          response_buf);
      }
    }
    // send out the response
    client_connection->Write(std::move(response_buf));
    client_connection->Send();
    if (finished_handle) {
      break;
    }
  }

  if (finished_handle) {
    std::ignore = client_connection->GetLooper()->DeleteConnection(from_fd);
    // client_connection ptr is invalid below here, do not touch it again
    return;
  }
}
}    // namespace
}    // namespace longlp::http

auto main(int argc, char* argv[]) -> int {
  cxxopts::Options options("longlp-http-server", "A simple http/1.1 server");

  // clang-format off
  options.add_options()
    (
      "address",
      "server address",
      cxxopts::value<std::string>()->default_value("127.0.0.1")
    )
    ("port","server port", cxxopts::value<uint16_t>()->default_value("8080"))
    (
      "directory",
      "directory for resources, it should contains index.html",
      cxxopts::value<std::string>()
    )
    ("h,help", "Print usage")
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  if (result.count("help") != 0U) {
    fmt::print("{}\n", options.help());
    return 0;
  }

  std::string address = "127.0.0.1";
  if (result.count("address") != 0U) {
    address = result["address"].as<std::string>();
  }

  uint16_t port = 8080;
  if (result.count("port") != 0U) {
    port = result["port"].as<uint16_t>();
  }

  std::string directory = result["directory"].as<std::string>();
  if (!longlp::http::IsDirectoryExists(directory)) {
    fmt::print("not found directory {}\n", directory);
  }

  longlp::NetAddress net_address{address, port, longlp::Protocol::Ipv4};

  fmt::print("Setting up server on {}\n", net_address.ToString());

  longlp::Server http_server(net_address, std::thread::hardware_concurrency());
  auto cache = std::make_shared<longlp::Cache>(longlp::Cache::kDefaultCapacity);
  http_server
    .OnHandle([&](longlp::Connection* client_connection) {
      longlp::http::ProcessHttpRequest(directory, cache, client_connection);
    })
    .Begin();
  return 0;
}
