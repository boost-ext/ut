//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

namespace ut = boost::ut;

namespace custom {
class runner {
 public:
  template <class... Ts>
  auto on(ut::events::test_run<Ts...> test) {
    test.test();
  }
  template <class... Ts>
  auto on(ut::events::test_skip<Ts...>) {}
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr>) -> bool {
    return true;
  }
  auto on(ut::events::fatal_assertion) {}
  auto on(ut::events::log) {}
};
}  // namespace custom

template <>
auto ut::cfg<ut::override> = custom::runner{};

int main() {
  using namespace ut;
  "should be ignored"_test = [] { expect(1_i == 2) << "doesn't fire"; };
}
