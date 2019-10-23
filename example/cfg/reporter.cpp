//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

namespace ut = boost::ut;

namespace cfg {
class reporter {
 public:
  auto on(ut::events::test_begin) -> void {}
  auto on(ut::events::test_run) -> void {}
  auto on(ut::events::test_skip) -> void {}
  auto on(ut::events::test_end) -> void {}
  auto on(ut::events::log) -> void {}
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion_pass<TLocation, TExpr>) -> void {}
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion_fail<TLocation, TExpr>) -> void {}
  auto on(ut::events::fatal_assertion) -> void {}
  auto on(ut::events::exception) -> void {}
  auto on(ut::events::summary) -> void{};
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<cfg::reporter>{};

int main() {
  using namespace ut;
  "example"_test = [] { expect(42 == 42_i); };
}
