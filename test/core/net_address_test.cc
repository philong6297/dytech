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
    constexpr std::string_view ip = "127.0.0.1";
    NetAddress address_ipv4(ip, 20080, Protocol::Ipv4);
    CHECK(address_ipv4.GetProtocol() == Protocol::Ipv4);

    const auto ip_as_string = address_ipv4.GetIp();
    CHECK(ip_as_string == ip);
    CHECK(ip_as_string.size() == ip.size());

    CHECK(address_ipv4.GetPort() == 20080);

    constexpr std::string_view expected_format = "127.0.0.1 @ 20080";
    const auto actual_format                   = address_ipv4.ToString();
    CHECK(actual_format == expected_format);
    CHECK(actual_format.size() == expected_format.size());
  }

  SECTION("Ipv6 net address") {
    constexpr std::string_view ip = "2001:db8:3333:4444:5555:6666:7777:8888";
    NetAddress address_ipv6(ip, 40080, Protocol::Ipv6);
    CHECK(address_ipv6.GetProtocol() == Protocol::Ipv6);

    const auto ip_as_string = address_ipv6.GetIp();
    CHECK(ip_as_string == ip);
    CHECK(ip_as_string.size() == ip.size());

    CHECK(address_ipv6.GetPort() == 40080);

    constexpr std::string_view expected_format =
      "2001:db8:3333:4444:5555:6666:7777:8888 @ 40080";
    const auto actual_format = address_ipv6.ToString();
    CHECK(actual_format == expected_format);
    CHECK(actual_format.size() == expected_format.size());
  }
}
