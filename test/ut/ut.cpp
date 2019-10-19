//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/ut.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ut = boost::ut;

constexpr auto to_string = [](const auto expr) {
  std::stringstream str{};
  str << std::boolalpha << expr;
  return str.str();
};

constexpr auto test_assert = [](bool result) {
  if (not result) {
    throw;
  }
};

struct fake_cfg {
  struct assertion_call {
    std::experimental::source_location location{};
    std::string str{};
    bool result{};
  };

  struct test_call {
    std::string_view type{};
    std::string_view name{};
    std::any arg{};
  };

  template <class Test, class TArg>
  auto run(Test test, [[maybe_unused]] const TArg& arg) -> void {
    if constexpr (std::is_invocable_v<Test, TArg>) {
      test(arg);
    } else {
      test.template operator()<TArg>();
    }
  }

  template <class Test>
  auto run(Test test, ut::none) {
    test();
  }

  template <class... Ts>
  auto on(ut::events::test_run<Ts...> test) {
    if (std::empty(test_filter) or test.name == test_filter) {
      test_run_calls.push_back({test.type, test.name, test.arg});
      try {
        run(test.test, test.arg);
      } catch (...) {
      }
    }
  }
  template <class... Ts>
  auto on(ut::events::test_skip<Ts...> test) {
    test_skip_calls.push_back({test.type, test.name, test.arg});
  }
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr> assertion) -> bool {
    assertion_calls.push_back(
        {assertion.location, to_string(assertion.expr), assertion.expr});
    return assertion.expr;
  }
  auto on(ut::events::fatal_assertion) { ++fatal_assertion_calls; }
  auto on(ut::events::log log) { log_calls.push_back(log.msg); }

  std::vector<test_call> test_run_calls{};
  std::vector<test_call> test_skip_calls{};
  std::vector<std::string_view> log_calls{};
  std::vector<assertion_call> assertion_calls{};
  std::size_t fatal_assertion_calls{};
  std::string test_filter{};
};

namespace ns {
template <char... Cs>
constexpr auto operator""_i() -> int {
  return sizeof...(Cs);
}
auto f() -> int { return 0_i; }
}  // namespace ns
auto f() -> int {
  using namespace ns;
  return 42_i;
}

struct custom {
  int i{};

  friend auto operator==(const custom& lhs, const custom& rhs) {
    return lhs.i == rhs.i;
  }

  friend auto operator<<(std::ostream& os, const custom& c) -> std::ostream& {
    return (os << "custom{" << c.i << '}');
  }
};

template <>
auto ut::cfg<ut::override> = fake_cfg{};

