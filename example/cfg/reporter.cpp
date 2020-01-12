//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <string_view>

namespace ut = boost::ut;

namespace cfg {
class reporter {
 public:
  auto on(ut::events::test_begin) -> void {}
  auto on(ut::events::test_run) -> void {}
  auto on(ut::events::test_skip) -> void {}
  auto on(ut::events::test_end) -> void {}
  template <class TMsg>
  auto on(ut::events::log<TMsg>) -> void {}
  template <class TExpr>
  auto on(ut::events::assertion_pass<TExpr>) -> void {}
  template <class TExpr>
  auto on(ut::events::assertion_fail<TExpr>) -> void {}
  auto on(ut::events::fatal_assertion) -> void {}
  auto on(ut::events::exception) -> void {}
  auto on(ut::events::summary) -> void {}
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<cfg::reporter>{};

int main() {
  using namespace ut;
  using namespace std::literals::string_view_literals;

  "example"_test = [] {
    expect(42 == 42_i);
    expect("ut"sv != "tu"sv);
  };
}
