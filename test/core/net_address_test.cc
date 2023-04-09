// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/net_address.h"

#include <catch2/catch_test_macros.hpp>

namespace {
using longlp::NetAddress;
using longlp::Protocol;
}    // namespace

TEST_CASE("[core/net_address]") {
  SECTION("Ipv4 net address") {
    NetAddress address_ipv4("127.0.0.1", 20080, Protocol::Ipv4);
    REQUIRE(address_ipv4.GetProtocol() == Protocol::Ipv4);
    REQUIRE(address_ipv4.GetIp() == "127.0.0.1");
    REQUIRE(address_ipv4.GetPort() == 20080);
    REQUIRE(address_ipv4.ToString() == "127.0.0.1 @ 20080");
  }

  SECTION("Ipv6 net address") {
    NetAddress address_ipv6(
      "2001:db8:3333:4444:5555:6666:7777:8888",
      40080,
      Protocol::Ipv6);
    REQUIRE(address_ipv6.GetProtocol() == Protocol::Ipv6);
    REQUIRE(address_ipv6.GetIp() == "2001:db8:3333:4444:5555:6666:7777:8888");
    REQUIRE(address_ipv6.GetPort() == 40080);
    REQUIRE(
      address_ipv6.ToString() ==
      "2001:db8:3333:4444:5555:6666:7777:8888 @ 40080");
  }
}
