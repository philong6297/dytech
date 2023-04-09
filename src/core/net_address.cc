// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/net_address.h"

#include <fmt/format.h>
#include <array>
#include <cstring>

namespace longlp {

NetAddress::NetAddress() noexcept {
  memset(&address_data_, 0, sizeof(address_data_));
}

NetAddress::
  NetAddress(const std::string_view ip, uint16_t port, Protocol protocol) :
  protocol_(protocol) {
  memset(&address_data_, 0, sizeof(address_data_));

  switch (protocol_) {
    case Protocol::Ipv4:
      {
        auto* as_ipv4       = bit_cast<sockaddr_in*>(&address_data_);
        as_ipv4->sin_family = AF_INET;
        inet_pton(AF_INET, ip.data(), &as_ipv4->sin_addr.s_addr);
        as_ipv4->sin_port = htons(port);
      }
      break;
    case Protocol::Ipv6:
      {
        auto* as_ipv6        = bit_cast<sockaddr_in6*>(&address_data_);
        as_ipv6->sin6_family = AF_INET6;
        inet_pton(AF_INET6, ip.data(), &as_ipv6->sin6_addr.s6_addr);
        as_ipv6->sin6_port = htons(port);
      }
      break;
  }
}

auto NetAddress::GetIp() const noexcept -> std::string {
  // long enough for both Ipv4 and Ipv6
  std::string ip_address(INET6_ADDRSTRLEN, 0);

  switch (protocol_) {
    case Protocol::Ipv4:
      {
        const auto* as_ipv4 = bit_cast<const sockaddr_in*>(&address_data_);
        inet_ntop(
          AF_INET,
          &as_ipv4->sin_addr,
          ip_address.data(),
          INET_ADDRSTRLEN);
      }
      break;
    case Protocol::Ipv6:
      {
        const auto* as_ipv6 = bit_cast<const sockaddr_in6*>(&address_data_);
        inet_ntop(
          AF_INET6,
          &as_ipv6->sin6_addr,
          ip_address.data(),
          INET6_ADDRSTRLEN);
      }
      break;
  }

  return ip_address;
}

auto NetAddress::GetPort() const noexcept -> uint16_t {
  if (protocol_ == Protocol::Ipv4) {
    const auto* as_ipv4 = bit_cast<const sockaddr_in*>(&address_data_);
    return ntohs(as_ipv4->sin_port);
  }

  // IP v6

  const auto* as_ipv6 = bit_cast<const sockaddr_in6*>(&address_data_);
  return ntohs(as_ipv6->sin6_port);
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
