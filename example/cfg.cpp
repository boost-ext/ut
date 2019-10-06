//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

class silent_cfg {
 public:
  template <class Test>
  auto on(ut::events::test_run<Test> test) {
    test.test();
  }
  template <class Test>
  auto on(ut::events::test_skip<Test>) {}
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr>) -> bool {
    return true;
  }
  auto on(ut::events::fatal_assertion) {}
  auto on(ut::events::log) {}
};

template <>
auto ut::cfg<ut::override> = silent_cfg{};

int main() {
  using namespace ut;
  "silent"_test = [] { expect(1_i == 2) << "ignored by config"; };
}
