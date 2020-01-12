//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

template <class...>
struct list {};

int main() {
  using namespace boost::ut;

  static constexpr auto i = 42;

  "tmp"_test = [] {
    expect(constant<42_i == i> and type<void> == type<void> and
           type<list<void, int>> == type<list<void, int>>);

    expect(type<>(i) == type<int>);
  };
}
