//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#undef NDEBUG
#include "boost/ut.hpp"

#include <cassert>
#include <cstdlib>
#include <experimental/source_location>
#include <string>
#include <string_view>
#include <vector>

struct fake_cfg {
  struct assertion_call {
    std::experimental::source_location location{};
    bool result{};
  };

  template <class Test>
  auto on(ut::events::test_run<Test> test) {
    if (std::empty(test_filter) or test.name == test_filter) {
      test_calls.push_back(test.name);
      test.test();
    }
  }
  template <class Test>
  auto on(ut::events::test_skip<Test> test) {
    test_skip_calls.push_back(test.name);
  }
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr> assertion) -> bool {
    assertion_calls.push_back({assertion.location, assertion.expr});
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

template <>
auto ut::cfg<ut::override> = fake_cfg{};

int main() {
  using namespace ut;
  using std::literals::string_view_literals::operator""sv;

  auto& cfg = ut::cfg<ut::override>;

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
    assert(42_i == 42_i);
    assert(42_i == 42);
    assert(42 == 42_i);
    assert(not(1_i == 2));
    assert(not(2 == 1_i));
  }

  {
    assert(_ul(42) == 42ul);
    assert(_i(42) == 42);
    assert(42 == _i(42));
    assert(true == _b(true));
    assert(_b(true) != _b(false));
    assert(_i(42) > _i({}));
    assert(42_i < 88);
    assert(true_b != false_b);
    assert(42 >= 42_i);
    assert(43 >= 42_i);
    assert(42 <= 42_i);
    assert(42 <= 43_i);
    assert(not false_b);
  }

  {
    assert(true_b and 42_c == 42_c);
    assert(false_b or 42_c == 42_c);
    assert(true_b or 42_c != 42_c);
    assert(not(42_c < 0));
  }

  {
    static_assert(type<int> == type<int>);
    static_assert(type<void*> == type<void*>);
    static_assert(type<int> != type<const int>);
    static_assert(type<int> != type<void>);
  }

  {
    cfg = fake_cfg{};

    "assertions"_test = [] { expect(42_i == 42); };

    assert(1 == std::size(cfg.test_calls));
    assert("assertions"sv == cfg.test_calls[0]);
    assert(0 == std::size(cfg.test_skip_calls));
    assert(0 == std::size(cfg.log_calls));
    assert(0 == cfg.fatal_assertion_calls);
    assert(1 == std::size(cfg.assertion_calls));
    assert(cfg.assertion_calls[0].location.line());
    assert(cfg.assertion_calls[0].result);
  }

  {
    cfg = fake_cfg{};
    cfg.test_filter = "run";

    "run"_test = [] {};
    "ignore"_test = [] {};

    assert(1 == std::size(cfg.test_calls));
    assert("run"sv == cfg.test_calls[0]);
    assert(0 == std::size(cfg.test_skip_calls));
    assert(0 == std::size(cfg.log_calls));
    assert(0 == cfg.fatal_assertion_calls);
  }

  {
    cfg = fake_cfg{};

    "run"_test = [] {};
    skip | "skip"_test = [] {};

    assert(1 == std::size(cfg.test_calls));
    assert("run"sv == cfg.test_calls[0]);
    assert(1 == std::size(cfg.test_skip_calls));
    assert("skip"sv == cfg.test_skip_calls[0]);
    assert(0 == std::size(cfg.log_calls));
    assert(0 == cfg.fatal_assertion_calls);
  }

  {
    cfg = fake_cfg{};

    "logging"_test = [] {
      log << "msg1"
          << "msg2";
    };

    assert(2 == std::size(cfg.log_calls));
    assert("msg1"sv == cfg.log_calls[0]);
    assert("msg2"sv == cfg.log_calls[1]);
  }

  {
    cfg = fake_cfg{};

    "exceptions"_test = [] {
      expect(throws<std::runtime_error>([] { throw std::runtime_error{""}; }))
          << "throws runtime_error";
      expect(throws([] { throw 0; })) << "throws any exception";
      expect(nothrow([] {})) << "doesn't throw";
    };

    assert(3 == std::size(cfg.assertion_calls));
  }

  {
    cfg = fake_cfg{};

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

    assert(4 == std::size(cfg.assertion_calls));
  }

  {
    cfg = fake_cfg{};

    "args"_test = [](const auto& arg) {
      expect(arg > 0_i) << "all values greater than 0";
    } | std::vector{1, 2, 3};

    assert(3 == std::size(cfg.assertion_calls));
  }

  {
    cfg = fake_cfg{};

    "types"_test = []<class T>() {
      expect(std::is_integral_v<T>) << "all types are integrals";
    }
    | std::tuple<bool, int>{};

    assert(2 == std::size(cfg.assertion_calls));
  }

  {
    cfg = fake_cfg{};

    "args and types"_test = []<class TArg>(const TArg& arg) {
      expect(42_i == arg or true_b == arg);
    }
    | std::tuple{true, 42};

    assert(2 == std::size(cfg.assertion_calls));
  }

  {
    cfg = fake_cfg{};

    "scenario"_test = [] {
      given("I have...") = [] {
        when("I run...") = [] {
          then("I expect...") = [] { expect(1_u == 1u); };
          then("I expect...") = [] { expect(1u == 1_u); };
        };
      };
    };

    assert(2 == std::size(cfg.assertion_calls));
  }
}
