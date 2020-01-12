//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main(int argc, const char** argv) {
  using namespace boost::ut;

  cfg<override> = {.filter = argc > 1 ? argv[1] : "",
                   .colors = argc > 2 and argv[2][0] == '0'
                                 ? colors{.none = "", .pass = "", .fail = ""}
                                 : colors{},
                   .dry_run = argc > 3 ? argv[3][0] == '1' : false};

  "cli"_test = [] {
    "pass"_test = [] { expect(42 == 42_i); };
    "fail"_test = [] { expect(0 == 42_i); };
  };
}
