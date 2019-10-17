//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/ut.hpp"

#include <cstdlib>
#include <string>
#include <string_view>
#include <sstream>
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

  template <class Test>
  auto on(ut::events::test_run<Test> test) {
    if (std::empty(test_filter) or test.name == test_filter) {
      test_calls.push_back(test.name);
      try {
        test.test();
      } catch (...) {
      }
    }
  }
  template <class Test>
  auto on(ut::events::test_skip<Test> test) {
    test_skip_calls.push_back(test.name);
  }
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr> assertion) -> bool {
    assertion_calls.push_back({assertion.location, to_string(assertion.expr), assertion.expr});
    return assertion.expr;
  }
  auto on(ut::events::fatal_assertion) { ++fatal_assertion_calls; }
  auto on(ut::events::log log) { log_calls.push_back(log.msg); }

  std::vector<std::string_view> test_calls{};
  std::vector<std::string_view> test_skip_calls{};
  std::vector<std::string_view> log_calls{};
  std::vector<assertion_call> assertion_calls{};
  std::size_t fatal_assertion_calls{};
  std::string test_filter{};
};

namespace custom {
template <char... Cs>
constexpr auto operator""_i() -> int {
  return sizeof...(Cs);
}
auto f() -> int { return 0_i; }
}  // namespace custom
auto f() -> int {
  using namespace custom;
  return 42_i;
}

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
    test_assert("not 0" == to_string(not 0_s));
    test_assert("42 == 42" == to_string(42_i == 42));
    test_assert("42 != 42" == to_string(42_i != 42));
    test_assert("42 > 0" == to_string(42_ul > 0_ul));
    test_assert("int == float" == to_string(type<int> == type<float>));
    test_assert("void != double" == to_string(type<void> != type<double>));
    test_assert("(true or 42.42 == 12.34)" == to_string(true_b or (42.42_d == 12.34)));
    test_assert("(not 1 == 2 and str == str2)" == to_string(not (1_i == 2) and ("str"sv == "str2"sv)));
  }

  {
    static_assert(type<int> == type<int>);
    static_assert(type<void*> == type<void*>);
    static_assert(type<int> != type<const int>);
    static_assert(type<int> != type<void>);
  }

  auto& test_cfg = ut::cfg<ut::override>;

  {
    test_cfg = fake_cfg{};

    expect(1 == 2_i);
    expect(42 == 42_i);
    expect(1 != 2_i);

    test_assert(3 == std::size(test_cfg.assertion_calls));
    test_assert("1 == 2" == test_cfg.assertion_calls[0].str);
    test_assert(not test_cfg.assertion_calls[0].result);

    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("42 == 42" == test_cfg.assertion_calls[1].str);

    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("1 != 2" == test_cfg.assertion_calls[2].str);
  }

  {
    test_cfg = fake_cfg{};

    "test_assertions"_test = [] { expect(42_i == 42); };

    test_assert(1 == std::size(test_cfg.test_calls));
    test_assert("test_assertions"sv == test_cfg.test_calls[0]);
    test_assert(0 == std::size(test_cfg.test_skip_calls));
    test_assert(0 == std::size(test_cfg.log_calls));
    test_assert(0 == test_cfg.fatal_assertion_calls);
    test_assert(1 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].location.line() > 0);
    test_assert("42 == 42" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[0].result);
  }

  {
    test_cfg = fake_cfg{};
    test_cfg.test_filter = "run";

    "run"_test = [] {};
    "ignore"_test = [] {};

    test_assert(1 == std::size(test_cfg.test_calls));
    test_assert("run"sv == test_cfg.test_calls[0]);
    test_assert(0 == std::size(test_cfg.test_skip_calls));
    test_assert(0 == std::size(test_cfg.log_calls));
    test_assert(0 == test_cfg.fatal_assertion_calls);
  }

  {
    test_cfg = fake_cfg{};

    "run"_test = [] {};
    skip | "skip"_test = [] {};

    test_assert(1 == std::size(test_cfg.test_calls));
    test_assert("run"sv == test_cfg.test_calls[0]);
    test_assert(1 == std::size(test_cfg.test_skip_calls));
    test_assert("skip"sv == test_cfg.test_skip_calls[0]);
    test_assert(0 == std::size(test_cfg.log_calls));
    test_assert(0 == test_cfg.fatal_assertion_calls);
  }

  {
    test_cfg = fake_cfg{};

    "logging"_test = [] {
      boost::ut::log << "msg1"
                     << "msg2";
    };

    test_assert(2 == std::size(test_cfg.log_calls));
    test_assert("msg1"sv == test_cfg.log_calls[0]);
    test_assert("msg2"sv == test_cfg.log_calls[1]);
  }

  {
    test_cfg = fake_cfg{};

    "exceptions"_test = [] {
      expect(throws<std::runtime_error>([] { throw std::runtime_error{""}; }))
          << "throws runtime_error";
      expect(throws([] { throw 0; })) << "throws any exception";
      expect(nothrow([] {})) << "doesn't throw";
    };

    test_assert(3 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("true" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("true" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("true" == test_cfg.assertion_calls[2].str);

    "should throw"_test = [] {
      auto f = [](const auto should_throw) {
        if (should_throw) throw 42;
        return 42;
      };
      expect(42_i == f(true));
    };

    test_assert(3 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("true" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("true" == test_cfg.assertion_calls[1].str);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert("true" == test_cfg.assertion_calls[2].str);
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

    "args"_test = [](const auto& arg) {
      expect(arg > 0_i) << "all values greater than 0";
    } | std::vector{1, 2, 3};

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

    "types"_test = []<class T>() {
      expect(std::is_integral_v<T>) << "all types are integrals";
    }
    | std::tuple<bool, int>{};

    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("true" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("true" == test_cfg.assertion_calls[1].str);
  }

  {
    test_cfg = fake_cfg{};

    "args and types"_test = []<class TArg>(const TArg& arg) {
      expect(42_i == arg or arg == 42.42_f);
      expect(type<TArg> == type<int> or type<TArg> == type<float>);
    }
    | std::tuple{42, 42.42f};

    test_assert(4 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert(test_cfg.assertion_calls[2].result);
    test_assert(test_cfg.assertion_calls[3].result);
    test_assert("(42 == 42 or 42 == 42.42)" == test_cfg.assertion_calls[0].str);
    test_assert("(int == int or int == float)" == test_cfg.assertion_calls[1].str);
    test_assert("(42 == 42.42 or 42.42 == 42.42)" == test_cfg.assertion_calls[2].str);
    test_assert("(float == int or float == float)" == test_cfg.assertion_calls[3].str);
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

    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[1].str);
  }

  {
    test_cfg = fake_cfg{};

    "should disambiguate operators"_test = [] {
      expect(1_i == custom::f());
      expect(2_i == f());
    };

    test_assert(2 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.assertion_calls[0].result);
    test_assert("1 == 1" == test_cfg.assertion_calls[0].str);
    test_assert(test_cfg.assertion_calls[1].result);
    test_assert("2 == 2" == test_cfg.assertion_calls[1].str);
  }
}
