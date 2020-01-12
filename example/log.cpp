//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  "log"_test = [] {
    boost::ut::log << "pre:" << 42;
    expect(42_i == 42) << "message on failure";
    boost::ut::log << "post";
  };
}
