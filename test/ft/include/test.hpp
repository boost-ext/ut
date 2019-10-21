//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/ut.hpp>
#include <iostream>

namespace test = boost::ut;

namespace ft {
struct cfg : test::default_cfg {
  template <class... Ts>
  auto on(test::events::test_run<Ts...> test) {
    std::cout << test.name << '\n';
    test.test();
  }

  using test::default_cfg::on;
};
}  // namespace ft

template <class... Ts>
static auto test::cfg<test::override, Ts...> = ft::cfg{};

using test::operator""_test;
