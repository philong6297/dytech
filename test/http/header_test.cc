// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/header.h"

#include <catch2/catch_test_macros.hpp>

#include "http/constants.h"

namespace {
using longlp::http::Header;
using longlp::http::kCRLF;
}    // namespace

TEST_CASE("[http/header]") {
  SECTION("normal constructor method for header") {
    std::string k  = "http_version";
    std::string v1 = "1.1";
    std::string v2 = "1.2";
    Header h1(k, v1);
    CHECK(h1.IsValid());
    CHECK(h1.GetKey() == k);
    CHECK(h1.GetValue() == v1);

    h1.SetValue(v2);
    CHECK(h1.GetValue() == v2);
  }

  SECTION("serialize/deserialize a header from string") {
    std::string valid_header_line   = "Host:YukunJ";
    std::string invalid_header_line = "Hello& No Way!";
    Header h1(valid_header_line);
    Header h2(invalid_header_line);

    CHECK(h1.IsValid());
    CHECK(h1.GetKey() == "Host");
    CHECK(h1.GetValue() == "YukunJ");

    // no colon in this header line, is not valid
    CHECK(!h2.IsValid());

    std::string serialized = h1.Serialize();
    CHECK(serialized == (valid_header_line + std::string(kCRLF)));
  }
}