int main() {
  using namespace ut;
  using namespace std::literals::string_view_literals;

  {
    static_assert("void"sv == reflection::type_name<void>());
    static_assert("int"sv == reflection::type_name<int>());
  }

  {
    static_assert(true_b);
    static_assert(not false_b);
    static_assert(42 == 42_i);
    static_assert(0u == 0_u);
    static_assert(42u == 42_u);
    static_assert(42 == 42_s);
    static_assert(42 == 42_c);
    static_assert(42 == 42_l);
    static_assert(42 == 42_ll);
    static_assert(42u == 42_uc);
    static_assert(42u == 42_us);
    static_assert(42u == 42_us);
    static_assert(42u == 42_ul);
    static_assert(42.42f == 42.42_f);
    static_assert(42.42 == 42.42_d);
    static_assert(42.42 == 42.42_ld);
    static_assert(0 == 0_i);
    static_assert(10'000 == 10'000_i);
    static_assert(42000'000 == 42000'000_i);
    static_assert(9'999 == 9'999_i);
    static_assert(42 == 42_i);
    static_assert(-42 == -42_i);
    static_assert(-42 == -42_i);
    static_assert(0. == 0.0_d);
    static_assert(0.f == .0_f);
    static_assert(0.1 == .1_d);
    static_assert(0.1 == 0.1_d);
    static_assert(0.1177 == 0.1177_d);
    static_assert(-0.1177 == -0.1177_d);
    static_assert(0.01177 == 0.01177_d);
    static_assert(-0.01177 == -0.01177_d);
    static_assert(0.001177 == 0.001177_d);
    static_assert(-0.001177 == -0.001177_d);
    static_assert(001.001177 == 001.001177_d);
    static_assert(-001.001177 == -001.001177_d);
    static_assert(01.001177 == 01.001177_d);
    static_assert(-01.001177 == -01.001177_d);
    static_assert(0.42f == 0.42_f);
    static_assert(2.42f == 2.42_f);
    static_assert(-2.42 == -2.42_d);
    static_assert(123.456 == 123.456_d);
  }

  {
    static_assert(_i(42) == 42_i);
    static_assert(_b(true));
    static_assert(not _b(false));
    static_assert(_s(42) == 42_s);
    static_assert(_c(42) == 42_c);
    static_assert(_l(42) == 42_l);
    static_assert(_ll(42) == 42_ll);
    static_assert(_uc(42u) == 42_uc);
    static_assert(_us(42u) == 42_us);
    static_assert(_us(42u) == 42_us);
    static_assert(_ul(42u) == 42_ul);
    static_assert(_f(42.42f) == 42.42_f);
    static_assert(_d(42.42) == 42.42_d);
    static_assert(_ld(42.42) == 42.42_ld);
  }

  {
    test_assert(42_i == 42_i);
    test_assert(42_i == 42);
    test_assert(42 == 42_i);
    test_assert(not(1_i == 2));
    test_assert(not(2 == 1_i));
  }

  {
    test_assert(_ul(42) == 42ul);
    test_assert(_i(42) == 42);
    test_assert(42 == _i(42));
    test_assert(true == _b(true));
    test_assert(_b(true) != _b(false));
    test_assert(_i(42) > _i({}));
    test_assert(42_i < 88);
    test_assert(true_b != false_b);
    test_assert(42 >= 42_i);
    test_assert(43 >= 42_i);
    test_assert(42 <= 42_i);
    test_assert(42 <= 43_i);
    test_assert(not false_b);
  }

  {
    test_assert(true_b and 42_i == 42_i);
    test_assert(false_b or 42_i == 42_i);
    test_assert(true_b or 42_i != 42_i);
    test_assert(not(42_i < 0));
  }

  {
    test_assert("0 == 0" == to_string(0_s == _s(0)));
    test_assert("0 != 0" == to_string(0_s != _s(0)));
    test_assert("0 < 0" == to_string(0_s < _s(0)));
    test_assert("0 > 0" == to_string(0_s > _s(0)));
    test_assert("0 >= 0" == to_string(0_s >= _s(0)));
    test_assert("0 <= 0" == to_string(0_s <= _s(0)));
    test_assert("0 != 0" == to_string(0_s != _s(0)));
    test_assert("not 0" == to_string(!0_s));
    test_assert("42 == 42" == to_string(42_i == 42));
    test_assert("42 != 42" == to_string(42_i != 42));
    test_assert("42 > 0" == to_string(42_ul > 0_ul));
    test_assert("int == float" == to_string(type<int> == type<float>));
    test_assert("void != double" == to_string(type<void> != type<double>));
    test_assert("(true or 42.42 == 12.34)" ==
                to_string(true_b or (42.42_d == 12.34)));
    test_assert("(not 1 == 2 and str == str2)" ==
                to_string(not(1_i == 2) and ("str"sv == "str2"sv)));
  }

  {
    static_assert(type<int> == type<int>);
    static_assert(type<void> == type<void>);
    static_assert(type<void*> == type<void*>);
    static_assert(type<int> != type<const int>);
    static_assert(type<int> != type<void>);
  }

  {
    struct test_cfg : default_cfg {
      using default_cfg::active_exception_;
      using default_cfg::asserts_;
      using default_cfg::tests_;
    };

    auto tcfg = test_cfg{};
    tcfg.on(events::test_run{"test", "run", none{}, [] {}});
    test_assert(1 == tcfg.tests_.pass);
    test_assert(0 == tcfg.tests_.fail);
    test_assert(0 == tcfg.tests_.skip);

    tcfg.on(events::test_skip{"test", "skip", none{}, [] {}});
    test_assert(1 == tcfg.tests_.pass);
    test_assert(0 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);

    tcfg.filter = "unknown";
    tcfg.on(events::test_run{"test", "filter", none{}, [] {}});
    test_assert(1 == tcfg.tests_.pass);
    test_assert(0 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);
    tcfg.filter = {};

    tcfg.filter = "filter";
    tcfg.on(events::test_run{"test", "filter", none{}, [] {}});
    test_assert(2 == tcfg.tests_.pass);
    test_assert(0 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);
    tcfg.filter = {};

    tcfg.on(events::test_run{"test", "pass", none{}, [&tcfg] {
                               tcfg.on(events::assertion{
                                   std::experimental::source_location{}, true});
                             }});

    test_assert(3 == tcfg.tests_.pass);
    test_assert(0 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);

    tcfg.on(
        events::test_run{"test", "fail", none{}, [&tcfg] {
                           tcfg.on(events::assertion{
                               std::experimental::source_location{}, false});
                         }});
    test_assert(3 == tcfg.tests_.pass);
    test_assert(1 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);

    tcfg.on(events::test_run{"test", "exception", none{}, [] { throw 42; }});
    test_assert(3 == tcfg.tests_.pass);
    test_assert(2 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);

    tcfg.on(events::test_run{
        "test", "section", none{}, [&tcfg] {
          tcfg.on(events::test_run{"test", "section", none{}, [] {}});
        }});
    test_assert(4 == tcfg.tests_.pass);
    test_assert(2 == tcfg.tests_.fail);
    test_assert(1 == tcfg.tests_.skip);

    tcfg.tests_ = {};
    tcfg.asserts_ = {};
  }

  auto& test_cfg = ut::cfg<ut::override>;

  {
    test_cfg = fake_cfg{};
    test_cfg.test_filter = "run";

    "run"_test = [] {};
    "ignore"_test = [] {};

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert("test"sv == test_cfg.test_run_calls[0].type);
    test_assert("run"sv == test_cfg.test_run_calls[0].name);
    void(std::any_cast<none>(test_cfg.test_run_calls[0].arg));
    test_assert(std::empty(test_cfg.test_skip_calls));
    test_assert(std::empty(test_cfg.log_calls));
    test_assert(0 == test_cfg.fatal_assertion_calls);
  }

  {
    test_cfg = fake_cfg{};

    "run"_test = [] {};
    skip | "skip"_test = [] {};

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert("run"sv == test_cfg.test_run_calls[0].name);
    test_assert(1 == std::size(test_cfg.test_skip_calls));
    test_assert("test"sv == test_cfg.test_skip_calls[0].type);
    test_assert("skip"sv == test_cfg.test_skip_calls[0].name);
    void(std::any_cast<none>(test_cfg.test_skip_calls[0].arg));
    test_assert(std::empty(test_cfg.log_calls));
    test_assert(0 == test_cfg.fatal_assertion_calls);
  }

  {
    test_cfg = fake_cfg{};

    "logging"_test = [] {
      boost::ut::log << "msg1"
                     << "msg2";
    };

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert("logging"sv == test_cfg.test_run_calls[0].name);
    test_assert(2 == std::size(test_cfg.log_calls));
    test_assert("msg1"sv == test_cfg.log_calls[0]);
    test_assert("msg2"sv == test_cfg.log_calls[1]);
  }

  {
    test_cfg = fake_cfg{};

    expect(1 == 2_i);
    expect(42 == 42_i);
    expect(1 != 2_i);
    expect(-42 != -42_i);
    expect(-1.1_d == -1.1);
    expect(2_i > 2_i) << "msg";

    test_assert(6 == std::size(test_cfg.assertion_calls));

    test_assert("1 == 2" == test_cfg.assertion_calls[0].str);
    test_assert(not test_cfg.assertion_calls[0].result);

    test_assert("42 == 42" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[1].result);

    test_assert("1 != 2" == test_cfg.assertion_calls[2].str);
    test_assert(test_cfg.assertion_calls[2].result);

    test_assert("-42 != -42" == test_cfg.assertion_calls[3].str);
    test_assert(not test_cfg.assertion_calls[3].result);

    test_assert("-1.1 == -1.1" == test_cfg.assertion_calls[4].str);
    test_assert(test_cfg.assertion_calls[4].result);

    test_assert("2 > 2" == test_cfg.assertion_calls[5].str);
    test_assert(not test_cfg.assertion_calls[5].result);
    test_assert(1 == std::size(test_cfg.log_calls));
    test_assert("msg"sv == test_cfg.log_calls[0]);
  }

  {
    test_cfg = fake_cfg{};

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

    expect("str1"sv == "str2"sv);

    test_assert(9 == std::size(test_cfg.assertion_calls));

    test_assert("str == str" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[0].result);

    test_assert("str1 != str2" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[1].result);

    test_assert("str == str" == test_cfg.assertion_calls[2].str);
    test_assert(test_cfg.assertion_calls[2].result);

    test_assert("str1 != str2" == test_cfg.assertion_calls[3].str);
    test_assert(test_cfg.assertion_calls[3].result);

    test_assert("str == str" == test_cfg.assertion_calls[4].str);
    test_assert(test_cfg.assertion_calls[4].result);

    test_assert("str1 != str2" == test_cfg.assertion_calls[5].str);
    test_assert(test_cfg.assertion_calls[5].result);

    test_assert("str == str" == test_cfg.assertion_calls[6].str);
    test_assert(test_cfg.assertion_calls[6].result);

    test_assert("str1 != str2" == test_cfg.assertion_calls[7].str);
    test_assert(test_cfg.assertion_calls[7].result);

    test_assert("str1 == str2" == test_cfg.assertion_calls[8].str);
    test_assert(not test_cfg.assertion_calls[8].result);
  }

  {
    test_cfg = fake_cfg{};

    expect(std::vector<int>{} == std::vector<int>{});
    expect(std::vector{'a', 'b'} == std::vector{'a', 'b'});
    expect(std::vector{1, 2, 3} == std::vector{1, 2});

    test_assert(3 == std::size(test_cfg.assertion_calls));

    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("{} == {}" == test_cfg.assertion_calls[0].str);

    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("{a, b} == {a, b}" == test_cfg.assertion_calls[1].str);

    test_assert(not test_cfg.assertion_calls[2].result);
    test_assert("{1, 2, 3} == {1, 2}" == test_cfg.assertion_calls[2].str);
  }

  {
    test_cfg = fake_cfg{};

    expect(std::array{1} != std::array{2});
    expect(std::array{1, 2, 3} == std::array{3, 2, 1});

    test_assert(2 == std::size(test_cfg.assertion_calls));

    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("{1} != {2}" == test_cfg.assertion_calls[0].str);

    test_assert(not test_cfg.assertion_calls[1].result);
    test_assert("{1, 2, 3} == {3, 2, 1}" == test_cfg.assertion_calls[1].str);
  }

  {
    test_cfg = fake_cfg{};

    expect(custom{42} == detail::value<custom>{custom{42}});

    test_assert(1 == std::size(test_cfg.assertion_calls));

    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("custom{42} == custom{42}" == test_cfg.assertion_calls[0].str);
  }

  {
    test_cfg = fake_cfg{};

    "assertions"_test = [] { expect(42_i == 42); };

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert("assertions"sv == test_cfg.test_run_calls[0].name);
    test_assert(std::empty(test_cfg.test_skip_calls));
    test_assert(std::empty(test_cfg.log_calls));
    test_assert(0 == test_cfg.fatal_assertion_calls);
    test_assert(1 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].location.line() > 0);
    test_assert("42 == 42" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[0].result);
  }

  {
    test_cfg = fake_cfg{};

    "fatal assertions"_test = [] {
      !expect(1_i == 1);
      !expect(2 != 2_i) << "fatal";
    };

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[0].str);
    test_assert(not test_cfg.assertion_calls[1].result);
    test_assert("2 != 2" == test_cfg.assertion_calls[1].str);
    test_assert(1 == test_cfg.fatal_assertion_calls);
    test_assert(1 == std::size(test_cfg.log_calls));
    test_assert("fatal"sv == test_cfg.log_calls[0]);
  }

  {
    test_cfg = fake_cfg{};

    "exceptions"_test = [] {
      expect(throws<std::runtime_error>([] { throw std::runtime_error{""}; }))
          << "throws runtime_error";
      expect(throws([] { throw 0; })) << "throws any exception";
      expect(throws<std::runtime_error>([] { throw 0; }))
          << "throws wrong exception";
      expect(throws<std::runtime_error>([] {})) << "doesn't throw";
      expect(nothrow([] {})) << "doesn't throw";
      expect(nothrow([] { throw 0; })) << "throws";
    };

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert("exceptions"sv == test_cfg.test_run_calls[0].name);
    test_assert(6 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("true" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("true" == test_cfg.assertion_calls[1].str);
    test_assert(not test_cfg.assertion_calls[2].result);
    test_assert("false" == test_cfg.assertion_calls[2].str);
    test_assert(not test_cfg.assertion_calls[3].result);
    test_assert("false" == test_cfg.assertion_calls[3].str);
    test_assert(test_cfg.assertion_calls[4].result);
    test_assert("true" == test_cfg.assertion_calls[4].str);
    test_assert(not test_cfg.assertion_calls[5].result);
    test_assert("false" == test_cfg.assertion_calls[5].str);

    "should throw"_test = [] {
      auto f = [](const auto should_throw) {
        if (should_throw) throw 42;
        return 42;
      };
      expect(42_i == f(true));
    };

    test_assert(2 == std::size(test_cfg.test_run_calls));
    test_assert("should throw"sv == test_cfg.test_run_calls[1].name);
    test_assert(6 == std::size(test_cfg.assertion_calls));
  }

  {
    test_cfg = fake_cfg{};

    "[vector]"_test = [] {
      std::vector<int> v(5);

      !expect(5_ul == std::size(v));

      "resize bigger"_test = [=]() mutable {
        v.resize(10);
        expect(10_ul == std::size(v));
      };

      !expect(5_ul == std::size(v));

      "resize smaller"_test = [=]() mutable {
        v.resize(0);
        expect(0_ul == std::size(v));
      };
    };

    test_assert(3 == std::size(test_cfg.test_run_calls));
    test_assert("[vector]"sv == test_cfg.test_run_calls[0].name);
    test_assert("resize bigger"sv == test_cfg.test_run_calls[1].name);
    test_assert("resize smaller"sv == test_cfg.test_run_calls[2].name);
    test_assert(4 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("5 == 5" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("10 == 10" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("5 == 5" == test_cfg.assertion_calls[2].str);
    test_assert(test_cfg.assertion_calls[3].result);
    test_assert("0 == 0" == test_cfg.assertion_calls[3].str);
  }

  {
    test_cfg = fake_cfg{};

    "args vector"_test = [](const auto& arg) {
      expect(arg > 0_i) << "all values greater than 0";
    } | std::vector{1, 2, 3};

    test_assert(3 == std::size(test_cfg.test_run_calls));
    test_assert("args vector"sv == test_cfg.test_run_calls[0].name);
    test_assert(1 == std::any_cast<int>(test_cfg.test_run_calls[0].arg));
    test_assert("args vector"sv == test_cfg.test_run_calls[1].name);
    test_assert(2 == std::any_cast<int>(test_cfg.test_run_calls[1].arg));
    test_assert("args vector"sv == test_cfg.test_run_calls[2].name);
    test_assert(3 == std::any_cast<int>(test_cfg.test_run_calls[2].arg));
    test_assert(3 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("1 > 0" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("2 > 0" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("3 > 0" == test_cfg.assertion_calls[2].str);
  }

  {
    test_cfg = fake_cfg{};

    "args array"_test = [](const auto& arg) {
      expect(0_i <= arg) << "all values greater than 0";
    } | std::array{99, 11};

    test_assert(2 == std::size(test_cfg.test_run_calls));
    test_assert("args array"sv == test_cfg.test_run_calls[0].name);
    test_assert(99 == std::any_cast<int>(test_cfg.test_run_calls[0].arg));
    test_assert("args array"sv == test_cfg.test_run_calls[1].name);
    test_assert(11 == std::any_cast<int>(test_cfg.test_run_calls[1].arg));
    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("0 <= 99" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("0 <= 11" == test_cfg.assertion_calls[1].str);
  }

  {
    test_cfg = fake_cfg{};

    "args string"_test = [](const auto& arg) {
      expect(_c('s') == arg or _c('t') == arg or _c('r') == arg);
    } | std::string{"str"};

    test_assert(3 == std::size(test_cfg.test_run_calls));
    test_assert("args string"sv == test_cfg.test_run_calls[0].name);
    test_assert('s' == std::any_cast<char>(test_cfg.test_run_calls[0].arg));
    test_assert("args string"sv == test_cfg.test_run_calls[1].name);
    test_assert('t' == std::any_cast<char>(test_cfg.test_run_calls[1].arg));
    test_assert("args string"sv == test_cfg.test_run_calls[2].name);
    test_assert('r' == std::any_cast<char>(test_cfg.test_run_calls[2].arg));
    test_assert(3 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("((s == s or t == s) or r == s)" ==
                test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("((s == t or t == t) or r == t)" ==
                test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("((s == r or t == r) or r == r)" ==
                test_cfg.assertion_calls[2].str);
  }

  if (false) {
    test_cfg = fake_cfg{};

    "args map"_test = [](const auto& arg) {
      const auto [key, value] = arg;
      expect(_c(key) == 'a' or _c('b') == key);
      expect(value > 0_i);
    } | std::unordered_map<char, int>{{'a', 1}, {'b', 2}};

    test_assert(2 == std::size(test_cfg.test_run_calls));
    test_assert("args map"sv == test_cfg.test_run_calls[0].name);
    test_assert('b' == std::any_cast<std::pair<const char, int>>(
                           test_cfg.test_run_calls[0].arg)
                           .first);
    test_assert("args map"sv == test_cfg.test_run_calls[1].name);
    test_assert('a' == std::any_cast<std::pair<const char, int>>(
                           test_cfg.test_run_calls[1].arg)
                           .first);
    test_assert(4 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("(b == a or b == b)" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("2 > 0" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("(a == a or b == a)" == test_cfg.assertion_calls[2].str);
    test_assert(test_cfg.assertion_calls[3].result);
    test_assert("1 > 0" == test_cfg.assertion_calls[3].str);
  }

  {
    test_cfg = fake_cfg{};

    "types"_test = []<class T>() {
      expect(std::is_integral<T>{} or
             type<void> == type<std::remove_pointer_t<T>>)
          << "all types are integrals or void";
    }
    | std::tuple<bool, int, void*>{};

    test_assert(3 == std::size(test_cfg.test_run_calls));
    test_assert("types"sv == test_cfg.test_run_calls[0].name);
    void(std::any_cast<bool>(test_cfg.test_run_calls[0].arg));
    test_assert("types"sv == test_cfg.test_run_calls[1].name);
    void(std::any_cast<int>(test_cfg.test_run_calls[1].arg));
    test_assert("types"sv == test_cfg.test_run_calls[2].name);
    void(std::any_cast<void*>(test_cfg.test_run_calls[2].arg));
    test_assert(3 == std::size(test_cfg.assertion_calls));
    test_assert("(true or void == bool)" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("(true or void == int)" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("(false or void == void)" == test_cfg.assertion_calls[2].str);
    test_assert(test_cfg.assertion_calls[2].result);
  }

  {
    test_cfg = fake_cfg{};

    "args and types"_test = []<class TArg>(const TArg& arg) {
      expect(42_i == arg or arg == 42.42_f);
      expect(type<TArg> == type<int> or type<TArg> == type<float>);
    }
    | std::tuple{42, 42.42f};

    test_assert(2 == std::size(test_cfg.test_run_calls));
    test_assert("args and types"sv == test_cfg.test_run_calls[0].name);
    test_assert(42 == std::any_cast<int>(test_cfg.test_run_calls[0].arg));
    test_assert("args and types"sv == test_cfg.test_run_calls[1].name);
    test_assert(42.42f == std::any_cast<float>(test_cfg.test_run_calls[1].arg));
    test_assert(4 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert(test_cfg.assertion_calls[3].result);
    test_assert("(42 == 42 or 42 == 42.42)" == test_cfg.assertion_calls[0].str);
    test_assert("(int == int or int == float)" ==
                test_cfg.assertion_calls[1].str);
    test_assert("(42 == 42.42 or 42.42 == 42.42)" ==
                test_cfg.assertion_calls[2].str);
    test_assert("(float == int or float == float)" ==
                test_cfg.assertion_calls[3].str);
  }

  {
    test_cfg = fake_cfg{};

    "scenario"_test = [] {
      given("I have...") = [] {
        when("I run...") = [] {
          then("I expect...") = [] { expect(1_u == 1u); };
          then("I expect...") = [] { expect(1u == 1_u); };
        };
      };
    };

    test_assert(5 == std::size(test_cfg.test_run_calls));
    test_assert("scenario"sv == test_cfg.test_run_calls[0].name);
    test_assert("given"sv == test_cfg.test_run_calls[1].type);
    test_assert("I have..."sv == test_cfg.test_run_calls[1].name);
    test_assert("when"sv == test_cfg.test_run_calls[2].type);
    test_assert("I run..."sv == test_cfg.test_run_calls[2].name);
    test_assert("then"sv == test_cfg.test_run_calls[3].type);
    test_assert("I expect..."sv == test_cfg.test_run_calls[3].name);
    test_assert("then"sv == test_cfg.test_run_calls[4].type);
    test_assert("I expect..."sv == test_cfg.test_run_calls[4].name);
    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[1].str);
  }

  {
    test_cfg = fake_cfg{};

    "should disambiguate operators"_test = [] {
      expect(1_i == ns::f());
      expect(2_i == f());
    };

    test_assert(1 == std::size(test_cfg.test_run_calls));
    test_assert("should disambiguate operators"sv ==
                test_cfg.test_run_calls[0].name);
    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("2 == 2" == test_cfg.assertion_calls[1].str);
  }
}
