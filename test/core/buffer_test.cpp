// Copyright 2023 Long Le Phi. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "core/buffer.h"

#include <catch2/catch_test_macros.hpp>
#include <string_view>
#include <vector>

/* for convenience reason */
using longlp::Buffer;

TEST_CASE("[core/buffer]") {
  Buffer buf(Buffer::kDefaultCapacity);
  REQUIRE(buf.Size() == 0);
  REQUIRE(buf.Capacity() == Buffer::kDefaultCapacity);

  SECTION("appending c-str into buffer from both side") {
    constexpr std::string_view msg1 = "Greeting from beginning!";
    constexpr std::string_view msg2 = "Farewell from end~";

    std::vector<uint8_t> buff_1;
    for (auto c : msg1) {
      buff_1.emplace_back(c);
    }

    std::vector<uint8_t> buff_2;
    for (auto c : msg2) {
      buff_2.emplace_back(c);
    }

    buf.AppendUnsafe(buff_1.data(), buff_1.size());
    buf.AppendHeadUnsafe(buff_2.data(), buff_2.size());
    CHECK(buf.ToStringView() == msg1);
    buf.Clear();
    CHECK(buf.Size() == 0);
  }

  SECTION("appending std::string into buffer from both side") {
    const std::string msg1 = "Greeting from beginning!";
    const std::string msg2 = "Farewell from end~";
    buf.AppendHead(msg1);
    buf.Append(msg2);
    const std::string together = msg1 + msg2;
    CHECK(buf.ToStringView() == together);
    buf.Clear();
    CHECK(buf.Size() == 0);
  }

  SECTION("find and pop based on the target first found") {
    const std::string msg =
      "GET / HTTP/1.1\r\n"
      "Connection: Keep-Alive\r\n"
      "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
      "Accept-Language: en-us\r\n"
      "\r\n";
    const std::string next_msg = "Something belongs to next message";
    buf.Append(msg);
    buf.Append(next_msg);
    auto op_str = buf.FindAndPopTill("\r\n\r\n");
    CHECK((op_str.has_value() && op_str.value() == msg));
    CHECK(buf.ToStringView() == next_msg);
  }
}
