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

  "sl"_test = [] {
    constexpr auto verify = [](auto sl, auto i) {
      expect(i == 42_i, sl) << "error with given source location";
    };

    verify(boost::ut::reflection::source_location::current(), 42_i);
  };
}
