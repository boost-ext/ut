//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
import boost.ut;

int main() {
  using namespace boost::ut;

  "module"_test = [] {
    expect(42_i == 42 and constant<3 == 3_i>);
    expect(std::vector{1, 2, 3} == std::vector{1, 2, 3});
  };
}
