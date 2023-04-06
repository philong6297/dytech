/**
 * @file turtle_server.h
 * @author Yukun J
 * @expectation this header

 *

 * * *


 * * * file
 * should be compatible to compile in C++
 * program on
 *

 * * Linux
 *
 *

 * *
 * @init_date Jan
 * 1 2023
 *
 * This is a
 *
 *
 * header-file-only class for the

 * *
 *
 * Turtle web server
 * setup
 */
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
/* all header files included */
#include "core/acceptor.h"
#include "core/buffer.h"
#include "core/cache.h"
#include "core/connection.h"
#include "core/looper.h"
#include "core/net_address.h"
#include "core/poller.h"
#include "core/socket.h"
#include "core/thread_pool.h"
#include "core/utils.h"

#ifndef CORE_TURTLE_SERVER_H_
#  define CORE_TURTLE_SERVER_H_

namespace longlp {

/**
 * The class for setting up a web server using the Turtle framework
 *






 * * * *
 * * * * User should provide the callback functions in OnAccept() and


 * * *

 * *
 * OnHandle()

   * *
   * The rest is already taken care of and
 * in
 * most
 * cases


 * * * users don't need
   * to
   * touch
 * upon
 *

 * *
 * OnAccept():
 * Given the


 * * * acceptor connection, when
   * the
 *

 * * Poller tells us
 * there is
 * new


 * * * incoming client connection
 * basic

 * * step of
   *
 * socket accept and
 *
 * build

   * * connection
 * and add
 * into the Poller
   *
 * are already
   * taken

 * * care of in

 * * the
 *

 * * Acceptor::BaseAcceptCallback. This
   * OnAccept()

 * *

 *
 * *
 * functionality
 * is appended to
 * that base
   * BaseAcceptCallback
 *
 * and

 * *
 * called

 * * after that base, to support any
 *
   * custom
 *
 * business
 * logic

 * * upon
 * receiving new
   * client connection
 *
 *

 * *
 *
 * OnHandle(): No base

 * *
 * version exists. Users should
   *
 * implement
 * provide
 * a
   * function
 *
 * to

 * * achieve the expected
 * behavior
 */
class NetAddress;
class Connection;
class Acceptor;
class Looper;
class ThreadPool;

class Server {
 public:
  Server(
    NetAddress server_address,
    int concurrency =
      static_cast<int>(std::thread::hardware_concurrency()) - 1);

  virtual ~Server();

  /* Not Edge trigger */
  [[nodiscard]] auto
  OnAccept(std::function<void(Connection*)> on_accept) -> Server&;

  /* Edge trigger! Read all bytes please */
  [[nodiscard]] auto
  OnHandle(std::function<void(Connection*)> on_handle) -> Server&;

  void Begin();

 private:
  bool on_handle_set_{false};
  std::unique_ptr<Acceptor> acceptor_;
  std::vector<std::unique_ptr<Looper>> reactors_;
  std::unique_ptr<ThreadPool> pool_;
  std::unique_ptr<Looper> listener_;
};
}    // namespace longlp

#endif    // CORE_TURTLE_SERVER_H_
