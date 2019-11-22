//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/ut.hpp>
#include <string_view>

int main() {
  using namespace boost::ut;
  using namespace std::literals::string_view_literals;

  "matcher"_test = [] {
    constexpr auto is_between = [](auto lhs, auto rhs) {
      return matcher([=](auto value) {
        return that % value >= lhs and that % value <= rhs;
      });
    };

    constexpr auto ends_with = matcher([](const auto& arg, const auto& ext) {
      log << "Ends with" << ext << arg;
      if (ext.size() > arg.size()) {
        return false;
      }
      return std::equal(ext.rbegin(), ext.rend(), arg.rbegin());
    });

    auto value = 42;
    auto str = "example.test"sv;

    expect(is_between(1, 100)(value) and ends_with(str, ".test"sv));
  };
}
