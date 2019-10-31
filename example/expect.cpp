//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <string>
#include <string_view>
#include <vector>

constexpr auto sum = [](auto... args) { return (0 + ... + args); };

int main() {
  using namespace boost::ut;

  "const"_test = [] {
    expect(42_l == 42_l);
    static_assert(42_l == 42_l);
  };

  "operators"_test = [] {
    expect(0_i == sum());
    expect(2_i != sum(1, 2));
    expect(sum(1) >= 0_i);
    expect(sum(1) <= 1_i);
  };

  "expressions"_test = [] {
    expect(0_i == sum() and 42_i == sum(40, 2));
    expect(1_i == sum() or 0_i == sum());
    expect(1_i == sum() or (sum() != 0_i or sum(1) > 0_i)) << "compound";
  };

  "message"_test = [] { expect(3_i == sum(1, 2)) << "wrong sum"; };

  "floating points"_test = [] {
    expect(42.1_d == 42.101) << "epsilon=0.1";
    expect(42.10_d == 42.101) << "epsilon=0.01";
    expect(42.10000001 == 42.1_d) << "epsilon=0.1";
  };

  "fatal"_test = [] {
    std::vector v{1, 2, 3};
    !expect(std::size(v) == 3_ul) << "fatal assertion";
    expect(v[0] == 1_i);
    expect(v[1] == 2_i);
    expect(v[2] == 3_i);
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
  };
}
