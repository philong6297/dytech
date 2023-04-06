/**
 * @file socket.h
 * @author Yukun J
 * @expectation this header file
 *

 * *

 * * should
 * be compatible to compile in C++
 * program on Linux
 *
 *
 * @init_date


 * * * Dec 25
 * 2022
 *
 * This is a header file implementing

 * * the Socket,
 * which

 * * acts as
 * either the
 * listener or client
 */

#ifndef CORE_SOCKET_H_
#define CORE_SOCKET_H_

#include "core/utils.h"

namespace longlp {

class NetAddress;

enum class Protocol;

/**
 * This Socket class encapsulates a socket descriptor
 * which can act
 *


 * * * as

   * * either listener or client
 * This class is compatible with
 *
 * both

 * * IPv4 and

   * * IPv6
 * */
class Socket {
 public:
  Socket() noexcept;

  explicit Socket(int fd) noexcept;

  DISALLOW_COPY(Socket);

  Socket(Socket&& other) noexcept;

  [[nodiscard]] auto operator=(Socket&& other) noexcept -> Socket&;

  ~Socket();

  [[nodiscard]] auto GetFd() const noexcept -> int;

  /* for client, one step: directly connect */
  void Connect(NetAddress& server_address);

  /* for server, three steps: bind + listen + accept */
  void Bind(NetAddress& server_address, bool set_reusable = true);

  void Listen();

  [[nodiscard]] auto Accept(NetAddress& client_address) -> int;

  void SetReusable();

  void SetNonBlocking();

  [[nodiscard]] auto GetAttrs() -> int;

 private:
  void CreateByProtocol(Protocol protocol);

  int fd_{-1};
};
}    // namespace longlp
#endif    // CORE_SOCKET_H_
