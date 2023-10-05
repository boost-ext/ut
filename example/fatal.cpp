//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <optional>
#include <vector>

int main() {
  using boost::ut::operator""_test;
  using namespace boost::ut::literals;
  using boost::ut::fatal;

  "fatal"_test = [] {
    using namespace boost::ut::operators;
    using boost::ut::expect;

    std::optional<int> o{42};
    expect(fatal(o.has_value())) << "fatal assertion";
    expect(*o == 42_i);
  };

  "fatal logging"_test = [] {
    using namespace boost::ut::operators;
    using boost::ut::expect;

    std::optional<int> o{42};
    expect(fatal(o.has_value())) << "fatal assertion";
    expect(*o == 42_i);
  };

  "fatal matcher"_test = [] {
    using namespace boost::ut::operators;
    using boost::ut::expect;
    using boost::ut::that;

    std::optional<int> o{42};
    expect(fatal(that % o.has_value()) and that % *o == 42)
        << "fatal assertion";
  };

  "fatal terse"_test = [] {
    using namespace boost::ut::operators::terse;

    std::optional<int> o{42};
    (o.has_value() >> fatal and *o == 42_i) << "fatal assertion";
  };

  using namespace boost::ut::operators;
  using boost::ut::expect;

  std::vector v{1u};
  expect(fatal(std::size(v) == 1_ul)) << "fatal assertion";
  expect(v[0] == 1_u);
}
