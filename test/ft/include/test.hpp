//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/ut.hpp>

namespace test = boost::ut;

#include <iostream>

namespace ft {
template <class TReporter>
struct runner : test::runner<TReporter> {
  template <class... Ts>
  auto on(test::events::test<Ts...> test) {
    std::cout << test.name << '\n';
    test::runner<TReporter>::on(test);
  }

  using test::runner<TReporter>::on;
};
}  // namespace ft

template <class... Ts>
inline auto test::cfg<test::override, Ts...> = ft::runner<test::reporter<>>{};
