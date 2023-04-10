// Copyright 2023 Phi-Long Le. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "http/response.h"

#include <catch2/catch_test_macros.hpp>
#include "http/constants.h"
#include "http/header.h"

namespace {
using longlp::http::kHeaderContentLength;
using longlp::http::kResponseStatusOK;
using longlp::http::Response;
}    // namespace

TEST_CASE("[http/response]") {
  SECTION("response should be able to modify header on the fly") {
    std::string status = "200 Success";
    Response
      response{kResponseStatusOK, false, std::string("nonexistent-file.txt")};
    auto headers = response.GetHeaders();
    bool find    = false;
    for (auto& h : headers) {
      if (h.GetKey() == kHeaderContentLength) {
        find = true;
      }
    }
    CHECK(find);
    std::string new_val = "1024";
    CHECK(response.ChangeHeader(kHeaderContentLength, new_val));
    find = false;
    std::string value;
    headers = response.GetHeaders();
    for (auto& h : headers) {
      if (h.GetKey() == kHeaderContentLength) {
        find  = true;
        value = h.GetValue();
      }
    }
    CHECK(find);
    CHECK(value == new_val);
  }
}
