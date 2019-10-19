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

namespace ft {
struct cfg : boost::ut::default_cfg {
  template <class... Ts>
  auto on(boost::ut::events::test_run<Ts...> test) {
    std::cout << test.name << '\n';
    test.test();
  }

  using boost::ut::default_cfg::on;
};
}  // namespace ft

template <>
#if defined(_MSC_VER)
static
#endif
    inline auto boost::ut::cfg<boost::ut::override> = ft::cfg{};

using boost::ut::operator""_test;
