//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <fstream>
#include <functional>
#include <streambuf>
#include <string>
#include <unordered_map>

namespace gherkin {
class steps {
  using step_t = std::string;
  using call_step_t = std::function<void(const std::string&)>;

  class step {
   public:
    template <class TPattern>
    step(steps& steps, const TPattern& pattern)
        : steps_{steps},
          expr_{steps.call_steps_[std::string{pattern}]},
          pattern_{pattern} {}

    ~step() noexcept { steps_.next(pattern_); }

    template <class T>
    static auto make(const std::string& arg) -> T {
      return T{std::stoi(arg)};
    }

    template <class TExpr>
    auto operator=(const TExpr& expr) -> void {
      if (expr_) {
        return;
      }
      expr_ = [this, expr, pattern = pattern_](const std::string& step) {
        [&]<class... TArgs>(boost::ut::type_traits::list<TArgs...>) {
          boost::ut::log << step;
          const auto& ms = matches(pattern, step);
          auto i = 0;
          expr(make<TArgs>(ms[i++])...);
        }
        (typename boost::ut::type_traits::function_traits<TExpr>::args{});
      };
    }

   private:
    steps& steps_;
    call_step_t& expr_;
    std::string pattern_{};
  };

 public:
  template <class TSteps>
  constexpr /*explicit(false)*/ steps(const TSteps& steps) : steps_{steps} {}

  template <class TGherkin>
  auto operator|(const TGherkin& gherkin) {
    gherkin_ = boost::ut::utility::split<std::string>(gherkin, '\n');
    return [this] {
      steps_(*this);
      current_step_ = tmp_current_step_ = {};
      call_steps_ = {};
    };
  }
  auto feature(std::string pattern) {
    return step{*this, "Feature: " + pattern};
  }
  auto scenario(std::string pattern) {
    return step{*this, "Scenario: " + pattern};
  }
  auto given(std::string pattern) { return step{*this, "Given " + pattern}; }
  auto when(std::string pattern) { return step{*this, "When " + pattern}; }
  auto then(std::string pattern) { return step{*this, "Then " + pattern}; }

 private:
  template <class T>
  static auto matches(T pattern, const std::string& str)
      -> std::vector<std::string> {
    std::vector<std::string> matches{};
    auto pi = 0u, si = 0u;

    const auto matcher = [&](char b, char e, char c = 0) {
      const auto match = si;
      while (str[si] && str[si] != b && str[si] != c) ++si;
      matches.emplace_back(str.substr(match, si - match));
      while (pattern[pi] && pattern[pi] != e) ++pi;
      pi++;
    };

    while (pi < pattern.size() && si < str.size()) {
      if (pattern[pi] == '\'' && str[si] == '\'' && pattern[pi + 1] == '{') {
        ++si;
        matcher('\'', '}');
      } else if (pattern[pi] == '{') {
        matcher(' ', '}', ',');
      } else if (pattern[pi] != str[si]) {
        return std::vector<std::string>{};
      }
      ++pi, ++si;
    }

    if (si < str.size() || pi < pattern.size()) {
      return std::vector<std::string>{};
    }

    return matches;
  }

  static inline bool re_match(const char* pattern, const char* str) {
    switch (*pattern) {
      default:
        return *pattern == *str && re_match(pattern + 1, str + 1);
      case '\0':
        return *str == '\0';
      case '?':
        return *str != '\0' && re_match(pattern + 1, str + 1);
      case '*':
        return (*str != '\0' && re_match(pattern, str + 1)) ||
               re_match(pattern + 1, str);
    }
  }

  template <class TPattern>
  static auto match(const TPattern& pattern, const std::string& str) {
    return re_match(pattern.c_str(), str.c_str()) or
           not std::empty(matches(pattern, str));
  }

  template <class TPattern>
  auto next(const TPattern& pattern) -> void {
    while (true) {
      auto i = 0u;
      for (auto current_step : gherkin_) {
        if (i++ == current_step_) {
          current_step.erase(0, current_step.find_first_not_of(" \t"));

          for (const auto& [step, call] : call_steps_) {
            if (pattern.find("Scenario") == std::string::npos and
                current_step.find("Scenario") != std::string::npos) {
              tmp_current_step_ = i;
              goto done;
            }

            if (match(step, current_step)) {
              current_step_ = i;
              call(current_step);
            }
          }
        }
      }

      if (tmp_current_step_ or pattern.find("Scenario") == std::string::npos) {
        break;
      } else {
        current_step_ = tmp_current_step_;
        tmp_current_step_ = {};
      }
    }
  done:;
  }

  void (*steps_)(steps&){};
  std::vector<std::string> gherkin_{};
  std::size_t current_step_{}, tmp_current_step_{};
  std::unordered_map<step_t, call_step_t> call_steps_{};
};
}  // namespace gherkin

gherkin::steps steps = [](auto& steps) {
  using namespace boost::ut;

  steps.feature("*") = [&] {
    steps.scenario("*") = [&] {
      steps.given("I have {}") = [&](int value) {
        auto i = value;
        steps.when("I increase it by {}") = [&](int value) { i += value; };
        steps.then("I expect {}") = [&](int result) {
          expect(that % i == result);
        };
      };
    };
  };
};

int main(int argc, char** argv) {
  using namespace boost::ut;

  // clang-format off
  "gherkin text"_test = steps |
    R"(
      Feature: Feature 0

        Scenario: Scenario 1
          Given I have 42
          When I increase it by 1
          Then I expect 43
          Then I expect 43
          When I increase it by 2
          Then I expect 45

        Scenario: Scenario 2
          Given I have 1234
          When I increase it by 10
          Then I expect 1244
    )";
  // clang-format on

  if (argc == 2) {
    const auto path = std::string{argv[1]};
    std::ifstream file{path};
    test(path) = steps | std::string{(std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>()};
  }
}
