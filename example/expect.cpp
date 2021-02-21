//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <array>
#include <boost/ut.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

constexpr auto sum = [](auto... args) { return (0 + ... + args); };

int main() {
  using namespace boost::ut;

  "operators"_test = [] {
    expect(0_i == sum());
    expect(2_i != sum(1, 2));
    expect(sum(1) >= 0_i);
    expect(sum(1) <= 1_i);
  };

  "message"_test = [] { expect(3_i == sum(1, 2)) << "wrong sum"; };

  "expressions"_test = [] {
    expect(0_i == sum() and 42_i == sum(40, 2));
    expect(1_i == sum() or 0_i == sum());
    expect(1_i == sum() or (sum() != 0_i or sum(1) > 0_i)) << "compound";
  };

  "that"_test = [] {
    expect(that % 0 == sum());
    expect(that % 42 == sum(40, 2) and that % (1 + 2) == sum(1, 2));
    expect(that % 1 != 2 or 2_i > 3);
  };

  "eq/neq/gt/ge/lt/le"_test = [] {
    expect(eq(42, sum(40, 2)));
    expect(neq(1, 2));
    expect(eq(sum(1), 1) and neq(sum(1, 2), 2));
    expect(eq(1, 1) and that % 1 == 1 and 1_i == 1);
  };

  "floating points"_test = [] {
    expect(42.1_d == 42.101) << "epsilon=0.1";
    expect(42.10_d == 42.101) << "epsilon=0.01";
    expect(42.10000001 == 42.1_d) << "epsilon=0.1";
  };

  "strings"_test = [] {
    using namespace std::literals::string_view_literals;
    using namespace std::literals::string_literals;

    expect("str"s == "str"s);
    expect("str1"s != "str2"s);

    expect("str"sv == "str"sv);
    expect("str1"sv != "str2"sv);

    expect("str"sv == "str"s);
    expect("str1"sv != "str2"s);
    expect("str"s == "str"sv);
    expect("str1"s != "str2"sv);
  };

  "types"_test = [] {
    expect(type<int> == type<int>);
    expect(type<float> != type<double>);

    const auto i = 0;
    expect(type<const int> == type<decltype(i)>);
    expect(type<decltype(i)> != type<int>);
  };

  "containers"_test = [] {
    std::vector v1{1, 2, 3};
    std::vector v2{1, 2, 3};
    expect(v1 == v2);
    expect(std::vector{"a", "b"} != std::vector{"c"});
    expect(std::array{true, false} == std::array{true, false});
  };

  "constant"_test = [] {
    constexpr auto compile_time_v = 42;
    auto run_time_v = 99;
    expect(constant<42_i == compile_time_v> and run_time_v == 99_i);
  };

  "convertible"_test = [] {
    expect(bool(std::make_unique<int>()));
    expect(not bool(std::unique_ptr<int>{}));
  };

  "boolean"_test = [] {
    expect("true"_b);
    expect("true"_b or not "true"_b);
    expect((not "true"_b) != "true"_b);
    expect("has value"_b == "value is set"_b);
  };
}
