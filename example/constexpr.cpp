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
class constexpr_runner {
  template <class...>
  struct item {
    friend constexpr auto get(item);
  };

  template <class TKey, class TValue>
  struct set {
    friend constexpr auto get(TKey) { return TValue{}; }
  };

 public:
  template <class... Ts>
  constexpr auto on(ut::events::test<Ts...>) const {}

  template <class TLocation, class TExpr>
  constexpr auto on(ut::events::assertion<TLocation, TExpr> assertion) const
      -> bool {
    void(set<item<>, decltype(assertion)>{});
    return assertion.expr;
  }

  template <class... Ts>
  constexpr auto run() const {
    static_assert(get(item<Ts...>{}).expr);
  }
};
}  // namespace cfg

template <>
constexpr auto ut::cfg<ut::override> = cfg::constexpr_runner{};

int main() {
  using namespace boost::ut;

  "constexpr"_test = [] { expect(type<int> == type<int>); };

  ut::cfg<ut::override>.run();
}
