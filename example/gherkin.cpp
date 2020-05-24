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

namespace gherkin {
namespace detail {
template <class TPattern, class TStr>
constexpr auto match(const TPattern& pattern, const TStr& str)
    -> std::vector<TStr> {
  std::vector<TStr> groups{};
  auto pi = 0u, si = 0u;

  const auto matcher = [&](char b, char e, char c = 0) {
    const auto match = si;
    while (str[si] and str[si] != b and str[si] != c) {
      ++si;
    }
    groups.emplace_back(str.substr(match, si - match));
    while (pattern[pi] and pattern[pi] != e) {
      ++pi;
    }
    pi++;
  };

  while (pi < std::size(pattern) && si < std::size(str)) {
    if (pattern[pi] == '\'' and str[si] == '\'' and pattern[pi + 1] == '{') {
      ++si;
      matcher('\'', '}');
    } else if (pattern[pi] == '{') {
      matcher(' ', '}', ',');
    } else if (pattern[pi] != str[si]) {
      return {};
    }
    ++pi, ++si;
  }

  if (si < str.size() or pi < std::size(pattern)) {
    return {};
  }

  return groups;
}

template <class TPattern, class TStr>
constexpr auto re_match(const TPattern& pattern, const TStr& str) -> bool {
  switch (*pattern) {
    default:
      return *pattern == *str and re_match(pattern + 1, str + 1);
    case '\0':
      return *str == '\0';
    case '?':
      return *str != '\0' and re_match(pattern + 1, str + 1);
    case '*':
      return (*str != '\0' and re_match(pattern, str + 1)) ||
             re_match(pattern + 1, str);
  }
}
}  // namespace detail

class steps {
  using step_t = std::string;
  using steps_t = void (*)(steps&);
  using gherkin_t = std::vector<step_t>;
  using call_step_t = boost::ut::utility::function<void(const std::string&)>;
  using call_steps_t = std::vector<std::pair<step_t, call_step_t>>;

  class step {
   public:
    template <class TPattern>
    step(steps& steps, const TPattern& pattern)
        : steps_{steps}, pattern_{pattern} {}

    ~step() { steps_.next(pattern_); }

    template <class TExpr>
    auto operator=(const TExpr& expr) -> void {
      for (const auto& [pattern, _] : steps_.call_steps()) {
        if (pattern_ == pattern) {
          return;
        }
      }

      steps_.call_steps().emplace_back(
          pattern_, [expr, pattern = pattern_](const auto& step) {
            [=]<class... TArgs>(boost::ut::type_traits::list<TArgs...>) {
              boost::ut::log << step;
              auto i = 0;
              const auto& ms = detail::match(pattern, step);
              expr(TArgs{std::stoi(ms[i++])}...);
            }
            (typename boost::ut::type_traits::function_traits<TExpr>::args{});
          });
    }

   private:
    steps& steps_;
    std::string pattern_{};
  };

 public:
  template <class TSteps>
  constexpr /*explicit(false)*/ steps(const TSteps& steps) : steps_{steps} {}

  template <class TGherkin>
  auto operator|(const TGherkin& gherkin) {
    gherkin_ = boost::ut::utility::split<std::string>(gherkin, '\n');
    for (auto& step : gherkin_) {
      step.erase(0, step.find_first_not_of(" \t"));
    }

    return [this] {
      step_ = {};
      steps_(*this);
    };
  }
  auto feature(const std::string& pattern) {
    return step{*this, "Feature: " + pattern};
  }
  auto scenario(const std::string& pattern) {
    return step{*this, "Scenario: " + pattern};
  }
  auto given(const std::string& pattern) {
    return step{*this, "Given " + pattern};
  }
  auto when(const std::string& pattern) {
    return step{*this, "When " + pattern};
  }
  auto then(const std::string& pattern) {
    return step{*this, "Then " + pattern};
  }

 private:
  template <class TPattern>
  auto next(const TPattern& pattern) -> void {
    const auto is_scenario = [&pattern](const auto& step) {
      constexpr auto scenario = "Scenario";
      return pattern.find(scenario) == std::string::npos and
             step.find(scenario) != std::string::npos;
    };

    const auto call_steps = [this, is_scenario](const auto& step,
                                                const auto i) {
      for (const auto& [name, call] : call_steps_) {
        if (is_scenario(step)) {
          break;
        }

        if (detail::re_match(name.c_str(), step.c_str()) or
            not std::empty(detail::match(name, step))) {
          step_ = i;
          call(step);
        }
      }
    };

    for (auto i = 0; const auto& step : gherkin_) {
      if (i++ == step_) {
        call_steps(step, i);
      }
    }
  }

  auto call_steps() -> call_steps_t& { return call_steps_; }

  steps_t steps_{};
  gherkin_t gherkin_{};
  call_steps_t call_steps_{};
  std::size_t step_{};
};
}  // namespace gherkin

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

gherkin::steps steps = [](auto& steps) {
  using namespace boost::ut;

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

int main(int argc, const char** argv) {
  using namespace boost::ut;

  // clang-format off
  "Gherkin Calculator"_test = steps |
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
    const auto path = std::string{argv[1]};
    std::ifstream file{path};
    test(path) = steps | std::string{(std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>()};
  }
}
