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
#include <sstream>
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

namespace ut = boost::ut;

ut::suite _ = [] {
  using namespace ut;

  bdd::gherkin::steps steps = [](auto& steps) {
    steps.feature("Calculator") = [&] {
      steps.scenario("*") = [&] {
        steps.given("I have calculator") = [&] {
          calculator<int> calc{};
          steps.when("I enter {value}") = [&](int value) { calc.enter(value); };
          steps.when("I enter the numbers:") =
              [&](const bdd::gherkin::doc_string& numbers) {
                std::istringstream stream{numbers.content};
                for (int value{}; stream >> value;) calc.enter(value);
              };
          steps.when("I enter the following values:") =
              [&](const bdd::gherkin::table& values) {
                for (const auto& row : values)
                  for (const auto& cell : row) calc.enter(std::stoi(cell));
              };
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
      # Comments start with # at the beginning of a line
      @calculator
      Feature: Calculator

        Background:
          Given I have calculator

        Scenario Outline: Addition of <a> and <b>
          When I enter <a>
          But I enter <b>
          * I press add
          Then I expect <sum>

          Examples:
            | a  | b | sum |
            | 1  | 2 | 3   |
            | 40 | 2 | 42  |

        Scenario: Sum of a table
          When I enter the following values:
            | 1 | 2 | 3 |
            | 4 | 5 | 6 |
           And I press add
          Then I expect 21

        Rule: Subtraction

          Scenario: Subtraction of a doc string
            When I enter the numbers:
              """
              45 2 1
              """
            And I press sub
            Then I expect 42

          @skip
          Scenario: Not implemented yet
            When I enter 1
            Then I expect 2
    )";
  // clang-format on

  // boost_ut_gherkin --feature <path to the feature file>
  if (const auto& path = bdd::gherkin::feature_file(); not std::empty(path)) {
    std::ifstream file{path};
    expect(file.is_open()) << "cannot open" << path;

    "Calculator"_test =
        steps | std::string{(std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>()};
  }
};

int main(int argc, const char** argv) {
  // the command line is parsed when the test suites are run
  return ut::cfg<>.run({.argc = argc, .argv = argv});
}
