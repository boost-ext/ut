//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <fstream>
#include <numeric>
#include <streambuf>
#include <string>

template <class T>
class calculator {
 public:
  auto enter(const T& value) -> void { values_.push_back(value); }
  auto add() -> void {
    result_ = std::accumulate(std::cbegin(values_), std::cend(values_), T{});
  }
  auto sub() -> void {
    result_ = std::accumulate(std::cbegin(values_) + 1, std::cend(values_),
                              values_.front(), std::minus{});
  }
  auto get() const -> T { return result_; }

 private:
  std::vector<T> values_{};
  T result_{};
};

int main(int argc, const char** argv) {
  using namespace boost::ut;

  bdd::gherkin::steps steps = [](auto& steps) {
    steps.feature("Calculator") = [&] {
      steps.scenario("*") = [&] {
        steps.given("I have calculator") = [&] {
          calculator<int> calc{};
          steps.when("I enter {value}") = [&](int value) { calc.enter(value); };
          steps.when("I press add") = [&] { calc.add(); };
          steps.when("I press sub") = [&] { calc.sub(); };
          steps.then("I expect {value}") = [&](int result) {
            expect(that % calc.get() == result);
          };
        };
      };
    };
  };

  // clang-format off
  "Calculator"_test = steps |
    R"(
      Feature: Calculator

        Scenario: Addition
          Given I have calculator
           When I enter 40
           When I enter 2
           When I press add
           Then I expect 42

        Scenario: Subtraction
          Given I have calculator
           When I enter 4
           When I enter 2
           When I press sub
           Then I expect 2
    )";
  // clang-format on

  if (argc == 2) {
    const auto file = [](const auto path) {
      std::ifstream file{path};
      return std::string{(std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>()};
    };

    "Calculator"_test = steps | file(argv[1]);
  }
}
