//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

int main() {
  using namespace boost::ut;

  /// Language syntax
  for (auto i : std::vector{1, 2, 3}) {
    test("args / " + std::to_string(i)) = [i] {  // 3 tests
      expect(that % i > 0);                      // 3 asserts
    };
  }

  /// Alternative syntax
  "args"_test = [](auto arg) {
    expect(arg > 0_i) << "all values greater than 0";
  } | std::vector{1, 2, 3};

  /// Alternative syntax
  "types"_test = []<class T>() {
    expect(std::is_integral_v<T>) << "all types are integrals";
  } | std::tuple<bool, int>{};

  /// Language syntax
  std::apply(
      [](auto... args) {
        ((test("args and types / " + std::to_string(args)) =
              [&] {
                using TArgs = decltype(args);
                expect((std::is_integral_v<TArgs>) >> fatal);
                expect(42_i == static_cast<int>(args) or args);
                expect(type<TArgs> == type<int> or type<TArgs> == type<bool>);
              }),
         ...);
      },
      std::tuple{42, true});

  /// Alternative syntax
  "args and types"_test = []<class TArg>(TArg arg) {
    expect((std::is_integral_v<TArg>) >> fatal);
    expect(42_i == static_cast<int>(arg) or arg);
    expect(type<TArg> == type<int> or type<TArg> == type<bool>);
  } | std::tuple{42, true};
}
