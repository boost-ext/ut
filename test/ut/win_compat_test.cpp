//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

// ensure no conflict between `Windows.h` and `ut.hpp`
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "boost/ut.hpp"

namespace ut = boost::ut;

int main() {
  using namespace ut;
  expect(true);
}
