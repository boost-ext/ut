//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "test.hpp"

static test::suite _ = [] {
  using namespace test;
  test::log << "suite:" << int() << double() << '\n';
  expect(type<int> == type<int>);
};
