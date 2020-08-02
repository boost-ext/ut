//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
import ut;

constexpr auto sum = [](auto... args) { return (0 + ... + args); };

ut::suite sums = [] {
  using namespace ut;
  using namespace ut::literals;
  using namespace ut::operators;

  "sum"_test = [] {
    expect(sum() == 0_i);
    expect(sum(40, 2) == 42_i);
  };
};

int main() {
  // clang-format off
  using ut::operator""_test;
  using namespace ut::literals;

  "terse"_test = [] {
    using namespace ut::operators::terse;
    42_i == 42;
    sum(40, 2) == 42_i;
  };

  using namespace ut::operators;
  static_assert(0_i == 0);
  static_assert(42_i == 42);

  using ut::expect;

  "expect"_test = [] {
    expect(42_i == 42) << "equal";
  };

  "section"_test = [] {
    using ut::should;

    should("be equal") = [] {
      expect(42 == 42_i);
    };
  };

  "BDD"_test = [] {
    using namespace ut::bdd;

    given("I have a sum") = [] {
      when("I add 40 and 2") = [] {
        const auto result = sum(40, 2);
        then("I should get 42") = [] {
          expect(42_i == result);
        };
      };
    };
  };

  using namespace ut::spec;

  describe("sum") = [] {
    it("should add arguments") = [] {
      expect(sum(40, 2) == 42_i);
    };
  };
  // clang-format on
}
