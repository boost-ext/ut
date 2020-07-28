//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <vector>

#define EXPECT(...)            \
  using namespace ::boost::ut; \
  ::boost::ut::expect(::boost::ut::that % __VA_ARGS__)
#define SUITE ::boost::ut::suite _ = []
#define TEST(name) ::boost::ut::detail::test{"test", name} = [=]() mutable
#define TEST_P(name, ...)                 \
  using namespace ::boost::ut::operators; \
  ::boost::ut::detail::test{"test", name} = [=](__VA_ARGS__) mutable

SUITE {
  TEST("suite") { EXPECT(42 == 42); };

  TEST_P("parameterized", const auto& arg) {
    EXPECT(arg > 0) << "all values greater than 0";
  }
  | std::vector{1, 2, 3};
};

int main() {
  TEST("macro") { EXPECT(1 != 2); };

  TEST("vector") {
    std::vector<int> v(5);

    EXPECT((5u == std::size(v)) >> fatal);

    TEST("resize bigger") {
      v.resize(10);
      EXPECT(10u == std::size(v));
    };
  };
}
