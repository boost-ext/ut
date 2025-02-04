//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <complex>
#include <concepts>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace boost::inline ext::ut {

namespace {
template <std::floating_point F>
std::string format_test_parameter(const std::complex<F>& arg,
                                         [[maybe_unused]] const int counter) {
  std::ostringstream oss;
  oss << arg.real() << '+' << arg.imag() << 'i';
  return oss.str();
}
}  // namespace

} // namespace boost::inline ext::ut

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
#ifndef __EMSCRIPTEN__
  "views"_test = [](auto arg) {
    expect(arg > 0_i) << "all values greater than 0";
  } | std::views::iota(1, 4);
#endif

  /// Alternative syntax
  "types"_test = []<class T>() {
    expect(std::is_integral_v<T>) << "all types are integrals";
  } | std::tuple<bool, int>{};

  /// Language syntax
  std::apply(
      []<class... TArgs>(TArgs... args) {
        ((test("args and types / " + std::to_string(args)) =
              [&] {
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

  // Modifying test names when using alternative syntax
  // When using the alternative syntax, the test names are extended based on the
  // test parameters (to ensure uniqueness). Here, for simple built-in types,
  // the parameter value is printed, while other types are simply enumerated.
  // Without the `format_test_parameter` overload above, the test names for the
  // test below would be:
  //   "parameterized test names (42, int)"
  //   "parameterized test names (true, bool)"
  //   "parameterized test names (3rd parameter, std::complex<double>)"
  // However, since the overload for std::complex is available, the third test name becomes:
  //   "parameterized test names (1.5+2i, std::complex<double>)"
  "parameterized test names"_test = []<class TArg>([[maybe_unused]] TArg arg) {
    expect(true);
  } | std::tuple{42, true, std::complex{1.5, 2.0}};
}
