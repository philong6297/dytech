// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/net_address.h"

#include <fmt/format.h>
#include <array>
#include <cstring>

namespace longlp {

NetAddress::NetAddress() noexcept {
  memset(&addr_, 0, sizeof(addr_));
}

NetAddress::
  NetAddress(const std::string_view ip, uint16_t port, Protocol protocol) :
  protocol_(protocol) {
  memset(&addr_, 0, sizeof(addr_));

  if (protocol_ == Protocol::Ipv4) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* addr_ipv4       = reinterpret_cast<sockaddr_in*>(&addr_);
    addr_ipv4->sin_family = AF_INET;
    inet_pton(AF_INET, ip.data(), &addr_ipv4->sin_addr.s_addr);
    addr_ipv4->sin_port = htons(port);
  }
  else {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* addr_ipv6        = reinterpret_cast<sockaddr_in6*>(&addr_);
    addr_ipv6->sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip.data(), &addr_ipv6->sin6_addr.s6_addr);
    addr_ipv6->sin6_port = htons(port);
  }
}

auto NetAddress::GetIp() const noexcept -> std::string {
  // long enough for both Ipv4 and Ipv6
  std::string ip_address(INET6_ADDRSTRLEN, 0);

  if (protocol_ == Protocol::Ipv4) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* addr_ipv4 = reinterpret_cast<sockaddr_in*>(&addr_);
    inet_ntop(
      AF_INET,
      &addr_ipv4->sin_addr,
      ip_address.data(),
      INET_ADDRSTRLEN);
  }
  else {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* addr_ipv6 = reinterpret_cast<sockaddr_in6*>(&addr_);
    inet_ntop(
      AF_INET6,
      &addr_ipv6->sin6_addr,
      ip_address.data(),
      INET6_ADDRSTRLEN);
  }
  return ip_address;
}

auto NetAddress::GetPort() const noexcept -> uint16_t {
  uint16_t port{};
  if (protocol_ == Protocol::Ipv4) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* addr_ipv4 = reinterpret_cast<sockaddr_in*>(&addr_);
    port            = ntohs(addr_ipv4->sin_port);
  }
  else {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* addr_ipv6 = reinterpret_cast<sockaddr_in6*>(&addr_);
    port            = ntohs(addr_ipv6->sin6_port);
  }
  return port;
}

auto NetAddress::ToString() const noexcept -> std::string {
  return fmt::format(
    "{ip} @ {port}",
    fmt::arg("ip", GetIp()),
    fmt::arg("port", GetPort()));
}

auto operator<<(std::ostream& os, const NetAddress& address) -> std::ostream& {
  os << address.ToString();
  return os;
}
}    // namespace longlp
