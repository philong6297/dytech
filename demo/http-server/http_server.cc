/**
 * @file http_server.cpp
 * @author Yukun J
 * @expectation this is
 *


 * *


 * * * *



 * * * *
 * the





 * * * *
 * *

 * * *


 * * * http
 *
 * server

 * * for


 * * *
 * illustration

 * * and


 * * * test
 *
 * purpose

 * *

 *
 * *

 * *
 *
 * @init_date


 * *
 * * Jan
 * 3
 *
 *
 *
 * 2023

 */

#include <system_error>
#include "core/turtle_server.h"
#include "http/cgier.h"
#include "http/header.h"
#include "http/http_utils.h"
#include "http/request.h"
#include "http/response.h"
#include "log/logger.h"

namespace longlp::HTTP {

void ProcessHttpRequest(
  const std::string& serving_directory,
  std::shared_ptr<Cache>& cache,
  Connection* client_conn) {
  Log<LogLevel::kInfo>("detect request");
  // edge-trigger, first read all available bytes
  int from_fd       = client_conn->GetFd();
  auto [read, exit] = client_conn->Recv();
  if (exit) {
    client_conn->GetLooper()->DeleteConnection(from_fd);
    Log<LogLevel::kInfo>(
      "client fd=" + std::to_string(from_fd) + " has exited");
    // client_conn ptr is invalid below here, do not touch it again
    return;
  }
  // check if there is any complete http request ready
  bool no_more_parse = false;
  std::optional<std::string> request_op =
    client_conn->FindAndPopTill("\r\n\r\n");
  while (request_op != std::nullopt) {
    Request request{request_op.value()};
    ByteData response_buf;
    if (!request.IsValid()) {
      auto response = Response::Make400Response();
      no_more_parse = true;
      response.Serialize(response_buf);
    }
    else {
      std::string resource_full_path =
        serving_directory + request.GetResourceUrl();
      Log<LogLevel::kInfo>(resource_full_path);
      if (IsCgiRequest(resource_full_path)) {
        // dynamic CGI request
        Cgier cgier = Cgier::ParseCgier(resource_full_path);
        if (!cgier.IsValid()) {
          auto response = Response::Make400Response();
          no_more_parse = true;
          response.Serialize(response_buf);
        }
        else {
          auto cgi_program_path = cgier.GetPath();
          if (!IsFileExists(cgi_program_path)) {
            auto response = Response::Make404Response();
            no_more_parse = true;
            response.Serialize(response_buf);
          }
          else {
            auto cgi_result = cgier.Run();
            auto response =
              Response::Make200Response(request.ShouldClose(), std::nullopt);
            response.ChangeHeader(
              HEADER_CONTENT_LENGTH,
              std::to_string(cgi_result.size()));
            no_more_parse = request.ShouldClose();
            response.Serialize(response_buf);
            response_buf
              .insert(response_buf.end(), cgi_result.begin(), cgi_result.end());
          }
        }
      }
      else {
        // static resource request
        if (!IsFileExists(resource_full_path)) {
          Log<LogLevel::kInfo>(resource_full_path + " not exist");
          auto response = Response::Make404Response();
          no_more_parse = true;
          response.Serialize(response_buf);
        }
        else {
          auto response = Response::
            Make200Response(request.ShouldClose(), resource_full_path);
          response.Serialize(response_buf);
          no_more_parse = request.ShouldClose();
          ByteData cache_buf;
          if (request.GetMethod() == Method::GET) {
            // only concern about carrying content when GET request
            bool resource_cached =
              cache->TryLoad(resource_full_path, cache_buf);
            if (!resource_cached) {
              // if content directly from cache, not disk file I/O
              // otherwise content not in cache, load from disk and try cache
              // it
              LoadFile(resource_full_path, cache_buf);
              cache->TryInsert(resource_full_path, cache_buf);
            }
          }
          // now cache_buf contains the file content anyway
          response_buf
            .insert(response_buf.end(), cache_buf.begin(), cache_buf.end());
        }
      }
    }
    // send out the response
    client_conn->WriteToWriteBuffer(std::move(response_buf));
    client_conn->Send();
    if (no_more_parse) {
      break;
    }
    request_op = client_conn->FindAndPopTill("\r\n\r\n");
  }
  if (no_more_parse) {
    client_conn->GetLooper()->DeleteConnection(from_fd);
    // client_conn ptr is invalid below here, do not touch it again
    return;
  }
}
}    // namespace longlp::HTTP

int main(int argc, char* argv[]) {
  const std::string usage =
    "Usage: \n"
    "./http_server [optional: port default=20080] [optional: directory "
    "default=../http_dir/] \n";
  std::cout << usage;
  if (argc > 3) {
    std::cout << "argument number error\n";
    exit(EXIT_FAILURE);
  }

  longlp::NetAddress address("127.0.0.1", 8080);
  std::string directory = "/home/longlp/dytech/build/demo/http-server/pages";
  if (argc >= 2) {
    auto port = static_cast<uint16_t>(std::strtol(argv[1], nullptr, 10));
    if (port == 0) {
      std::cout << "port error\n";
      std::cout << usage;
      exit(EXIT_FAILURE);
    }
    address = {"127.0.0.1", port};
    if (argc == 3) {
      directory = argv[2];
      if (!longlp::HTTP::IsDirectoryExists(directory)) {
        std::cout << "directory error\n";
        std::cout << usage;
        exit(EXIT_FAILURE);
      }
    }
  }
  longlp::Server http_server(address);
  auto cache = std::make_shared<longlp::Cache>();
  http_server
    .OnHandle([&](longlp::Connection* client_conn) {
      longlp::HTTP::ProcessHttpRequest(directory, cache, client_conn);
    })
    .Begin();
  return 0;
}
