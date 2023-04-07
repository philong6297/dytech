// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef CORE_NET_ADDRESS_H_
#define CORE_NET_ADDRESS_H_

#include <arpa/inet.h>

#include <bit>
#include <iostream>
#include <string>
#include <string_view>

namespace longlp {

// Network protocol supported
enum class Protocol {
  Ipv4,
  Ipv6
};

// This NetAddress class encapsulates the unique identifier of a network host in
// the form of "IP Address + Port"
// Compatible with both IPv4 and IPv6
class NetAddress {
 public:
  NetAddress() noexcept;

  NetAddress(std::string_view ip, uint16_t port, Protocol protocol);

  [[nodiscard]] auto GetProtocol() const noexcept -> Protocol {
    return protocol_;
  }

  [[nodiscard]] auto YieldAddr() -> sockaddr* {
    return std::bit_cast<sockaddr*>(&address_data_);
  }

  [[nodiscard]] auto YieldAddrLen() -> socklen_t* { return &addr_len_; }

  [[nodiscard]] auto GetIp() const noexcept -> std::string;

  [[nodiscard]] auto GetPort() const noexcept -> uint16_t;

  [[nodiscard]] auto ToString() const noexcept -> std::string;

 private:
  Protocol protocol_{Protocol::Ipv4};

  sockaddr_storage address_data_{};

  socklen_t addr_len_ = sizeof(address_data_);
};

[[nodiscard]] auto
operator<<(std::ostream& os, const NetAddress& address) -> std::ostream&;

}    // namespace longlp
#endif    // CORE_NET_ADDRESS_H_
