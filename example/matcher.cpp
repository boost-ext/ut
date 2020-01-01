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

struct expr {
  const bool result{};
  const std::string str{};

  constexpr explicit operator bool() const { return result; }
  friend auto operator<<(std::ostream& os, const expr& self) -> std::ostream& {
    return (os << self.str);
  }
};

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
      std::stringstream str{};
      str << '(' << arg << " ends with " << ext << ')';
      if (ext.size() > arg.size()) {
        return expr{{}, str.str()};
      }
      return expr{std::equal(ext.rbegin(), ext.rend(), arg.rbegin()),
                  str.str()};
    });

    auto value = 42;
    auto str = "example.test"sv;

    expect(is_between(1, 100)(value) and ends_with(str, ".test"sv));
    expect(not is_between(1, 100)(0));
  };
}
