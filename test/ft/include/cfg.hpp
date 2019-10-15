//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <iostream>

namespace ut = boost::ut;

namespace ft {
struct cfg : ut::default_cfg {
  template <class Test>
  auto on(ut::events::test_run<Test> test) {
    std::cout << test.name << '\n';
    test.test();
  }

  using ut::default_cfg::on;
};
}  // namespace ft

template <>
inline auto ut::cfg<ut::override> = ft::cfg{};

using ut::operator""_test;
