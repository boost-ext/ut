//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <stdexcept>

namespace ut = boost::ut;

namespace cfg {
class runner {
 public:
  template <class... Ts>
  auto on(ut::events::test<Ts...> test) {
    test();
  }
  template <class... Ts>
  auto on(ut::events::skip<Ts...>) {}
  template <class TExpr>
  auto on(ut::events::assertion<TExpr>) -> bool {
    return true;
  }
  auto on(ut::events::fatal_assertion) {}

  template <class TMsg>
  auto on(ut::events::log<TMsg>) {}
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = cfg::runner{};

int main() {
  using namespace ut;

  "should be ignored"_test = [] {
    expect(throws([] { throw std::runtime_error{"exception!"}; }));
    expect(1_i == 2) << "doesn't fire";
  };
}
