//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main(int argc, const char** argv) {
  using namespace boost::ut;

  cfg<override> = {/*.filter =*/argc > 1 ? argv[1] : ""};

  "pass"_test = [] { expect(42 == 42_i); };
  "fail"_test = [] { expect(0 == 42_i); };
}
