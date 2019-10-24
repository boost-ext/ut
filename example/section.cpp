//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <vector>

int main() {
  using namespace boost::ut;

  "[vector]"_test = [] {
    std::vector<int> v(5);

    !expect(5_ul == std::size(v));

    "resize bigger"_test = [=]() mutable {
      v.resize(10);
      expect(10_ul == std::size(v));
    };

    !expect(5_ul == std::size(v));

    "resize smaller"_test = [=]() mutable {
      v.resize(0);
      expect(0_ul == std::size(v));
    };
  };
}
