//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "boost/ut.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ut = boost::ut;

namespace test_has_member_object {

struct not_defined {};

class private_member_object_value {
  int value;

 public:
  private_member_object_value(int value_) : value(value_) {}
};

struct public_member_object_value {
  int value;
};

struct public_static_member_object_value {
  static int value;
};

struct public_member_function_value {
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  int value() const { return 0; }
};

struct public_static_member_function_value {
  static int value() { return 0; }
};

class private_member_object_epsilon {
  int epsilon;

 public:
  private_member_object_epsilon(int epsilon_) : epsilon(epsilon_) {}
};

struct public_member_object_epsilon {
  int epsilon;
};

struct public_static_member_object_epsilon {
  static int epsilon;
};

struct public_member_function_epsilon {
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  int epsilon() const { return 0; }
};

struct public_static_member_function_epsilon {
  static int epsilon() { return 0; }
};

}  // namespace test_has_member_object

constexpr auto to_string = [](const auto& expr) {
  ut::printer printer{{.none = "", .pass = "", .fail = ""}};
  printer << std::boolalpha << expr;
  return printer.str();
};

namespace {
auto test_assert = [](const bool result,
                      const ut::reflection::source_location& sl =
                          ut::reflection::source_location::current()) {
  if (not result) {
    std::cerr << sl.file_name() << ':' << sl.line() << ":FAILED" << std::endl;
    std::abort();
  }
};
}  // namespace

struct fake_cfg {
  struct assertion_call {
    std::string expr{};
    ut::reflection::source_location location{};
    bool result{};
  };

  struct test_call {
    std::string_view type{};
    std::string name{};
    std::vector<std::string_view> tag{};
    ut::reflection::source_location location{};
    std::any arg{};
  };

  template <class... Ts>
  auto on(ut::events::test<Ts...> test) -> void {
    if (std::find_if(std::cbegin(test.tag), std::cend(test.tag),
                     [](const auto& name) {
                       return ut::utility::is_match(name, "skip");
                     }) != std::cend(test.tag)) {
      on(ut::events::skip<>{.type = test.type, .name = test.name});
      return;
    }
    if (std::empty(test_filter) or std::string_view{test.name} == test_filter) {
      run_calls.push_back({.type = test.type,
                           .name = test.name,
                           .tag = test.tag,
                           .location = test.location,
                           .arg = test.arg});
      try {
        test();
      } catch (const ut::events::fatal_assertion&) {
      } catch (const ut::events::exception& exception) {
        exception_calls.push_back(exception.what());
      } catch (const std::exception& exception) {
        exception_calls.push_back(exception.what());
      } catch (...) {
        exception_calls.push_back("Unknown exception");
      }
    }
  }

  template <class... Ts>
  auto on(ut::events::skip<Ts...> test) -> void {
    skip_calls.push_back(
        {.type = test.type, .name = std::string{test.name}, .arg = test.arg});
  }

  template <class TExpr>
  auto on(const ut::events::assertion<TExpr>& assertion) -> bool {
    assertion_calls.push_back({.expr = to_string(assertion.expr),
                               .location = assertion.location,
                               .result = assertion.expr});
    return assertion.expr;
  }

  auto on(ut::events::fatal_assertion fatal_assertion) -> void {
    ++fatal_assertion_calls;
    throw fatal_assertion;
  }

  template <class TMsg>
  auto on(ut::events::log<TMsg> log) -> void {
    log_calls.push_back(log.msg);
  }

  std::vector<test_call> run_calls{};
  std::vector<test_call> skip_calls{};
  std::vector<std::any> log_calls{};
  std::vector<std::string> exception_calls{};
  std::vector<assertion_call> assertion_calls{};
  std::size_t fatal_assertion_calls{};
  std::string test_filter{};
};

template <class T>
class fake_calculator {
 public:
  auto enter(const T& value) -> void { values_.push_back(value); }
  auto name(std::string value) -> void { name_ = std::move(value); }
  auto add() -> void {
    result_ = std::accumulate(std::cbegin(values_), std::cend(values_), 0);
  }
  auto sub() -> void {
    result_ = std::accumulate(std::cbegin(values_) + 1, std::cend(values_),
                              values_.front(), std::minus{});
  }
  [[nodiscard]] auto get() const -> const T& { return result_; }
  [[nodiscard]] auto name() const -> std::string { return name_; }

 private:
  std::vector<T> values_{};
  T result_{};
  std::string name_{};
};

struct test_reporter : ut::reporter<ut::printer> {
  using reporter::asserts_;
  using reporter::tests_;
  using reporter::operator=;
};

struct test_runner : ut::runner<test_reporter> {
  using runner::reporter_;
  using runner::run_;
  using runner::operator=;
};

struct test_empty {
  auto operator()() -> void {}
};

struct test_throw {
  // NOLINTNEXTLINE(hicpp-exception-baseclass)
  auto operator()() -> void { throw 42; }
};

struct test_throw_runtime_error {
  auto operator()() -> void { throw std::runtime_error("exception"); }
};

struct test_assertion_true {
  explicit test_assertion_true(test_runner& _run) : run{_run} {}

  auto operator()() -> void {
    void(run.on(ut::events::assertion<bool>{.expr = true, .location = {}}));
  }

 private:
  test_runner& run;
};

struct test_assertion_false {
  explicit test_assertion_false(test_runner& _run) : run{_run} {}

  auto operator()() -> void {
    void(run.on(ut::events::assertion<bool>{.expr = false, .location = {}}));
  }

 private:
  test_runner& run;
};

struct test_assertions {
  explicit test_assertions(test_runner& _run) : run{_run} {}

  auto operator()() -> void {
    void(run.on(ut::events::assertion<bool>{.expr = false, .location = {}}));
    run.on(ut::events::fatal_assertion{});
    void(run.on(ut::events::assertion<bool>{.expr = true, .location = {}}));
  }

 private:
  test_runner& run;
};

struct test_sub_section {
  explicit test_sub_section(test_runner& _run) : run{_run} {}

  auto operator()() -> void {
    run.on(ut::events::test<test_empty>{.type = "test",
                                        .name = "sub-section",
                                        .location = {},
                                        .arg = ut::none{},
                                        .run = test_empty{}});
  }

 private:
  test_runner& run;
};

struct test_sub_sections {
  explicit test_sub_sections(test_runner& _run) : run{_run} {}

  auto operator()() -> void {
    run.on(ut::events::test<test_empty>{.type = "test",
                                        .name = "sub-section-1",
                                        .location = {},
                                        .arg = ut::none{},
                                        .run = test_empty{}});
    run.on(ut::events::test<test_throw>{.type = "test",
                                        .name = "sub-section-2",
                                        .location = {},
                                        .arg = ut::none{},
                                        .run = test_throw{}});
  }

 private:
  test_runner& run;
};

struct test_summary_reporter : ut::reporter<ut::printer> {
  auto count_summaries(std::size_t& counter) -> void {
    summary_counter_ = &counter;
  }

  auto count_runs(std::size_t& counter) -> void { runs_counter_ = &counter; }

  auto on(ut::events::summary) const -> void {
    if (summary_counter_) {
      ++*summary_counter_;
    }
  }

  auto on(ut::events::run_begin) const -> void {
    if (runs_counter_) {
      ++*runs_counter_;
    }
  }

  std::size_t* summary_counter_{};
  std::size_t* runs_counter_{};
};

struct test_summary_runner : ut::runner<test_summary_reporter> {
  using runner::reporter_;
};

namespace ns {
namespace {
template <char... Cs>
constexpr auto operator""_i() -> int {
  return sizeof...(Cs);
}
auto f() -> int { return 0_i; }
}  // namespace
}  //  namespace ns
namespace {
auto f() -> int {
  using namespace ns;
  return 42_i;
}
}  // namespace

struct custom {
  int i{};

  friend auto operator==(const custom& lhs, const custom& rhs) {
    return lhs.i == rhs.i;
  }

  friend auto operator!=(const custom& lhs, const custom& rhs) {
    return lhs.i != rhs.i;
  }

  friend auto operator<<(std::ostream& os, const custom& c) -> std::ostream& {
    return os << "custom{" << c.i << '}';
  }
};

struct custom_vec : std::vector<int> {
  using std::vector<int>::vector;

  friend auto operator<<(std::ostream& os, const custom_vec& c)
      -> std::ostream& {
    os << "custom_vec{";
    if (!c.empty()) {
      os << c.front();
      std::for_each(std::next(c.begin()), c.end(),
                    [&os](int const v) { os << ", " << v; });
    }
    os << '}';
    return os;
  }
};

struct custom_non_printable_type {
  int value;
};

struct custom_printable_type {
  int value;
};

// NOLINTBEGIN(misc-use-anonymous-namespace)
static std::string format_test_parameter(const custom_printable_type& value,
                                         [[maybe_unused]] const int counter) {
  return "custom_printable_type(" + std::to_string(value.value) + ")";
}

template <class... Ts>
static auto ut::cfg<ut::override, Ts...> = fake_cfg{};
// NOLINTEND(misc-use-anonymous-namespace)

int main() {  // NOLINT(readability-function-size)
  {
    using namespace ut;
    using namespace std::literals::string_view_literals;

    {
      static_assert(std::is_same_v<type_traits::list<>,
                                   type_traits::function_traits<void()>::args>);
      static_assert(
          std::is_same_v<void,
                         type_traits::function_traits<void()>::result_type>);
      static_assert(
          std::is_same_v<type_traits::list<int>,
                         type_traits::function_traits<void(int)>::args>);
      static_assert(
          std::is_same_v<
              type_traits::list<int, const float&>,
              type_traits::function_traits<void(int, const float&)>::args>);
      static_assert(
          std::is_same_v<int,
                         type_traits::function_traits<int()>::result_type>);
    }

    {
      static_assert(!type_traits::has_static_member_object_value_v<
                    test_has_member_object::not_defined>);
      static_assert(!type_traits::has_static_member_object_value_v<
                    test_has_member_object::private_member_object_value>);
      static_assert(!type_traits::has_static_member_object_value_v<
                    test_has_member_object::public_member_object_value>);
      static_assert(type_traits::has_static_member_object_value_v<
                    test_has_member_object::public_static_member_object_value>);
      static_assert(!type_traits::has_static_member_object_value_v<
                    test_has_member_object::public_member_function_value>);
      static_assert(
          !type_traits::has_static_member_object_value_v<
              test_has_member_object::public_static_member_function_value>);

      static_assert(!type_traits::has_static_member_object_epsilon_v<
                    test_has_member_object::not_defined>);
      static_assert(!type_traits::has_static_member_object_epsilon_v<
                    test_has_member_object::private_member_object_epsilon>);
      static_assert(!type_traits::has_static_member_object_epsilon_v<
                    test_has_member_object::public_member_object_epsilon>);
      static_assert(
          type_traits::has_static_member_object_epsilon_v<
              test_has_member_object::public_static_member_object_epsilon>);
      static_assert(!type_traits::has_static_member_object_epsilon_v<
                    test_has_member_object::public_member_function_epsilon>);
      static_assert(
          !type_traits::has_static_member_object_epsilon_v<
              test_has_member_object::public_static_member_function_epsilon>);
    }

    {
      static_assert("void" == reflection::type_name<void>());
      static_assert("int" == reflection::type_name<int>());

#if defined(_MSC_VER) and not defined(__clang__)
      static_assert("struct fake_cfg" == reflection::type_name<fake_cfg>());
#else
      static_assert("fake_cfg" == reflection::type_name<fake_cfg>());
#endif
    }

    {
      static_assert(utility::regex_match("", ""));
      static_assert(utility::regex_match("hello", "hello"));
      static_assert(utility::regex_match("hello", "h.llo"));
      static_assert(utility::regex_match("hello", "he..o"));
      static_assert(not utility::regex_match("hello", "hella"));
      static_assert(not utility::regex_match("hello", "helao"));
      static_assert(not utility::regex_match("hello", "hlllo"));
      static_assert(not utility::regex_match("hello", ""));
      static_assert(not utility::regex_match("", "hello"));
      static_assert(not utility::regex_match("hi", "hello"));
      static_assert(not utility::regex_match("hello there", "hello"));
    }

    {
      test_assert(utility::is_match("", ""));
      test_assert(utility::is_match("", "*"));
      test_assert(utility::is_match("abc", "abc"));
      test_assert(utility::is_match("abc", "a?c"));
      test_assert(utility::is_match("abc", "a*"));
      test_assert(utility::is_match("abc", "a*c"));

      test_assert(not utility::is_match("", "abc"));
      test_assert(not utility::is_match("abc", "b??"));
      test_assert(not utility::is_match("abc", "a*d"));
      test_assert(not utility::is_match("abc", "*C"));
    }

    {
      test_assert(std::vector<std::string_view>{} ==
                  utility::split<std::string_view>("", "."));
      test_assert(std::vector<std::string_view>{"a"} ==
                  utility::split<std::string_view>("a.", "."));
      test_assert(std::vector<std::string_view>{"a", "b"} ==
                  utility::split<std::string_view>("a.b", "."));
      test_assert(std::vector<std::string_view>{"a", "b", "cde"} ==
                  utility::split<std::string_view>("a.b.cde", "."));
    }

    {
      test_assert(std::vector<std::string>{"Greg Smith"} ==
                  utility::match(std::string{R"(named "{name}")"},
                                 std::string{R"(named "Greg Smith")"}));
      test_assert(std::vector<std::string>{"Greg Smith"} ==
                  utility::match(std::string{"named '{name}'"},
                                 std::string{"named 'Greg Smith'"}));
      test_assert(std::vector<std::string>{"4", "x y"} ==
                  utility::match(std::string{R"({n} and "{s}")"},
                                 std::string{R"(4 and "x y")"}));
      test_assert(std::empty(utility::match(std::string{R"(named "{name}")"},
                                            std::string{"other"})));
    }

    {
      static_assert("true"_b);
      static_assert((not "true"_b) != "true"_b);
      static_assert("named"_b);
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
      static_assert(42 == 42_i8);
      static_assert(42 == 42_i16);
      static_assert(42 == 42_i32);
      static_assert(42 == 42_i64);
      static_assert(42u == 42_u8);
      static_assert(42u == 42_u16);
      static_assert(42u == 42_u32);
      static_assert(42u == 42_u64);
      static_assert(42.42f == 42.42_f);
      static_assert(42.42 == 42.42_d);
      static_assert(240.209996948 == 240.209996948_d);
      static_assert(static_cast<long double>(42.42) == 42.42_ld);
      static_assert(240.20999694824218 == 240.20999694824218_ld);
      static_assert(0 == 0_i);
      static_assert(10'000 == 10'000_i);
      static_assert(42'000'000 == 42'000'000_i);
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
      static_assert(not std::is_same_v<decltype(_sc(42)), decltype(_c(42))>);
      static_assert(not std::is_same_v<decltype(42_sc), decltype(42_c)>);
      static_assert(not std::is_same_v<decltype(_sc(42)), decltype(_uc(42u))>);
      static_assert(not std::is_same_v<decltype(42_sc), decltype(42_uc)>);
    }

    {
      static_assert(_i(42) == 42_i);
      static_assert(_b(true));
      static_assert(not _b(false));
      static_assert(_s(42) == 42_s);
      static_assert(_c(42) == 42_c);
      static_assert(_sc(42) == 42_sc);
      static_assert(_l(42) == 42_l);
      static_assert(_ll(42) == 42_ll);
      static_assert(_uc(42u) == 42_uc);
      static_assert(_us(42u) == 42_us);
      static_assert(_us(42u) == 42_us);
      static_assert(_ul(42u) == 42_ul);
      static_assert(_ull(42u) == 42_ull);
      static_assert(_ull(18'446'744'073'709'551'615ull) ==
                    18'446'744'073'709'551'615_ull);
    }

    {
      test_assert(_f(42.101f) == 42.101_f);
      test_assert(_f(42.101f, 0.01f) == 42.10_f);
      test_assert(_f(42.101f, 0.1f) != 42.1000_f);
      test_assert(_f(42.1010001f, 0.1f) == 42.1_f);
      test_assert(_f(42.101f) != 42.10_f);
      test_assert(_f(42.101f) != 42.100_f);
      test_assert(_f(42.10f) == 42.1_f);
      test_assert(_f(42.42f) == 42.42_f);
      test_assert(_d(42.42) == 42.420_d);
      test_assert(_d(42.0) == 42.0_d);
      test_assert(_d(42.) == 42._d);
      test_assert(_ld{static_cast<long double>(42.42)} == 42.42_ld);

      test_assert(42_i == 42_i);
      test_assert(1234._f == 1234.f);
      test_assert(1234.56_f == 1234.56f);
      test_assert(12345678.9f == 12345678.9_f);
      test_assert(111111.42f == 111111.42_f);
      test_assert(1111111111.42 == 1111111111.42_d);
      test_assert(_d(1.e19) == 1.e19);
      test_assert(1.e19 == _d(1.e19));
      test_assert(_d(1.e300) == 1.e300);
      test_assert(_f(1.e20f) == 1.e20f);
      test_assert(42_i == 42);
      test_assert(43 == 43_i);
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
      test_assert("true"_b != not "true"_b);
      test_assert(42 >= 42_i);
      test_assert(43 >= 42_i);
      test_assert(42 <= 42_i);
      test_assert(42 <= 43_i);
      test_assert((not "true"_b) == (not "true"_b));
    }

    {
      test_assert("true"_b and 42_i == 42_i);
      test_assert(not "true"_b or 42_i == 42_i);
      test_assert("true"_b or 42_i != 42_i);
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
      test_assert("not true" == to_string(!"true"_b));
      test_assert("42 == 42" == to_string(42_i == 42));
      test_assert("42 != 42" == to_string(42_i != 42));
      test_assert("42 > 0" == to_string(42_ul > 0_ul));
      test_assert("int == float" == to_string(type<int> == type<float>));
      test_assert("int == float" == to_string(type<>(int{}) == type<float>));
      test_assert("void != double" == to_string(type<void> != type<double>));
      test_assert("(true or 42.42 == 12.34)" ==
                  to_string("true"_b or (42.42_d == 12.34)));
      test_assert("(not 1 == 2 and str == str2)" ==
                  to_string(not(1_i == 2) and ("str"sv == "str2"sv)));
      test_assert("lhs == rhs" == to_string("lhs"_b == "rhs"_b));
    }

    {
      constexpr auto return_int = []() -> int { return {}; };
      static_assert(type<int> == return_int());
      static_assert(type<float> != return_int());
      static_assert(type<const int&> != return_int());
      static_assert(type<int&> != return_int());
      static_assert(type<int*> != return_int());
      static_assert(type<double> != return_int());
      static_assert(type<int> == type<int>);
      static_assert(!(type<int> != type<int>));
      static_assert(type<void> == type<void>);
      static_assert(type<void*> == type<void*>);
      static_assert(type<int> != type<const int>);
      static_assert(type<int> != type<void>);
    }

    {
      std::stringstream out{};
      std::stringstream err{};
      auto* old_cout = std::cout.rdbuf(out.rdbuf());
      auto* old_cerr = std::cerr.rdbuf(err.rdbuf());

      auto reporter = test_reporter{};

      reporter.on(events::test_begin{});
      reporter.on(events::test_run{});
      reporter.on(events::assertion_pass<bool>{
          .expr = true, .location = reflection::source_location::current()});
      reporter.on(events::assertion_fail<bool>{
          .expr = false, .location = reflection::source_location::current()});
      reporter.on(events::fatal_assertion{});
      reporter.on(events::test_end{});

      reporter.on(events::summary{});
      test_assert(std::empty(out.str()));
      test_assert(not std::empty(err.str()));
      test_assert(1 == reporter.asserts_.pass);
      test_assert(1 == reporter.asserts_.fail);
      test_assert(0 == reporter.tests_.skip);
      test_assert(1 == reporter.tests_.fail);
      test_assert(0 == reporter.tests_.pass);

      reporter.on(events::test_skip{});
      test_assert(1 == reporter.tests_.skip);

      std::cout.rdbuf(old_cout);
      std::cerr.rdbuf(old_cerr);
    }

    {
      test_runner run;
      auto& reporter = run.reporter_;
      run.run_ = true;

      run.on(events::test<test_empty>{.type = "test",
                                      .name = "run",
                                      .location = {},
                                      .arg = none{},
                                      .run = test_empty{}});
      test_assert(1 == reporter.tests_.pass);
      test_assert(0 == reporter.tests_.fail);
      test_assert(0 == reporter.tests_.skip);

      run.on(events::skip<>{.type = "test", .name = "skip", .arg = none{}});
      test_assert(1 == reporter.tests_.pass);
      test_assert(0 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run = {.filter = "unknown"};
      run.on(events::test<test_empty>{.type = "test",
                                      .name = "filter",
                                      .location = {},
                                      .arg = none{},
                                      .run = test_empty{}});
      test_assert(1 == reporter.tests_.pass);
      test_assert(0 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run = options{};

      run = {.filter = "filter"};
      run.on(events::test<test_empty>{.type = "test",
                                      .name = "filter",
                                      .location = {},
                                      .arg = none{},
                                      .run = test_empty{}});
      test_assert(2 == reporter.tests_.pass);
      test_assert(0 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run = options{};

      run.on(
          events::test<test_assertion_true>{.type = "test",
                                            .name = "pass",
                                            .location = {},
                                            .arg = none{},
                                            .run = test_assertion_true{run}});

      test_assert(3 == reporter.tests_.pass);
      test_assert(0 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run.on(
          events::test<test_assertion_false>{.type = "test",
                                             .name = "fail",
                                             .location = {},
                                             .arg = none{},
                                             .run = test_assertion_false{run}});
      test_assert(3 == reporter.tests_.pass);
      test_assert(1 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run.on(events::test<test_throw>{.type = "test",
                                      .name = "exception",
                                      .location = {},
                                      .arg = none{},
                                      .run = test_throw{}});
      test_assert(3 == reporter.tests_.pass);
      test_assert(2 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run.on(events::test<test_throw_runtime_error>{
          .type = "test",
          .name = "exception",
          .location = {},
          .arg = none{},
          .run = test_throw_runtime_error{}});
      test_assert(3 == reporter.tests_.pass);
      test_assert(3 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run.on(events::test<test_sub_section>{.type = "test",
                                            .name = "section",
                                            .location = {},
                                            .arg = none{},
                                            .run = test_sub_section{run}});
      test_assert(4 == reporter.tests_.pass);
      test_assert(3 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run = {.filter = "section"};
      run.on(events::test<test_sub_sections>{.type = "test",
                                             .name = "section",
                                             .location = {},
                                             .arg = none{},
                                             .run = test_sub_sections{run}});
      test_assert(4 == reporter.tests_.pass);
      test_assert(4 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);

      run = options{};

      run = {.filter = "section.sub-section-1"};
      run.on(events::test<test_sub_sections>{.type = "test",
                                             .name = "section",
                                             .location = {},
                                             .arg = none{},
                                             .run = test_sub_sections{run}});
      test_assert(5 == reporter.tests_.pass);
      test_assert(4 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);
      run = options{};

      run = {.filter = "section.sub-section-2"};
      run.on(events::test<test_sub_sections>{.type = "test",
                                             .name = "section",
                                             .location = {},
                                             .arg = none{},
                                             .run = test_sub_sections{run}});
      test_assert(5 == reporter.tests_.pass);
      test_assert(5 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);
      run = options{};

      run = {.filter = "section.sub-section-*"};
      run.on(events::test<test_sub_sections>{.type = "test",
                                             .name = "section",
                                             .location = {},
                                             .arg = none{},
                                             .run = test_sub_sections{run}});
      test_assert(5 == reporter.tests_.pass);
      test_assert(6 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);
      run = options{};

      run.on(
          events::test<test_assertion_true>{.type = "test",
                                            .name = "normal",
                                            .location = {},
                                            .arg = none{},
                                            .run = test_assertion_true{run}});
      test_assert(6 == reporter.tests_.pass);
      test_assert(6 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);
      
      /*run.on(events::test<test_assertions>{.type = "test",
                                           .name = "fatal",
                                           .location = {},
                                           .arg = none{},
                                           .run = test_assertions{run}});
      test_assert(6 == reporter.tests_.pass);
      test_assert(7 == reporter.tests_.fail);
      test_assert(1 == reporter.tests_.skip);*/

      reporter = printer{};
    }

    {
      std::size_t summary_count = 0;
      {
        test_summary_runner run;
        run.reporter_.count_summaries(summary_count);
        test_assert(!run.run({.report_errors = true}));
      }
      test_assert(1 == summary_count);
    }

    {
      std::size_t run_count = 0;
      {
        auto run = test_summary_runner{};
        run.reporter_.count_runs(run_count);
        test_assert(!run.run({.report_errors = true}));
      }
      test_assert(1 == run_count);
    }

    auto& test_cfg = ut::cfg<ut::override>;

    {
      test_cfg = fake_cfg{};
      test_cfg.test_filter = "run";

      "run"_test = [] {};
      "ignore"_test = [] {};

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("test"sv == test_cfg.run_calls[0].type);
      test_assert("run"sv == test_cfg.run_calls[0].name);
      void(std::any_cast<none>(test_cfg.run_calls[0].arg));
      test_assert(std::empty(test_cfg.skip_calls));
      test_assert(std::empty(test_cfg.log_calls));
      test_assert(0 == test_cfg.fatal_assertion_calls);
    }

    {
      test_cfg = fake_cfg{};

      "run"_test = [] {};
      skip / "skip"_test = [] {};

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("run"sv == test_cfg.run_calls[0].name);
      test_assert(1 == std::size(test_cfg.skip_calls));
      test_assert("test"sv == test_cfg.skip_calls[0].type);
      test_assert("skip"sv == test_cfg.skip_calls[0].name);
      void(std::any_cast<none>(test_cfg.skip_calls[0].arg));
      test_assert(std::empty(test_cfg.log_calls));
      test_assert(0 == test_cfg.fatal_assertion_calls);
    }

    {
      test_cfg = fake_cfg{};

      tag("tag1") / tag("tag2") / "tag"_test = [] {};

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("tag"sv == test_cfg.run_calls[0].name);
      test_assert(std::empty(test_cfg.skip_calls));
      test_assert(std::empty(test_cfg.log_calls));
      test_assert(0 == test_cfg.fatal_assertion_calls);
    }

    {
      test_cfg = fake_cfg{};
      int run_count{};

      skip / "skipped range"_test = [&](auto) { ++run_count; } |
                                   std::vector{1, 2};
      skip / "skipped tuple"_test = [&](auto) { ++run_count; } |
                                   std::tuple{42, 'x'};
      skip / "skipped types"_test = [&]<class T>() { ++run_count; } |
                                   std::tuple<int, char>{};

      test_assert(0 == run_count);
      test_assert(std::empty(test_cfg.run_calls));
      test_assert(6 == std::size(test_cfg.skip_calls));
      test_assert("skipped range (1)"sv == test_cfg.skip_calls[0].name);
      test_assert("skipped range (2)"sv == test_cfg.skip_calls[1].name);
      test_assert("skipped tuple (42, int)"sv == test_cfg.skip_calls[2].name);
      test_assert("skipped tuple (x, char)"sv == test_cfg.skip_calls[3].name);
      test_assert("skipped types (int)"sv == test_cfg.skip_calls[4].name);
      test_assert("skipped types (char)"sv == test_cfg.skip_calls[5].name);
    }

    {
      test_cfg = fake_cfg{};
      int run_count{};

      tag("tag1") / tag("tag2") / "tagged range"_test = [&](auto) {
        ++run_count;
      } | std::array{1, 2};
      tag("tag1") / tag("tag2") / "tagged tuple"_test = [&](auto) {
        ++run_count;
      } | std::tuple{42, 'x'};
      tag("tag1") / "empty range"_test = [&](auto) { ++run_count; } |
                                       std::vector<int>{};
      tag("tag1") / "empty tuple"_test = [&]<class T>() { ++run_count; } |
                                       std::tuple<>{};

      test_assert(4 == run_count);
      test_assert(4 == std::size(test_cfg.run_calls));
      for (const auto& call : test_cfg.run_calls) {
        test_assert((std::vector<std::string_view>{"tag1", "tag2"} == call.tag));
      }
      test_assert(std::empty(test_cfg.skip_calls));
    }

    {
      test_cfg = fake_cfg{};
      std::string_view callback_type;
      std::string_view callback_name;

      "callback"_test = [&](std::string_view type, std::string_view name) {
        callback_type = type;
        callback_name = name;
      };

      test_assert("test"sv == callback_type);
      test_assert("callback"sv == callback_name);

      struct ref_qualified_callback {
        int& calls;

        void operator()(std::string_view, std::string_view) const { ++calls; }
        void operator()(std::string_view, std::string_view,
                        const std::vector<std::string_view>&) && {}
      };
      int callback_calls{};
      "ref-qualified callback"_test = ref_qualified_callback{callback_calls};
      test_assert(1 == callback_calls);
    }

    {
      test_cfg = fake_cfg{};

      "logging"_test = [] {
        boost::ut::log << "msg1"
                       << "msg2" << std::string{"msg3"} << 42;
      };

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("logging"sv == test_cfg.run_calls[0].name);
      test_assert(8 == std::size(test_cfg.log_calls));
      test_assert('\n' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("msg1"sv ==
                  std::any_cast<const char*>(test_cfg.log_calls[1]));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[2]));
      test_assert("msg2"sv ==
                  std::any_cast<const char*>(test_cfg.log_calls[3]));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[4]));
      test_assert("msg3"sv ==
                  std::any_cast<std::string>(test_cfg.log_calls[5]));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[6]));
      test_assert(42 == std::any_cast<int>(test_cfg.log_calls[7]));
    }
#if defined(BOOST_UT_HAS_FORMAT)
    {
      test_cfg = fake_cfg{};

      "logging with format"_test = [] {
        boost::ut::log("{:<10} {:#x}\n{:<10} {}\n", "expected:", 42,
                       std::string("actual:"), 99);
      };

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("logging with format"sv == test_cfg.run_calls[0].name);
      test_assert(1 == std::size(test_cfg.log_calls));
      test_assert("expected:  0x2a\nactual:    99\n"sv ==
                  std::any_cast<std::string>(test_cfg.log_calls[0]));
    }
#endif
    {
      test_cfg = fake_cfg{};
      expect(true) << "true msg" << true;
      expect(false) << "false msg" << false;

      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert("true" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);

      test_assert("false" == test_cfg.assertion_calls[1].expr);
      test_assert(not test_cfg.assertion_calls[1].result);
      test_assert(4 == std::size(test_cfg.log_calls));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("false msg"sv ==
                  std::any_cast<const char*>(test_cfg.log_calls[1]));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[2]));
      test_assert(not std::any_cast<bool>(test_cfg.log_calls[3]));
    }

    {
      test_cfg = fake_cfg{};
      auto f = [] { return "msg"; };
      expect(false) << f;

      test_assert(1 == std::size(test_cfg.assertion_calls));

      test_assert("false" == test_cfg.assertion_calls[0].expr);
      test_assert(not test_cfg.assertion_calls[0].result);
      test_assert(2 == std::size(test_cfg.log_calls));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("msg"sv == std::any_cast<const char*>(test_cfg.log_calls[1]));
    }

    {
      struct convertible_to_bool {
        constexpr operator bool() const { return false; }
      };

      struct explicit_convertible_to_bool {
        constexpr explicit operator bool() const { return true; }
      };

      test_cfg = fake_cfg{};

      expect(convertible_to_bool{});
      expect(static_cast<bool>(explicit_convertible_to_bool{}));

      test_assert(2 == std::size(test_cfg.assertion_calls));

      test_assert("false" == test_cfg.assertion_calls[0].expr);
      test_assert(not test_cfg.assertion_calls[0].result);

      test_assert("true" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[1].result);
    }

    {
      test_cfg = fake_cfg{};

      // clang-format off
      {
        constexpr auto c = 4;
        auto r = 2;
        expect(constant<4_i == c> and r == 2_i);
      }

      {
        constexpr auto c = 5;
        auto r = 2;
        expect(constant<4_i == c> and r == 2_i) << "fail compile-time";
      }

      {
        constexpr auto c = 4;
        auto r = 3;
        expect(constant<4_i == c> and r == 2_i) << "fail run-time";
      }
      // clang-format on

      test_assert(3 == std::size(test_cfg.assertion_calls));

#if defined(__cpp_nontype_template_parameter_class)
      test_assert("(4 == 4 and 2 == 2)" == test_cfg.assertion_calls[0].expr);
#else
      test_assert("(true and 2 == 2)" == test_cfg.assertion_calls[0].expr);
#endif
      test_assert(test_cfg.assertion_calls[0].result);

      test_assert(4 == std::size(test_cfg.log_calls));

#if defined(__cpp_nontype_template_parameter_class)
      test_assert("(4 == 5 and 2 == 2)" == test_cfg.assertion_calls[1].expr);
#else
      test_assert("(false and 2 == 2)" == test_cfg.assertion_calls[1].expr);
#endif
      test_assert(not test_cfg.assertion_calls[1].result);
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("fail compile-time"sv ==
                  std::any_cast<const char*>(test_cfg.log_calls[1]));

#if defined(__cpp_nontype_template_parameter_class)
      test_assert("(4 == 4 and 3 == 2)" == test_cfg.assertion_calls[2].expr);
#else
      test_assert("(true and 3 == 2)" == test_cfg.assertion_calls[2].expr);
#endif
      test_assert(not test_cfg.assertion_calls[2].result);
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[2]));
      test_assert("fail run-time"sv ==
                  std::any_cast<const char*>(test_cfg.log_calls[3]));
    }

    {
      test_cfg = fake_cfg{};

      expect(1 == 2_i);
      expect(2 == 2_i);
      expect(1 != 2_i);
      expect(-42 != -42_i);
      expect(-1.1_d == -1.1);
      expect((not _b(true)) == not "true"_b);
      expect(2_i > 2_i) << "msg";

      test_assert(7 == std::size(test_cfg.assertion_calls));

      test_assert("1 == 2" == test_cfg.assertion_calls[0].expr);
      test_assert(not test_cfg.assertion_calls[0].result);

      test_assert("2 == 2" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[1].result);

      test_assert("1 != 2" == test_cfg.assertion_calls[2].expr);
      test_assert(test_cfg.assertion_calls[2].result);

      test_assert("-42 != -42" == test_cfg.assertion_calls[3].expr);
      test_assert(not test_cfg.assertion_calls[3].result);

      test_assert("-1.1 == -1.1" == test_cfg.assertion_calls[4].expr);
      test_assert(test_cfg.assertion_calls[4].result);

      test_assert("not true == not true" == test_cfg.assertion_calls[5].expr);
      test_assert(test_cfg.assertion_calls[5].result);

      test_assert("2 > 2" == test_cfg.assertion_calls[6].expr);
      test_assert(not test_cfg.assertion_calls[6].result);
      test_assert(2 == std::size(test_cfg.log_calls));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("msg"sv == std::any_cast<const char*>(test_cfg.log_calls[1]));
    }

    {
      test_cfg = fake_cfg{};

      expect(that % 42 == 42);

      test_assert(1 == std::size(test_cfg.assertion_calls));
      test_assert("42 == 42" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert(0 == std::size(test_cfg.log_calls));

      expect(that % 1 == 2 or that % 3 > 7) << "msg";
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert("(1 == 2 or 3 > 7)" == test_cfg.assertion_calls[1].expr);
      test_assert(not test_cfg.assertion_calls[1].result);
      test_assert(2 == std::size(test_cfg.log_calls));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("msg"sv == std::any_cast<const char*>(test_cfg.log_calls[1]));

      const auto f = false;
      expect(3_i > 5 and that % not true == f);
      test_assert(3 == std::size(test_cfg.assertion_calls));
      test_assert("(3 > 5 and false == false)" ==
                  test_cfg.assertion_calls[2].expr);
      test_assert(not test_cfg.assertion_calls[2].result);
    }

    {
      test_cfg = fake_cfg{};

      constexpr auto is_gt = [](auto N) {
        return [=](auto value) { return that % value > N; };
      };

      expect(is_gt(42)(99));
      expect(is_gt(2)(1));

      test_assert(2 == std::size(test_cfg.assertion_calls));

      test_assert("99 > 42" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);

      test_assert("1 > 2" == test_cfg.assertion_calls[1].expr);
      test_assert(not test_cfg.assertion_calls[1].result);
    }

    {
      test_cfg = fake_cfg{};

      expect(eq(42, 42));

      test_assert(1 == std::size(test_cfg.assertion_calls));
      test_assert("42 == 42" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert(0 == std::size(test_cfg.log_calls));

      expect(eq(1, 2) or gt(3, 7)) << "msg";
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert("(1 == 2 or 3 > 7)" == test_cfg.assertion_calls[1].expr);
      test_assert(not test_cfg.assertion_calls[1].result);
      test_assert(2 == std::size(test_cfg.log_calls));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("msg"sv == std::any_cast<const char*>(test_cfg.log_calls[1]));

      const auto f = false;
      expect(3_i > 5 and eq(not true, f));
      test_assert(3 == std::size(test_cfg.assertion_calls));
      test_assert("(3 > 5 and false == false)" ==
                  test_cfg.assertion_calls[2].expr);
      test_assert(not test_cfg.assertion_calls[2].result);
    }

    {
      test_cfg = fake_cfg{};

      expect(approx(42_i, 43_i, 2_i));
      test_assert(1 == std::size(test_cfg.assertion_calls));
      test_assert("42 ~ (43 +/- 2)" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);

      expect(approx(3.141592654, 3.1415926536, 1e-9));
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert("3.14159 ~ (3.14159 +/- 1e-09)" ==
                  test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[1].result);

      expect(approx(1_u, 2_u, 3_u));
      test_assert(3 == std::size(test_cfg.assertion_calls));
      test_assert("1 ~ (2 +/- 3)" == test_cfg.assertion_calls[2].expr);
      test_assert(test_cfg.assertion_calls[2].result);
    }

    {
      test_cfg = fake_cfg{};

      using namespace std::literals::string_view_literals;
      using namespace std::literals::string_literals;

      {
        std::stringstream str{};
        str << "should print" << ' ' << "string"s
            << " and "
            << "string_view"sv;
        test_assert("should print string and string_view" == str.str());
      }

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

      test_assert("str == str" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);

      test_assert("str1 != str2" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[1].result);

      test_assert("str == str" == test_cfg.assertion_calls[2].expr);
      test_assert(test_cfg.assertion_calls[2].result);

      test_assert("str1 != str2" == test_cfg.assertion_calls[3].expr);
      test_assert(test_cfg.assertion_calls[3].result);

      test_assert("str == str" == test_cfg.assertion_calls[4].expr);
      test_assert(test_cfg.assertion_calls[4].result);

      test_assert("str1 != str2" == test_cfg.assertion_calls[5].expr);
      test_assert(test_cfg.assertion_calls[5].result);

      test_assert("str == str" == test_cfg.assertion_calls[6].expr);
      test_assert(test_cfg.assertion_calls[6].result);

      test_assert("str1 != str2" == test_cfg.assertion_calls[7].expr);
      test_assert(test_cfg.assertion_calls[7].result);

      test_assert("str1 == str2" == test_cfg.assertion_calls[8].expr);
      test_assert(not test_cfg.assertion_calls[8].result);
    }

    {
      test_cfg = fake_cfg{};

      expect(std::vector<int>{} == std::vector<int>{});
      expect(std::vector{'a', 'b'} == std::vector{'a', 'b'});
      expect(std::vector{1, 2, 3} == std::vector{1, 2});

      test_assert(3 == std::size(test_cfg.assertion_calls));

      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("{} == {}" == test_cfg.assertion_calls[0].expr);

      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("{a, b} == {a, b}" == test_cfg.assertion_calls[1].expr);

      test_assert(not test_cfg.assertion_calls[2].result);
      test_assert("{1, 2, 3} == {1, 2}" == test_cfg.assertion_calls[2].expr);
    }

    {
      test_cfg = fake_cfg{};

      expect(std::array{1} != std::array{2});
      expect(std::array{1, 2, 3} == std::array{3, 2, 1});

      test_assert(2 == std::size(test_cfg.assertion_calls));

      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("{1} != {2}" == test_cfg.assertion_calls[0].expr);

      test_assert(not test_cfg.assertion_calls[1].result);
      test_assert("{1, 2, 3} == {3, 2, 1}" == test_cfg.assertion_calls[1].expr);
    }

    {
      test_cfg = fake_cfg{};

      expect(custom{42} == _t{custom{42}});
      expect(_t{custom{42}} == _t{custom{42}});
      expect(_t{custom{42}} == custom{42});
      expect(_t{custom{42}} != custom{24});
      expect(_t{custom{1}} == custom{2} or 1_i == 22);

      test_assert(5 == std::size(test_cfg.assertion_calls));

      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("custom{42} == custom{42}" ==
                  test_cfg.assertion_calls[0].expr);

      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("custom{42} == custom{42}" ==
                  test_cfg.assertion_calls[1].expr);

      test_assert(test_cfg.assertion_calls[2].result);
      test_assert("custom{42} == custom{42}" ==
                  test_cfg.assertion_calls[2].expr);

      test_assert(test_cfg.assertion_calls[3].result);
      test_assert("custom{42} != custom{24}" ==
                  test_cfg.assertion_calls[3].expr);

      test_assert(not test_cfg.assertion_calls[4].result);
      test_assert("(custom{1} == custom{2} or 1 == 22)" ==
                  test_cfg.assertion_calls[4].expr);
    }

    {
      test_cfg = fake_cfg{};

      expect(custom_vec{42, 5} == custom_vec{42, 5});
      expect(custom_vec{42, 5, 3} != custom_vec{42, 5, 6});

      test_assert(2 == std::size(test_cfg.assertion_calls));

      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("custom_vec{42, 5} == custom_vec{42, 5}" ==
                  test_cfg.assertion_calls[0].expr);

      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("custom_vec{42, 5, 3} != custom_vec{42, 5, 6}" ==
                  test_cfg.assertion_calls[1].expr);
    }

    {
      test_cfg = fake_cfg{};

      "assertions"_test = [] { expect(42_i == 42); };

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("assertions"sv == test_cfg.run_calls[0].name);
      test_assert(std::empty(test_cfg.skip_calls));
      test_assert(std::empty(test_cfg.log_calls));
      test_assert(0 == test_cfg.fatal_assertion_calls);
      test_assert(1 == std::size(test_cfg.assertion_calls));
      test_assert("42 == 42" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);
    }

    {
      test_cfg = fake_cfg{};

      "fatal assertions via function"_test = [] {
        expect(fatal(1_i == 1));
        expect(fatal(2 != 2_i)) << "fatal";
      };

      "fatal assertions via operator"_test = [] {
        expect((1_i == 1) >> fatal);
        expect((2 != 2_i) >> fatal) << "fatal";
      };

      test_assert(2 == std::size(test_cfg.run_calls));
      test_assert(4 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);

      test_assert("1 == 1" == test_cfg.assertion_calls[0].expr);
      test_assert(not test_cfg.assertion_calls[1].result);
      test_assert("2 != 2" == test_cfg.assertion_calls[1].expr);

      test_assert("1 == 1" == test_cfg.assertion_calls[2].expr);
      test_assert(not test_cfg.assertion_calls[3].result);
      test_assert("2 != 2" == test_cfg.assertion_calls[3].expr);

      test_assert(2 == test_cfg.fatal_assertion_calls);
      test_assert(std::empty(test_cfg.log_calls));
    }

    {
      test_cfg = fake_cfg{};

      "fatal assertions logging"_test = [] {
        expect(2 != 2_i) << "fatal" << fatal;
      };

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert(1 == std::size(test_cfg.assertion_calls));
      test_assert(not test_cfg.assertion_calls[0].result);
      test_assert("2 != 2" == test_cfg.assertion_calls[0].expr);
      test_assert(1 == test_cfg.fatal_assertion_calls);
      test_assert(2 == std::size(test_cfg.log_calls));
      test_assert(' ' == std::any_cast<char>(test_cfg.log_calls[0]));
      test_assert("fatal"sv ==
                  std::any_cast<const char*>(test_cfg.log_calls[1]));
    }

    {
      // NOLINTBEGIN(hicpp-exception-baseclass)
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

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("exceptions"sv == test_cfg.run_calls[0].name);
      test_assert(6 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
#if defined(_MSC_VER) and not defined(__clang__)
      test_assert("throws<class std::runtime_error>" ==
                  test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("throws" == test_cfg.assertion_calls[1].expr);
      test_assert(not test_cfg.assertion_calls[2].result);
      test_assert("throws<class std::runtime_error>" ==
                  test_cfg.assertion_calls[2].expr);
      test_assert(not test_cfg.assertion_calls[3].result);
      test_assert("throws<class std::runtime_error>" ==
                  test_cfg.assertion_calls[3].expr);
#else
      test_assert("throws<std::runtime_error>" ==
                  test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("throws" == test_cfg.assertion_calls[1].expr);
      test_assert(not test_cfg.assertion_calls[2].result);
      test_assert("throws<std::runtime_error>" ==
                  test_cfg.assertion_calls[2].expr);
      test_assert(not test_cfg.assertion_calls[3].result);
      test_assert("throws<std::runtime_error>" ==
                  test_cfg.assertion_calls[3].expr);
#endif
      test_assert(test_cfg.assertion_calls[4].result);
      test_assert("nothrow" == test_cfg.assertion_calls[4].expr);
      test_assert(not test_cfg.assertion_calls[5].result);
      test_assert("nothrow" == test_cfg.assertion_calls[5].expr);

      "should throw"_test = [] {
        auto f = [](const auto should_throw) {
          if (should_throw) {
            throw 42;
          }
          return 42;
        };
        expect(42_i == f(true));
      };

      "std::exception throw"_test = [] {
        throw std::runtime_error("std::exception message");
      };

      "events::exception throw"_test = [] {
        throw events::exception{.msg = "events::exception message"};
      };

      "generic throw"_test = [] { throw 0; };

      test_assert(5 == std::size(test_cfg.run_calls));
      test_assert("should throw"sv == test_cfg.run_calls[1].name);
      test_assert("std::exception throw"sv == test_cfg.run_calls[2].name);
      test_assert("events::exception throw"sv == test_cfg.run_calls[3].name);
      test_assert("generic throw"sv == test_cfg.run_calls[4].name);
      test_assert("std::exception message"sv == test_cfg.exception_calls[1]);
      test_assert("events::exception message"sv == test_cfg.exception_calls[2]);
      test_assert("Unknown exception"sv == test_cfg.exception_calls[3]);
      test_assert(6 == std::size(test_cfg.assertion_calls));
      // NOLINTEND(hicpp-exception-baseclass)
    }

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>) and not defined(EMSCRIPTEN)
    {
      test_cfg = fake_cfg{};

      "abort"_test = [] {
        expect(aborts([] {}));
        expect(aborts([] { throw; }));
      };

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("abort"sv == test_cfg.run_calls[0].name);
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert(not test_cfg.assertion_calls[0].result);
      test_assert("aborts" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("aborts" == test_cfg.assertion_calls[1].expr);
    }
#endif

    {
      test_cfg = fake_cfg{};
      [[maybe_unused]] struct {
      } empty{};

      "[capture empty]"_test = [=] {};

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("[capture empty]"sv == test_cfg.run_calls[0].name);
    }

    {
      test_cfg = fake_cfg{};

      "[vector]"_test = [] {
        std::vector<int> v(5);

        expect((5_ul == std::size(v)) >> fatal);

        "resize bigger"_test = [=] {
          mut(v).resize(10);
          expect(10_ul == std::size(v));
        };

        expect((5_ul == std::size(v)) >> fatal);

        "resize smaller"_test = [=]() mutable {
          v.resize(0);
          expect(0_ul == std::size(v));
        };
      };

      test_assert(3 == std::size(test_cfg.run_calls));
      test_assert("[vector]"sv == test_cfg.run_calls[0].name);
      test_assert("resize bigger"sv == test_cfg.run_calls[1].name);
      test_assert("resize smaller"sv == test_cfg.run_calls[2].name);
      test_assert(4 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("5 == 5" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("10 == 10" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[2].result);
      test_assert("5 == 5" == test_cfg.assertion_calls[2].expr);
      test_assert(test_cfg.assertion_calls[3].result);
      test_assert("0 == 0" == test_cfg.assertion_calls[3].expr);
    }

    {
      test_cfg = fake_cfg{};

      "args vector"_test = [](const auto& arg) {
        expect(arg > 0_i) << "all values greater than 0";
      } | std::vector{1, 2, 3};

      test_assert(3 == std::size(test_cfg.run_calls));
      test_assert("args vector (1)"sv == test_cfg.run_calls[0].name);
      test_assert(1 == std::any_cast<int>(test_cfg.run_calls[0].arg));
      test_assert("args vector (2)"sv == test_cfg.run_calls[1].name);
      test_assert(2 == std::any_cast<int>(test_cfg.run_calls[1].arg));
      test_assert("args vector (3)"sv == test_cfg.run_calls[2].name);
      test_assert(3 == std::any_cast<int>(test_cfg.run_calls[2].arg));
      test_assert(3 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("1 > 0" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("2 > 0" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[2].result);
      test_assert("3 > 0" == test_cfg.assertion_calls[2].expr);
    }

    {
      test_cfg = fake_cfg{};

      "args array"_test = [](const auto& arg) {
        expect(0_i <= arg) << "all values greater than 0";
      } | std::array{99, 11};

      test_assert(2 == std::size(test_cfg.run_calls));
      test_assert("args array (99)"sv == test_cfg.run_calls[0].name);
      test_assert(99 == std::any_cast<int>(test_cfg.run_calls[0].arg));
      test_assert("args array (11)"sv == test_cfg.run_calls[1].name);
      test_assert(11 == std::any_cast<int>(test_cfg.run_calls[1].arg));
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("0 <= 99" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("0 <= 11" == test_cfg.assertion_calls[1].expr);
    }

    {
      test_cfg = fake_cfg{};

      "args string"_test = [](const auto& arg) {
        expect(_c('s') == arg or _c('t') == arg or _c('r') == arg);
      } | std::string{"str"};

      test_assert(3 == std::size(test_cfg.run_calls));
      test_assert("args string (s)"sv == test_cfg.run_calls[0].name);
      test_assert('s' == std::any_cast<char>(test_cfg.run_calls[0].arg));
      test_assert("args string (t)"sv == test_cfg.run_calls[1].name);
      test_assert('t' == std::any_cast<char>(test_cfg.run_calls[1].arg));
      test_assert("args string (r)"sv == test_cfg.run_calls[2].name);
      test_assert('r' == std::any_cast<char>(test_cfg.run_calls[2].arg));
      test_assert(3 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("((s == s or t == s) or r == s)" ==
                  test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("((s == t or t == t) or r == t)" ==
                  test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[2].result);
      test_assert("((s == r or t == r) or r == r)" ==
                  test_cfg.assertion_calls[2].expr);
    }

    {
      test_cfg = fake_cfg{};

      "args map"_test = [](const auto& arg) {
        const auto [key, value] = arg;
        expect(_c(key) == 'a' or _c('b') == key);
        expect(value > 0_i);
      } | std::map<char, int>{{'a', 1}, {'b', 2}};

      test_assert(2 == std::size(test_cfg.run_calls));
      test_assert("args map (1st parameter)"sv == test_cfg.run_calls[0].name);
      test_assert(
          std::pair<const char, int>{'a', 1} ==
          std::any_cast<std::pair<const char, int>>(test_cfg.run_calls[0].arg));
      test_assert("args map (2nd parameter)"sv == test_cfg.run_calls[1].name);
      test_assert(
          std::pair<const char, int>{'b', 2} ==
          std::any_cast<std::pair<const char, int>>(test_cfg.run_calls[1].arg));
      test_assert(4 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("(a == a or b == a)" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("1 > 0" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[2].result);
      test_assert("(b == a or b == b)" == test_cfg.assertion_calls[2].expr);
      test_assert(test_cfg.assertion_calls[3].result);
      test_assert("2 > 0" == test_cfg.assertion_calls[3].expr);
    }

    {
      test_cfg = fake_cfg{};

      "types"_test = []<class T>() {
        expect(std::is_integral<T>{} or
               type<void> == type<std::remove_pointer_t<T>>)
            << "all types are integrals or void";
      } | std::tuple<bool, int, void*>{};

      test_assert(3 == std::size(test_cfg.run_calls));
      test_assert("types (bool)"sv == test_cfg.run_calls[0].name);
      void(std::any_cast<bool>(test_cfg.run_calls[0].arg));
      test_assert("types (int)"sv == test_cfg.run_calls[1].name);
      void(std::any_cast<int>(test_cfg.run_calls[1].arg));
      // the pointer is printed differently on different platforms, so we only check the prefix
      test_assert(test_cfg.run_calls[2].name.starts_with("types (void"));
      void(std::any_cast<void*>(test_cfg.run_calls[2].arg));
      test_assert(3 == std::size(test_cfg.assertion_calls));
      test_assert("(true or void == bool)" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("(true or void == int)" == test_cfg.assertion_calls[1].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("(false or void == void)" ==
                  test_cfg.assertion_calls[2].expr);
      test_assert(test_cfg.assertion_calls[2].result);
    }

    {
      test_cfg = fake_cfg{};

      "args and types"_test = []<class TArg>(const TArg& arg) {
        expect(42_i == arg or arg == _c('x'));
        expect(type<TArg> == type<int> or type<TArg> == type<char>);
      } | std::tuple{42, 'x'};

      test_assert(2 == std::size(test_cfg.run_calls));
      test_assert("args and types (42, int)"sv == test_cfg.run_calls[0].name);
      test_assert(42 == std::any_cast<int>(test_cfg.run_calls[0].arg));
      test_assert("args and types (x, char)"sv == test_cfg.run_calls[1].name);
      test_assert('x' == std::any_cast<char>(test_cfg.run_calls[1].arg));
      test_assert(4 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert(test_cfg.assertion_calls[2].result);
      test_assert(test_cfg.assertion_calls[3].result);
      test_assert("(42 == 42 or 42 == x)" == test_cfg.assertion_calls[0].expr);
      test_assert("(int == int or int == char)" ==
                  test_cfg.assertion_calls[1].expr);
      test_assert("(42 == x or x == x)" == test_cfg.assertion_calls[2].expr);
      test_assert("(char == int or char == char)" ==
                  test_cfg.assertion_calls[3].expr);
    }

    {
      test_cfg = fake_cfg{};

      "parameterized test names"_test =
          []<class TArg>(const TArg& /*arg*/) { expect(true); } |
          std::tuple{custom_non_printable_type{0},
                     std::string{"str"},
                     "str",
                     "str"sv,
                     42.5,
                     false,
                     std::complex{1.5, 2.0},
                     custom_printable_type{42}};

      test_assert(8 == std::size(test_cfg.run_calls));
      test_assert(test_cfg.run_calls[0].name.starts_with(
          "parameterized test names (1st parameter, "));
      test_assert(test_cfg.run_calls[0].name.find(
                      "custom_non_printable_type") != std::string::npos);
      test_assert(test_cfg.run_calls[1].name.starts_with(
          "parameterized test names (2nd parameter, "));
      test_assert(test_cfg.run_calls[1].name.find("string") !=
                  std::string::npos);
      test_assert(test_cfg.run_calls[2].name.starts_with(
          "parameterized test names (3rd parameter, const char"));
      test_assert(test_cfg.run_calls[2].name.find('*') != std::string::npos);
      test_assert(test_cfg.run_calls[3].name.starts_with(
          "parameterized test names (4th parameter, "));
      test_assert(test_cfg.run_calls[3].name.find("string_view") !=
                  std::string::npos);
      test_assert(test_cfg.run_calls[4].name ==
                  "parameterized test names (42.5, double)"sv);
      test_assert(test_cfg.run_calls[5].name ==
                  "parameterized test names (false, bool)"sv);
      test_assert(test_cfg.run_calls[6].name.starts_with(
          "parameterized test names (7th parameter, "));
      test_assert(test_cfg.run_calls[6].name.find("std::complex<double>") !=
                  std::string::npos);
      test_assert(test_cfg.run_calls[7].name.starts_with(
          "parameterized test names (custom_printable_type(42), "));
    }

    {
      // See https://github.com/boost-ext/ut/issues/581
      test_cfg = fake_cfg{};
      test("count asserts using operator|") = []() {
        test("MyTest") = [](auto fixture) {
          expect(fixture > 0);
          expect(false);
        } | std::vector{1, 2, 3, 4};
      };
      test_assert(test_cfg.assertion_calls.size() == 8);

      test_cfg = fake_cfg{};
      test("count asserts using for") = []() {
        for (auto fixture : std::vector{1, 2, 3, 4}) {
          test("MyTest" + std::to_string(fixture)) = [&fixture]() {
            expect(fixture > 0);
            expect(false);
          };
        }
      };
      test_assert(test_cfg.assertion_calls.size() == 8);
    }

    {
      test_cfg = fake_cfg{};

      using namespace ut::bdd;

      "scenario"_test = [] {
        given("I have...") = [] {
          when("I run...") = [] {
            then("I expect...") = [] { expect(1_u == 1u); };
            then("I expect...") = [] { expect(1u == 1_u); };
          };
        };
      };

      test_assert(5 == std::size(test_cfg.run_calls));
      test_assert("scenario"sv == test_cfg.run_calls[0].name);
      test_assert("given"sv == test_cfg.run_calls[1].type);
      test_assert("I have..."sv == test_cfg.run_calls[1].name);
      test_assert("when"sv == test_cfg.run_calls[2].type);
      test_assert("I run..."sv == test_cfg.run_calls[2].name);
      test_assert("then"sv == test_cfg.run_calls[3].type);
      test_assert("I expect..."sv == test_cfg.run_calls[3].name);
      test_assert("then"sv == test_cfg.run_calls[4].type);
      test_assert("I expect..."sv == test_cfg.run_calls[4].name);
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("1 == 1" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("1 == 1" == test_cfg.assertion_calls[1].expr);
    }

    {
      test_cfg = fake_cfg{};

      using namespace ut::bdd;

      feature("feature...") = [] {
        scenario("scenario...") = [] {
          given("given...") = [] {};
          when("when...") = [] {};
          then("then...") = [] {};
        };
      };

      test_assert(5 == std::size(test_cfg.run_calls));
      test_assert("feature..."sv == test_cfg.run_calls[0].name);
      test_assert("feature"sv == test_cfg.run_calls[0].type);
      test_assert("scenario..."sv == test_cfg.run_calls[1].name);
      test_assert("scenario"sv == test_cfg.run_calls[1].type);
      test_assert("given..."sv == test_cfg.run_calls[2].name);
      test_assert("given"sv == test_cfg.run_calls[2].type);
      test_assert("when..."sv == test_cfg.run_calls[3].name);
      test_assert("when"sv == test_cfg.run_calls[3].type);
      test_assert("then..."sv == test_cfg.run_calls[4].name);
      test_assert("then"sv == test_cfg.run_calls[4].type);
    }

    {
      test_cfg = fake_cfg{};

      using namespace ut::spec;

      describe("describe") = [] { it("it") = [] { expect(1_u == 1u); }; };

      test_assert(2 == std::size(test_cfg.run_calls));
      test_assert("describe"sv == test_cfg.run_calls[0].name);
      test_assert("it"sv == test_cfg.run_calls[1].type);
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("1 == 1" == test_cfg.assertion_calls[0].expr);
    }

    {
      test_cfg = fake_cfg{};

      test("name") = [] {};

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("name"sv == test_cfg.run_calls[0].name);
      test_assert("test"sv == test_cfg.run_calls[0].type);
    }

    {
      test_cfg = fake_cfg{};

      should("name") = [] {};

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("name"sv == test_cfg.run_calls[0].name);
      test_assert("test"sv == test_cfg.run_calls[0].type);
    }

    {
      test_cfg = fake_cfg{};

      "should disambiguate operators"_test = [] {
        expect(1_i == ns::f());
        expect(2_i == f());
      };

      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert("should disambiguate operators"sv ==
                  test_cfg.run_calls[0].name);
      test_assert(2 == std::size(test_cfg.assertion_calls));
      test_assert(test_cfg.assertion_calls[0].result);
      test_assert("1 == 1" == test_cfg.assertion_calls[0].expr);
      test_assert(test_cfg.assertion_calls[1].result);
      test_assert("2 == 2" == test_cfg.assertion_calls[1].expr);
    }

#if (__has_builtin(__builtin_FILE) and __has_builtin(__builtin_LINE))
    {
      using namespace ut::bdd;

      test_cfg = fake_cfg{};

      constexpr auto file = std::string_view{__FILE__};
      constexpr auto line = __LINE__;

      "test location"_test = [] {};
      test("test location") = [] {};
      should("test location") = [] {};
      given("test location") = [] {};
      when("test location") = [] {};
      then("test location") = [] {};

      test_assert(6 == std::size(test_cfg.run_calls));
      test_assert(file == test_cfg.run_calls[0].location.file_name());
      test_assert(file == test_cfg.run_calls[1].location.file_name());
      test_assert(file == test_cfg.run_calls[2].location.file_name());
      test_assert(file == test_cfg.run_calls[3].location.file_name());
      test_assert(file == test_cfg.run_calls[4].location.file_name());
      test_assert(file == test_cfg.run_calls[5].location.file_name());

      test_assert(line + 2 == test_cfg.run_calls[0].location.line());
      test_assert(line + 3 == test_cfg.run_calls[1].location.line());
      test_assert(line + 4 == test_cfg.run_calls[2].location.line());
      test_assert(line + 5 == test_cfg.run_calls[3].location.line());
      test_assert(line + 6 == test_cfg.run_calls[4].location.line());
      test_assert(line + 7 == test_cfg.run_calls[5].location.line());
    }
#endif
  }

  {
    using ut::expect;
    using ut::fatal;
    using namespace ut::literals;
    using namespace ut::operators::terse;
    using namespace std::literals::string_view_literals;

    auto& test_cfg = ut::cfg<ut::override>;

    test_cfg = fake_cfg{};

    // clang-format off
    42_i == 42;
    1 == 2_i;
    (0_i == 1 and 1_i > 2) or 3 <= 3_i;
    try {
    ("true"_b == false) >> fatal and 1_i > 0;
    } catch(const ut::events::fatal_assertion&) {}
    2 == 1_i;
    custom{42}%_t == custom{41};
    (3 >= 4_i) << "fatal";
    // clang-format on

    test_assert(7 == std::size(test_cfg.assertion_calls));
    test_assert(test_cfg.fatal_assertion_calls > 0);

    test_assert("42 == 42" == test_cfg.assertion_calls[0].expr);
    test_assert(test_cfg.assertion_calls[0].result);

    test_assert("1 == 2" == test_cfg.assertion_calls[1].expr);
    test_assert(not test_cfg.assertion_calls[1].result);

    test_assert("((0 == 1 and 1 > 2) or 3 <= 3)" ==
                test_cfg.assertion_calls[2].expr);
    test_assert(test_cfg.assertion_calls[2].result);

    test_assert("false" == test_cfg.assertion_calls[3].expr);
    test_assert(not test_cfg.assertion_calls[3].result);

    test_assert("2 == 1" == test_cfg.assertion_calls[4].expr);
    test_assert(not test_cfg.assertion_calls[4].result);

    test_assert("custom{42} == custom{41}" == test_cfg.assertion_calls[5].expr);
    test_assert(not test_cfg.assertion_calls[5].result);

    test_assert("3 >= 4" == test_cfg.assertion_calls[6].expr);
    test_assert(not test_cfg.assertion_calls[6].result);
    test_assert(2 == std::size(test_cfg.log_calls));
    test_assert('\n' == std::any_cast<char>(test_cfg.log_calls[0]));
    test_assert("fatal"sv == std::any_cast<const char*>(test_cfg.log_calls[1]));
  }

#if not defined(_MSC_VER)
  {
    using namespace ut;
    auto& test_cfg = ut::cfg<ut::override>;
    test_cfg = fake_cfg{};

    bdd::gherkin::steps steps_int = [](auto& steps) {
      steps.feature("Calculator") = [&] {
        steps.scenario("*") = [&] {
          steps.given("I have calculator") = [&] {
            fake_calculator<int> calc{};
            steps.when("I enter {value}") = [&](int value) {
              calc.enter(value);
            };
            steps.when("I name it '{value}'") = [&](std::string value) {
              calc.name(std::move(value));
            };
            steps.when("I press add") = [&] { calc.add(); };
            steps.when("I press sub") = [&] { calc.sub(); };
            steps.then("I expect {value}") = [&](int result) {
              expect(that % calc.get() == result);
            };
            steps.then("it's name is '{value}'") = [&](std::string value) {
              expect(calc.name() == value);
            };
          };
        };
      };
    };

    // clang-format off
    "Calculator/int"_test = steps_int |
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

          Scenario: Give a calculator a name
            Given I have calculator
             When I name it 'My calculator'
             Then it's name is 'My calculator'
      )";
    // clang-format on

    test_assert(3 == std::size(test_cfg.assertion_calls));

    test_assert("42 == 42" == test_cfg.assertion_calls[0].expr);
    test_assert(test_cfg.assertion_calls[0].result);

    test_assert("2 == 2" == test_cfg.assertion_calls[1].expr);
    test_assert(test_cfg.assertion_calls[1].result);

    test_assert(test_cfg.assertion_calls[2].result);
  }

  {
    using namespace ut;
    auto& test_cfg = ut::cfg<ut::override>;
    test_cfg = fake_cfg{};

    bdd::gherkin::steps steps_double = [](auto& steps) {
      steps.feature("Calculator") = [&] {
        steps.scenario("*") = [&] {
          steps.given("I have calculator") = [&] {
            fake_calculator<double> calc{};
            steps.when("I enter {value}") = [&](double value) {
              calc.enter(value);
            };
            steps.when("I press add") = [&] { calc.add(); };
            steps.when("I press sub") = [&] { calc.sub(); };
            steps.then("I expect {value}") = [&](double result) {
              expect(that % calc.get() == result);
            };
          };
        };
      };
    };

    // clang-format off
    "Calculator/double"_test = steps_double |
      R"(
        Feature: Calculator

          Scenario: Addition
            Given I have calculator
             When I enter 40.0
             When I enter 2.0
             When I press add
             Then I expect 42.0

          Scenario: Subtraction
            Given I have calculator
             When I enter 4.
             When I enter 2.
             When I press sub
             Then I expect 2.
      )";
    // clang-format on

    test_assert(2 == std::size(test_cfg.assertion_calls));

    test_assert("42 == 42" == test_cfg.assertion_calls[0].expr);
    test_assert(test_cfg.assertion_calls[0].result);

    test_assert("2 == 2" == test_cfg.assertion_calls[1].expr);
    test_assert(test_cfg.assertion_calls[1].result);
  }
  {
    using namespace ut;
    namespace gk = bdd::gherkin;
    using gk::ast::step_type;
    using namespace std::literals::string_literals;

    // ---- parser ----

    {  // 1. Feature, description, tags
      // clang-format off
      const auto doc = gk::parse(R"gk(
@a @b
Feature: Calc
  first line

  second line # not a comment
  # a comment
  third line

Scenario: S
  Given x
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      test_assert(doc.feature.has_value());
      const auto& feature = *doc.feature;
      test_assert(3 == feature.line);
      test_assert("Calc" == feature.name);
      test_assert((std::vector<std::string>{"a", "b"}) == feature.tags);
      test_assert("first line\n\nsecond line # not a comment\nthird line" ==
                  feature.description);
      test_assert(1 == std::size(feature.scenarios));
      test_assert(std::empty(feature.rules));
      test_assert(not feature.background);
    }

    {  // 1. description stops at Background/Rule/Example/Scenario/Outline
      for (const auto* header : {"Background:", "Rule: R", "Example: E",
                                 "Scenario: S", "Scenario Outline: O"}) {
        const auto doc =
            gk::parse("Feature: F\n  desc\n"s + header + "\n  Given x\n");
        test_assert(doc.feature.has_value());
        test_assert("desc" == doc.feature->description);
      }
    }

    {  // 2. errors
      auto doc = gk::parse("\nGiven x\nFeature: F\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(2 == doc.errors[0].line);
      test_assert("expected 'Feature:'" == doc.errors[0].message);
      test_assert(doc.feature.has_value());
      test_assert("F" == doc.feature->name);

      doc = gk::parse("Feature: A\nScenario: S\nFeature: B\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(3 == doc.errors[0].line);
      test_assert(doc.feature.has_value());
      test_assert("A" == doc.feature->name);

      doc = gk::parse("");
      test_assert(not doc.feature);
      test_assert(std::empty(doc.errors));

      doc = gk::parse("\n  \n# language: en\n\n");
      test_assert(not doc.feature);
      test_assert(std::empty(doc.errors));

      doc = gk::parse("Scenario: S\n");
      test_assert(not doc.feature);
      test_assert(1 == std::size(doc.errors));
      test_assert(1 == doc.errors[0].line);
    }

    {  // 3. synonyms
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
  Scenario: A
  Example: B
  Scenario Outline: C
    Examples: X
      | a |
      | 1 |
  Scenario Template: D
    Scenarios: Y
      | a |
      | 1 |
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& scenarios = doc.feature->scenarios;
      test_assert(4 == std::size(scenarios));
      test_assert("Scenario" == scenarios[0].keyword);
      test_assert("A" == scenarios[0].name);
      test_assert("Example" == scenarios[1].keyword);
      test_assert("Scenario Outline" == scenarios[2].keyword);
      test_assert("Scenario Template" == scenarios[3].keyword);
      test_assert("Examples" == scenarios[2].examples[0].keyword);
      test_assert("Scenarios" == scenarios[3].examples[0].keyword);
    }

    {  // 4. steps
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    * star first
    Given g
    And a1
    When w
    But b1
    Then t
    * star last
    And   spaced text
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& steps = doc.feature->scenarios[0].steps;
      test_assert(8 == std::size(steps));
      const std::vector<std::pair<std::string, step_type>> expected{
          {"*", step_type::unknown}, {"Given", step_type::given},
          {"And", step_type::given}, {"When", step_type::when},
          {"But", step_type::when},  {"Then", step_type::then},
          {"*", step_type::then},    {"And", step_type::then}};
      for (std::size_t i{}; i < std::size(steps); ++i) {
        test_assert(expected[i].first == steps[i].keyword);
        test_assert(expected[i].second == steps[i].type);
      }
      test_assert("star first" == steps[0].text);
      test_assert("g" == steps[1].text);
      test_assert("spaced text" == steps[7].text);
      test_assert(4 == steps[0].line);
      test_assert(5 == steps[1].line);
    }

    {  // 4. keyword with colon is not a step
      const auto doc =
          gk::parse("Feature: F\nScenario: S\nGiven: x\nGiven y\n");
      test_assert(std::empty(doc.errors));
      const auto& scenario = doc.feature->scenarios[0];
      test_assert("Given: x" == scenario.description);
      test_assert(1 == std::size(scenario.steps));
      test_assert("y" == scenario.steps[0].text);
    }

    {  // 5. descriptions
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
Rule: R
  rule desc
  Background: B
    background desc
    Given a
  Scenario: S
    scenario desc

    more scenario desc
    Given b
  Scenario Outline: O
    outline desc
    Given c <x>
    Examples: E
      examples desc
      | x |
      | 1 |
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& rule = doc.feature->rules[0];
      test_assert("rule desc" == rule.description);
      test_assert("B" == rule.background->name);
      test_assert("background desc" == rule.background->description);
      test_assert(1 == std::size(rule.background->steps));
      test_assert("scenario desc\n\nmore scenario desc" ==
                  rule.scenarios[0].description);
      test_assert(1 == std::size(rule.scenarios[0].steps));
      test_assert("outline desc" == rule.scenarios[1].description);
      test_assert("examples desc" == rule.scenarios[1].examples[0].description);
      test_assert(2 == std::size(rule.scenarios[1].examples[0].data));
    }

    {  // 6. Background
      // clang-format off
      auto doc = gk::parse(R"gk(
Feature: F
  Background: B
    Given a
  Scenario: S
    Given b
)gk");
      test_assert(std::empty(doc.errors));
      test_assert(doc.feature->background.has_value());
      test_assert(3 == doc.feature->background->line);
      test_assert("B" == doc.feature->background->name);
      test_assert(1 == std::size(doc.feature->background->steps));

      doc = gk::parse(R"gk(
Feature: F
  Background:
    Given a
  Background:
    Given b
)gk");
      test_assert(1 == std::size(doc.errors));
      test_assert(5 == doc.errors[0].line);
      test_assert(1 == std::size(doc.feature->background->steps));
      test_assert("a" == doc.feature->background->steps[0].text);

      doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a
  Background:
    Given b
)gk");
      test_assert(1 == std::size(doc.errors));
      test_assert(5 == doc.errors[0].line);
      test_assert(not doc.feature->background);
      test_assert(1 == std::size(doc.feature->scenarios[0].steps));

      doc = gk::parse(R"gk(
Feature: F
  Background:
    Given f
  Rule: R
    Background:
      Given r
    Scenario: S
      Given s
)gk");
      test_assert(std::empty(doc.errors));
      test_assert("f" == doc.feature->background->steps[0].text);
      test_assert("r" == doc.feature->rules[0].background->steps[0].text);

      doc = gk::parse(R"gk(
Feature: F
  Rule: R
    Background:
      Given r1
    Background:
      Given r2
)gk");
      test_assert(1 == std::size(doc.errors));
      test_assert(6 == doc.errors[0].line);

      doc = gk::parse(R"gk(
Feature: F
  Rule: R
    Scenario: S
    Background:
      Given r
)gk");
      test_assert(1 == std::size(doc.errors));
      test_assert(5 == doc.errors[0].line);
      // clang-format on
    }

    {  // 7. Rules
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
  Scenario: S0
    Given a
  @r1 @r2
  Rule: R1
    Scenario: S1
  Rule: R2
    Scenario: S2
    Scenario: S3
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& feature = *doc.feature;
      test_assert(1 == std::size(feature.scenarios));
      test_assert("S0" == feature.scenarios[0].name);
      test_assert(2 == std::size(feature.rules));
      test_assert("R1" == feature.rules[0].name);
      test_assert((std::vector<std::string>{"r1", "r2"}) ==
                  feature.rules[0].tags);
      test_assert(1 == std::size(feature.rules[0].scenarios));
      test_assert("R2" == feature.rules[1].name);
      test_assert(std::empty(feature.rules[1].tags));
      test_assert(2 == std::size(feature.rules[1].scenarios));
      test_assert("S3" == feature.rules[1].scenarios[1].name);
    }

    {  // 8. Scenario Outline and Examples
      // clang-format off
      auto doc = gk::parse(R"gk(
Feature: F
  Scenario Outline: O
    Given <a> and <b>
    @t1
    Examples: first
      | a | b |
      | 1 | 2 |
      | 3 | 4 |
    @t2 @t3
    Examples: second
      | a | b |
      | 5 | 6 |
)gk");
      test_assert(std::empty(doc.errors));
      const auto& outline = doc.feature->scenarios[0];
      test_assert(2 == std::size(outline.examples));
      test_assert("first" == outline.examples[0].name);
      test_assert((std::vector<std::string>{"t1"}) == outline.examples[0].tags);
      test_assert(3 == std::size(outline.examples[0].data));
      test_assert((std::vector<std::string>{"a", "b"}) ==
                  outline.examples[0].data[0]);
      test_assert((std::vector<std::string>{"3", "4"}) ==
                  outline.examples[0].data[2]);
      test_assert("second" == outline.examples[1].name);
      test_assert((std::vector<std::string>{"t2", "t3"}) ==
                  outline.examples[1].tags);
      test_assert(2 == std::size(outline.examples[1].data));

      doc = gk::parse(R"gk(
Feature: F
  Scenario Outline: O
    Given <a>
)gk");
      test_assert(1 == std::size(doc.errors));
      test_assert(3 == doc.errors[0].line);
      test_assert("Scenario Outline must contain one or more Examples" ==
                  doc.errors[0].message);

      doc = gk::parse(R"gk(
Feature: F
  Scenario Outline: O
    Given <a>
  Scenario: S
    Given b
)gk");
      test_assert(1 == std::size(doc.errors));
      test_assert(3 == doc.errors[0].line);
      // clang-format on
    }

    {  // 9. Doc Strings
      // clang-format off
      auto doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a doc
      """json
      {
        "a": 1,

        "b": \"\"\"
      }
      """
    Then done
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& steps = doc.feature->scenarios[0].steps;
      test_assert(2 == std::size(steps));
      test_assert(steps[0].doc.has_value());
      test_assert("json" == steps[0].doc->content_type);
      test_assert("{\n  \"a\": 1,\n\n  \"b\": \"\"\"\n}" ==
                  steps[0].doc->content);
      test_assert(not steps[1].doc);

      // clang-format off
      doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a doc
      ```
      first
      """
      \`\`\`
      ```
    Given another
      """
      ```
      """
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& s = doc.feature->scenarios[0].steps;
      test_assert(2 == std::size(s));
      test_assert("first\n\"\"\"\n```" == s[0].doc->content);
      test_assert(std::empty(s[0].doc->content_type));
      test_assert("```" == s[1].doc->content);

      // clang-format off
      doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a doc
      """
      Scenario: not a scenario
      Given not a step
      # not a comment
      | not | table |
      @not_a_tag
    Then never closed
)gk");
      // clang-format on
      test_assert(1 == std::size(doc.errors));
      test_assert(5 == doc.errors[0].line);
      test_assert("unterminated Doc String" == doc.errors[0].message);

      // clang-format off
      doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a doc
      """
      Scenario: not a scenario
      Given not a step
      # not a comment
      | not | table |
      @not_a_tag
  Scenario: Real
)gk");
      // clang-format on
      test_assert(1 == std::size(doc.errors));
      test_assert(1 == std::size(doc.feature->scenarios));

      // clang-format off
      doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a doc
      """
      Scenario: not a scenario
      Given not a step
      # not a comment
      | not | table |
      @not_a_tag
      """
  Scenario: Real
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      test_assert(2 == std::size(doc.feature->scenarios));
      test_assert(
          "Scenario: not a scenario\nGiven not a step\n# not a comment\n"
          "| not | table |\n@not_a_tag" ==
          doc.feature->scenarios[0].steps[0].doc->content);
    }

    {  // 9. Doc String dedent
      const auto doc = gk::parse(
          "Feature: F\nScenario: S\n  Given a doc\n    \"\"\"\n"
          "  less\n    same\n        deeper\n    \"\"\"\n");
      test_assert(std::empty(doc.errors));
      test_assert("less\nsame\n    deeper" ==
                  doc.feature->scenarios[0].steps[0].doc->content);
    }

    {  // 10. Data Tables
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given a table
      |  a  | b |   c  |
      | 1\|2 | x\ny | back\\slash |
      |     | z |      |
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& data = doc.feature->scenarios[0].steps[0].data;
      test_assert(data.has_value());
      test_assert(3 == std::size(*data));
      test_assert((gk::table::row_t{"a", "b", "c"}) == (*data)[0]);
      test_assert((gk::table::row_t{"1|2", "x\ny", "back\\slash"}) ==
                  (*data)[1]);
      test_assert((gk::table::row_t{"", "z", ""}) == (*data)[2]);
      test_assert(not doc.feature->scenarios[0].steps[0].doc);
    }

    {  // 10. Data Table errors
      auto doc = gk::parse(
          "Feature: F\nScenario: S\n  Given t\n    | a | b |\n    | c |\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(5 == doc.errors[0].line);
      test_assert(1 == std::size(*doc.feature->scenarios[0].steps[0].data));

      doc = gk::parse("Feature: F\nScenario: S\n  Given t\n    | a | b\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(4 == doc.errors[0].line);

      doc = gk::parse(
          "Feature: F\nScenario: S\n  Given t\n    | a |\n    \"\"\"\n    x\n"
          "    \"\"\"\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(5 == doc.errors[0].line);

      doc = gk::parse(
          "Feature: F\nScenario: S\n  Given t\n    \"\"\"\n    x\n    \"\"\"\n"
          "    | a |\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(7 == doc.errors[0].line);
    }

    {  // 11. Tags
      // clang-format off
      const auto doc = gk::parse(R"gk(
@f1 @f2
@f3 # comment
Feature: F
  @r
  Rule: R
    @s1
    @s2 @s3 # trailing @comment
    Scenario: S
      Given a
    @o
    Scenario Outline: O
      Given <x>
      @e
      Examples:
        | x |
        | 1 |
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& feature = *doc.feature;
      test_assert((std::vector<std::string>{"f1", "f2", "f3"}) == feature.tags);
      const auto& rule = feature.rules[0];
      test_assert((std::vector<std::string>{"r"}) == rule.tags);
      test_assert((std::vector<std::string>{"s1", "s2", "s3"}) ==
                  rule.scenarios[0].tags);
      test_assert((std::vector<std::string>{"o"}) == rule.scenarios[1].tags);
      test_assert((std::vector<std::string>{"e"}) ==
                  rule.scenarios[1].examples[0].tags);
    }

    {  // 11. Tag errors
      auto doc = gk::parse("Feature: F\n@t\nBackground:\n  Given a\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(2 == doc.errors[0].line);

      doc = gk::parse("Feature: F\nScenario: S\n@t\n  Given a\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(3 == doc.errors[0].line);
      test_assert(1 == std::size(doc.feature->scenarios[0].steps));

      doc = gk::parse("Feature: F\n@a bogus @b\nScenario: S\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(2 == doc.errors[0].line);
      test_assert(doc.errors[0].message.find("bogus") != std::string::npos);
      test_assert((std::vector<std::string>{"a", "b"}) ==
                  doc.feature->scenarios[0].tags);

      doc = gk::parse("Feature: F\nScenario: S\n@dangling\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(3 == doc.errors[0].line);
    }

    {  // 12. Comments
      // clang-format off
      const auto doc = gk::parse(R"gk(
# language: en
Feature: F
# comment
  Scenario: S
  # comment
    Given a
    # comment
      | a | b |
      # comment
      | 1 | 2 |
        # comment
    Then b
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto& steps = doc.feature->scenarios[0].steps;
      test_assert(2 == std::size(steps));
      test_assert(2 == std::size(*steps[0].data));
      test_assert(std::empty(doc.feature->scenarios[0].description));
    }

    {  // 13. Whitespace
      auto doc = gk::parse(
          "\tFeature: F\n\tScenario: S\n\t\tGiven a\n"
          "\t\t\t| a |\n\t\t\t| b |\n");
      test_assert(std::empty(doc.errors));
      test_assert("F" == doc.feature->name);
      test_assert(1 == std::size(doc.feature->scenarios[0].steps));
      test_assert(2 == std::size(*doc.feature->scenarios[0].steps[0].data));

      doc = gk::parse(
          "Feature: F\r\nScenario: S\r\nGiven a\r\n| a |\r\n| 1 |\r\n"
          "Given b\r\n\"\"\"\r\nx\r\n  y\r\n\"\"\"\r\n");
      test_assert(std::empty(doc.errors));
      test_assert("F" == doc.feature->name);
      test_assert("S" == doc.feature->scenarios[0].name);
      const auto& steps = doc.feature->scenarios[0].steps;
      test_assert(2 == std::size(steps));
      test_assert("a" == steps[0].text);
      test_assert((gk::table::row_t{"1"}) == (*steps[0].data)[1]);
      test_assert("x\n  y" == steps[1].doc->content);
    }

    // ---- compiler ----

    {  // 14. Background order
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
  Background:
    Given fb
  Scenario: Outside
    When s0
  Rule: R
    Background:
      Given rb
    Scenario: Inside
      When s1
    Scenario: Empty
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto pickles = gk::compile(*doc.feature);
      test_assert(3 == std::size(pickles));
      test_assert("Outside" == pickles[0].name);
      test_assert(2 == std::size(pickles[0].steps));
      test_assert("fb" == pickles[0].steps[0].text);
      test_assert("s0" == pickles[0].steps[1].text);
      test_assert("Inside" == pickles[1].name);
      test_assert(3 == std::size(pickles[1].steps));
      test_assert("fb" == pickles[1].steps[0].text);
      test_assert("rb" == pickles[1].steps[1].text);
      test_assert("s1" == pickles[1].steps[2].text);
      test_assert("Empty" == pickles[2].name);
      test_assert(std::empty(pickles[2].steps));
      test_assert("F" == pickles[2].feature);
    }

    {  // 15. Outline expansion
      // clang-format off
      const auto doc = gk::parse(R"gk(
Feature: F
  Scenario Outline: Add <a> and <b>
    Given <a> plus <b> <unknown>
    And a doc
      """
      doc <a>
      """
    And a table
      | <a> | <b> |
      | x   | <a> |
    Examples:
      | a   | b |
      | 1   | 2 |
      | <b> | 4 |
    Examples: other
      | a | b |
      | 5 | 6 |
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto pickles = gk::compile(*doc.feature);
      test_assert(3 == std::size(pickles));
      test_assert("Add 1 and 2" == pickles[0].name);
      test_assert("1 plus 2 <unknown>" == pickles[0].steps[0].text);
      test_assert("doc 1" == pickles[0].steps[1].doc->content);
      test_assert((gk::table::row_t{"1", "2"}) ==
                  (*pickles[0].steps[2].data)[0]);
      test_assert((gk::table::row_t{"x", "1"}) ==
                  (*pickles[0].steps[2].data)[1]);
      test_assert("Add <b> and 4" == pickles[1].name);
      test_assert("<b> plus 4 <unknown>" == pickles[1].steps[0].text);
      test_assert("doc <b>" == pickles[1].steps[1].doc->content);
      test_assert("Add 5 and 6" == pickles[2].name);
      test_assert(step_type::given == pickles[2].steps[1].type);
      test_assert(3 == pickles[2].line);
    }

    {  // 16. Tag inheritance
      // clang-format off
      const auto doc = gk::parse(R"gk(
@f
Feature: F
  @plain
  Scenario: Outside
    Given a
  @r
  Rule: R
    @s
    Scenario Outline: O
      Given <x>
      @e1
      Examples:
        | x |
        | 1 |
      @e2
      Examples:
        | x |
        | 2 |
)gk");
      // clang-format on
      test_assert(std::empty(doc.errors));
      const auto pickles = gk::compile(*doc.feature);
      test_assert(3 == std::size(pickles));
      test_assert(not pickles[0].rule);
      test_assert((std::vector<std::string>{"f", "plain"}) == pickles[0].tags);
      test_assert(pickles[1].rule.has_value());
      test_assert("R" == *pickles[1].rule);
      test_assert((std::vector<std::string>{"f", "r", "s", "e1"}) ==
                  pickles[1].tags);
      test_assert((std::vector<std::string>{"f", "r", "s", "e2"}) ==
                  pickles[2].tags);
    }

    // ---- execution ----

    static std::vector<std::string> trace{};
    const auto logged = [](const std::string_view text) {
      auto& cfg = ut::cfg<ut::override>;
      for (const auto& log : cfg.log_calls) {
        if (const auto* msg = std::any_cast<std::string>(&log)) {
          if (msg->find(text) != std::string::npos) {
            return true;
          }
        }
      }
      return false;
    };

    {  // 17. scenarios are nested tests, @skip
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.feature("*") = [&] {
          s.scenario("*") = [&] {
            s.given("step") = [&] { trace.push_back("step"); };
          };
        };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Scenario: One
    Given step
  @skip
  Scenario: Two
    Given step
  Scenario: Three
    Given step
)gk";
      // clang-format on

      test_assert(3 == std::size(test_cfg.run_calls));
      test_assert("outer" == test_cfg.run_calls[0].name);
      test_assert("scenario" == test_cfg.run_calls[1].type);
      test_assert("One" == test_cfg.run_calls[1].name);
      test_assert("scenario" == test_cfg.run_calls[2].type);
      test_assert("Three" == test_cfg.run_calls[2].name);
      test_assert(1 == std::size(test_cfg.skip_calls));
      test_assert("scenario" == test_cfg.skip_calls[0].type);
      test_assert("Two" == test_cfg.skip_calls[0].name);
      test_assert(2 == std::size(trace));
      test_assert(std::empty(test_cfg.assertion_calls));

      test_cfg = fake_cfg{};
      trace.clear();
      // clang-format off
      "outer"_test = defs | R"gk(
@skip
Feature: F
  Scenario: One
    Given step
  Scenario: Two
    Given step
)gk";
      // clang-format on
      test_assert(1 == std::size(test_cfg.run_calls));
      test_assert(2 == std::size(test_cfg.skip_calls));
      test_assert(std::empty(trace));
    }

    {  // 18. Background and fresh state
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.feature("*") = [&] {
          s.scenario("*") = [&] {
            s.given("a counter") = [&] {
              int count{};
              trace.push_back("background");
              s.when("increment") = [&] { ++count; };
              s.then("count is {n}") = [&](int n) {
                trace.push_back("check");
                expect(that % count == n);
              };
            };
          };
        };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Background:
    Given a counter
  Scenario: One
    When increment
    Then count is 1
  Scenario: Two
    Then count is 0
  Scenario: Three
    When increment
    And increment
    Then count is 2
)gk";
      // clang-format on

      test_assert(4 == std::size(test_cfg.run_calls));
      test_assert(3 == std::size(test_cfg.assertion_calls));
      for (const auto& call : test_cfg.assertion_calls) {
        test_assert(call.result);
      }
      test_assert((std::vector<std::string>{"background", "check", "background",
                                            "check", "background", "check"}) ==
                  trace);
    }

    {  // 19. And/But/* and keyword matching
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.given("g1") = [&] { trace.push_back("g1"); };
        s.given("g2") = [&] { trace.push_back("g2"); };
        s.when("w1") = [&] { trace.push_back("w1"); };
        s.when("w2") = [&] { trace.push_back("w2"); };
        s.then("t1") = [&] { trace.push_back("t1"); };
        s.then("t2") = [&] { trace.push_back("t2"); };
        s.then("t3") = [&] { trace.push_back("t3"); };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given g1
    And g2
    When w1
    But w2
    Then t1
    And t2
    * t3
)gk";
      // clang-format on
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"g1", "g2", "w1", "w2", "t1", "t2",
                                            "t3"}) == trace);
    }

    {  // 19. step() matches any keyword
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.step("any {value}") = [&](std::string value) {
          trace.push_back(value);
        };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    * any 0
    Given any 1
    When any 2
    Then any 3
    And any 4
)gk";
      // clang-format on
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"0", "1", "2", "3", "4"}) == trace);
    }

    {  // 19. same text, keyword type decides
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.then("same") = [&] { trace.push_back("then"); };
        s.given("same") = [&] { trace.push_back("given"); };
        s.given("barrier") = [&] { trace.push_back("barrier"); };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given barrier
    And same
    Then same
    And same
)gk";
      // clang-format on
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"barrier", "given", "then",
                                            "then"}) == trace);
    }

    {  // 20. table and doc string arguments, quoted captures
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.given("a table") = [&](const bdd::gherkin::table& data) {
          trace.push_back("table " + std::to_string(std::size(data)) + " " +
                          data[1][1]);
        };
        s.given("{n} rows") = [&](int n, const bdd::gherkin::table& data) {
          trace.push_back("rows " + std::to_string(n) + " " +
                          std::to_string(std::size(data)));
        };
        s.given("a doc") = [&](const bdd::gherkin::doc_string& doc) {
          trace.push_back("doc " + doc.content_type + " " + doc.content);
        };
        s.given("{n} lines in {name}") =
            [&](int n, std::string name, const bdd::gherkin::doc_string& doc) {
              trace.push_back("lines " + std::to_string(n) + " " + name + " " +
                              doc.content);
            };
        s.when("named \"{name}\"") = [&](std::string name) {
          trace.push_back("dq " + name);
        };
        s.when("named '{name}'") = [&](std::string name) {
          trace.push_back("sq " + name);
        };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given a table
      | a | b |
      | c | d |
    And 3 rows
      | x |
      | y |
      | z |
    And a doc
      """txt
      hello
      """
    And 2 lines in text
      ```
      l1
      l2
      ```
    When named "Greg Smith"
    And named 'Jane Q Public'
)gk";
      // clang-format on
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert(
          (std::vector<std::string>{"table 2 d", "rows 3 3", "doc txt hello",
                                    "lines 2 text l1\nl2", "dq Greg Smith",
                                    "sq Jane Q Public"}) == trace);
    }

    {  // 21. Outline run
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps defs = [](auto& s) {
        s.feature("*") = [&] {
          s.scenario("*") = [&] {
            s.given("{a} and {b}") = [&](int a, int b) {
              trace.push_back(std::to_string(a) + "+" + std::to_string(b));
            };
          };
        };
      };

      // clang-format off
      "outer"_test = defs | R"gk(
Feature: F
  Scenario Outline: Sum <a> <b>
    Given <a> and <b>
    Examples:
      | a | b |
      | 1 | 2 |
      | 3 | 4 |
    Examples:
      | a | b |
      | 5 | 6 |
)gk";
      // clang-format on
      test_assert(4 == std::size(test_cfg.run_calls));
      test_assert("Sum 1 2" == test_cfg.run_calls[1].name);
      test_assert("Sum 3 4" == test_cfg.run_calls[2].name);
      test_assert("Sum 5 6" == test_cfg.run_calls[3].name);
      test_assert((std::vector<std::string>{"1+2", "3+4", "5+6"}) == trace);
    }

    {  // 22. failures
      auto& test_cfg = ut::cfg<ut::override>;
      const auto failed_once = [&](const std::string_view message) {
        test_assert(1 == std::size(test_cfg.assertion_calls));
        test_assert(not test_cfg.assertion_calls[0].result);
        test_assert(logged(message));
      };

      {  // undefined step
        test_cfg = fake_cfg{};
        bdd::gherkin::steps defs = [](auto& s) { s.given("a") = [&] {}; };
        // clang-format off
        "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given a
    When nope
)gk";
        // clang-format on
        failed_once("undefined step");
      }

      {  // scenario without a definition
        test_cfg = fake_cfg{};
        bdd::gherkin::steps defs = [](auto& s) {
          s.feature("*") = [&] {
            s.scenario("Other") = [&] { s.given("a") = [&] {}; };
          };
        };
        // clang-format off
        "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given a
)gk";
        // clang-format on
        failed_once("no definition matches");
      }

      {  // table not accepted
        test_cfg = fake_cfg{};
        bdd::gherkin::steps defs = [](auto& s) { s.given("a") = [&] {}; };
        // clang-format off
        "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given a
      | x |
)gk";
        // clang-format on
        failed_once("Data Table");
      }

      {  // table expected
        test_cfg = fake_cfg{};
        bdd::gherkin::steps defs = [](auto& s) {
          s.given("a") = [&](const bdd::gherkin::table&) {};
        };
        // clang-format off
        "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given a
)gk";
        // clang-format on
        failed_once("Data Table");
      }

      {  // doc string expected / not accepted
        test_cfg = fake_cfg{};
        bdd::gherkin::steps defs = [](auto& s) {
          s.given("a") = [&](const bdd::gherkin::doc_string&) {};
        };
        // clang-format off
        "outer"_test = defs | R"gk(
Feature: F
  Scenario: S
    Given a
)gk";
        // clang-format on
        failed_once("Doc String");

        test_cfg = fake_cfg{};
        bdd::gherkin::steps plain = [](auto& s) { s.given("a") = [&] {}; };
        // clang-format off
        "outer"_test = plain | R"gk(
Feature: F
  Scenario: S
    Given a
      """
      x
      """
)gk";
        // clang-format on
        failed_once("Doc String");
      }

      {  // syntax errors
        test_cfg = fake_cfg{};
        bdd::gherkin::steps defs = [](auto& s) { s.given("a") = [&] {}; };
        // clang-format off
        "outer"_test = defs | R"gk(
bogus
Feature: F
  Scenario: S
    Given a
      | a |
      | b | c |
)gk";
        // clang-format on
        test_assert(1 == std::size(test_cfg.run_calls));
        test_assert(2 == std::size(test_cfg.assertion_calls));
        test_assert(not test_cfg.assertion_calls[0].result);
        test_assert(not test_cfg.assertion_calls[1].result);
        test_assert(logged("expected 'Feature:'"));
        test_assert(logged("inconsistent"));
      }
    }

    {  // 23. rule nesting
      auto& test_cfg = ut::cfg<ut::override>;
      test_cfg = fake_cfg{};
      trace.clear();

      bdd::gherkin::steps with_rule = [](auto& s) {
        s.feature("*") = [&] {
          trace.push_back("feature");
          s.rule("R*") = [&] {
            trace.push_back("rule");
            s.scenario("*") = [&] {
              trace.push_back("scenario");
              s.given("a") = [&] { trace.push_back("a"); };
            };
          };
        };
      };

      // clang-format off
      const auto text = R"gk(
Feature: F
  Rule: Rx
    Scenario: S
      Given a
)gk";
      // clang-format on
      "outer"_test = with_rule | text;
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"feature", "rule", "scenario",
                                            "a"}) == trace);

      test_cfg = fake_cfg{};
      trace.clear();
      bdd::gherkin::steps no_rule = [](auto& s) {
        s.feature("*") = [&] {
          s.scenario("*") = [&] {
            s.given("a") = [&] { trace.push_back("a"); };
          };
        };
      };
      "outer"_test = no_rule | text;
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert(2 == std::size(test_cfg.run_calls));
      test_assert((std::vector<std::string>{"a"}) == trace);

      test_cfg = fake_cfg{};
      trace.clear();
      bdd::gherkin::steps flat = [](auto& s) {
        s.given("a") = [&] { trace.push_back("a"); };
      };
      "outer"_test = flat | text;
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"a"}) == trace);

      test_cfg = fake_cfg{};
      trace.clear();
      "outer"_test = with_rule | R"gk(
Feature: F
  Rule: Other
    Scenario: S
      Given a
)gk";
      test_assert(1 == std::size(test_cfg.assertion_calls));
      test_assert(not test_cfg.assertion_calls[0].result);
    }

    // ---- additional parser / execution checks ----

    {  // BOM is skipped
      const auto doc = gk::parse(std::string{"\xEF\xBB\xBF"} +
                                 "Feature: F\n  Scenario: S\n    Given x\n");
      test_assert(std::empty(doc.errors));
      test_assert(doc.feature.has_value());
      test_assert("F" == doc.feature->name);
    }

    {  // steps are only allowed inside scenarios
      // clang-format off
      const auto in_examples = gk::parse(R"gk(
Feature: F
  Scenario Outline: O
    Given x <n>
    Examples:
      Given bad
      | n |
      | 1 |
)gk");
      const auto in_feature = gk::parse(R"gk(
Feature: F
  Given bad
  Scenario: S
    Given x
)gk");
      const auto in_rule = gk::parse(R"gk(
Feature: F
  Rule: R
    Given bad
    Scenario: S
      Given x
)gk");
      // clang-format on
      const auto has_error = [](const gk::ast::document& doc, int line) {
        for (const auto& error : doc.errors) {
          if (static_cast<int>(error.line) == line and
              error.message.find("steps are only allowed") !=
                  std::string::npos) {
            return true;
          }
        }
        return false;
      };
      test_assert(has_error(in_examples, 6));
      test_assert(in_examples.feature.has_value());
      test_assert(std::empty(
          in_examples.feature->scenarios[0].examples[0].description));
      test_assert(has_error(in_feature, 3));
      test_assert(has_error(in_rule, 4));
    }

    {  // doc string is closed only by a line equal to the delimiter
      // clang-format off
      const auto quotes = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given x
      """
      """" quoted
      """json
      """
)gk");
      const auto ticks = gk::parse(R"gk(
Feature: F
  Scenario: S
    Given x
      ```
      ```cpp
      ```
)gk");
      // clang-format on
      test_assert(std::empty(quotes.errors));
      test_assert(quotes.feature.has_value());
      test_assert("\"\"\"\" quoted\n\"\"\"json" ==
                  quotes.feature->scenarios[0].steps[0].doc->content);
      test_assert(std::empty(ticks.errors));
      test_assert(ticks.feature.has_value());
      test_assert("```cpp" ==
                  ticks.feature->scenarios[0].steps[0].doc->content);
    }

    {  // empty document at parser level
      const auto doc = gk::parse("");
      test_assert(not doc.feature.has_value());
      test_assert(std::empty(doc.errors));
    }

    {  // no Feature: at all
      auto& test_cfg = ut::cfg<ut::override>;
      bdd::gherkin::steps defs = [](auto& s) { s.given("a") = [&] {}; };
      for (const auto* text : {"", "\n# only a comment\n\n   \n"}) {
        test_cfg = fake_cfg{};
        "outer"_test = defs | text;
        test_assert(1 == std::size(test_cfg.assertion_calls));
        test_assert(not test_cfg.assertion_calls[0].result);
        test_assert(logged("expected 'Feature:'"));
        for (const auto& call : test_cfg.run_calls) {
          test_assert("scenario" != call.type);
        }
      }
    }

    {  // numbered names for scenarios sharing a name
      auto& test_cfg = ut::cfg<ut::override>;
      const auto scenario_names = [&] {
        std::vector<std::string> names;
        for (const auto& call : test_cfg.run_calls) {
          if (call.type == "scenario") {
            names.emplace_back(call.name);
          }
        }
        return names;
      };

      test_cfg = fake_cfg{};
      bdd::gherkin::steps wild = [](auto& s) {
        s.scenario("*") = [&] { s.given("a") = [&] {}; };
      };
      // clang-format off
      "outer"_test = wild | R"gk(
Feature: F
  Scenario Outline: same
    Given a
    Examples:
      | n |
      | 1 |
      | 2 |
      | 3 |
)gk";
      // clang-format on
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"same (1)", "same (2)",
                                            "same (3)"}) == scenario_names());

      test_cfg = fake_cfg{};
      // clang-format off
      "outer"_test = wild | R"gk(
Feature: F
  Scenario Outline: eating <n>
    Given a
    Examples:
      | n |
      | 1 |
      | 2 |
)gk";
      // clang-format on
      test_assert((std::vector<std::string>{"eating 1", "eating 2"}) ==
                  scenario_names());

      test_cfg = fake_cfg{};
      // clang-format off
      "outer"_test = wild | R"gk(
Feature: F
  Rule: R1
    Scenario: dup
      Given a
  Rule: R2
    Scenario: dup
      Given a
)gk";
      // clang-format on
      test_assert((std::vector<std::string>{"dup (1)", "dup (2)"}) ==
                  scenario_names());

      test_cfg = fake_cfg{};
      trace.clear();
      bdd::gherkin::steps exact = [](auto& s) {
        s.scenario("same") = [&] {
          s.given("a") = [&] { trace.push_back("a"); };
        };
      };
      // clang-format off
      "outer"_test = exact | R"gk(
Feature: F
  Scenario Outline: same
    Given a
    Examples:
      | n |
      | 1 |
      | 2 |
      | 3 |
)gk";
      // clang-format on
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert(3 == std::size(trace));
    }

    {  // every matching definition is entered
      auto& test_cfg = ut::cfg<ut::override>;
      // clang-format off
      const auto text = R"gk(
Feature: F
  Scenario: S
    Given a
    When b
)gk";
      // clang-format on

      test_cfg = fake_cfg{};
      trace.clear();
      bdd::gherkin::steps two_features = [](auto& s) {
        s.feature("*") = [&] { s.given("a") = [&] { trace.push_back("a"); }; };
        s.feature("*") = [&] { s.when("b") = [&] { trace.push_back("b"); }; };
      };
      "outer"_test = two_features | text;
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"a", "b"}) == trace);

      test_cfg = fake_cfg{};
      trace.clear();
      bdd::gherkin::steps two_scenarios = [](auto& s) {
        s.scenario("*") = [&] { s.given("a") = [&] { trace.push_back("a"); }; };
        s.scenario("*") = [&] { s.when("b") = [&] { trace.push_back("b"); }; };
      };
      "outer"_test = two_scenarios | text;
      test_assert(std::empty(test_cfg.assertion_calls));
      test_assert((std::vector<std::string>{"a", "b"}) == trace);
    }

    // ---- parser error paths and description fallbacks ----

    const auto reported = [](const gk::ast::document& doc,
                             const std::size_t line,
                             const std::string_view message) {
      for (const auto& error : doc.errors) {
        if (error.line == line and
            error.message.find(message) != std::string::npos) {
          return true;
        }
      }
      return false;
    };

    {  // Examples without a Scenario
      const auto doc =
          gk::parse("Feature: F\n@t\nExamples:\n| a |\nScenario: S\nGiven x\n");
      test_assert(1 == std::size(doc.errors));
      test_assert(reported(doc, 3, "Examples must follow the steps"));
      test_assert(doc.feature.has_value());
      test_assert(1 == std::size(doc.feature->scenarios));
      test_assert(std::empty(doc.feature->scenarios[0].tags));
      test_assert(std::empty(doc.feature->scenarios[0].examples));
    }

    {  // table row and Doc String which have nothing to belong to
      const auto doc = gk::parse(
          "Feature: F\nBackground:\nGiven x\nBackground:\n| a |\n"
          "\"\"\"\nignored\n\"\"\"\nScenario: S\nGiven y\n");
      test_assert(3 == std::size(doc.errors));
      test_assert(reported(doc, 4, "only one Background"));
      test_assert(reported(doc, 5, "table row must follow a step or Examples"));
      test_assert(reported(doc, 6, "Doc String must follow a step"));
      test_assert(doc.feature.has_value());
      test_assert(1 == std::size(doc.feature->background->steps));
      test_assert(not doc.feature->background->steps[0].data);
      test_assert(not doc.feature->background->steps[0].doc);
      test_assert(1 == std::size(doc.feature->scenarios));
      test_assert("y" == doc.feature->scenarios[0].steps[0].text);
    }

    {  // text after a step or an Examples table
      const auto after_step =
          gk::parse("Feature: F\nScenario: S\nGiven x\nstray text\nThen y\n");
      test_assert(1 == std::size(after_step.errors));
      test_assert(reported(after_step, 4, "unexpected 'stray text'"));
      test_assert(2 == std::size(after_step.feature->scenarios[0].steps));

      const auto after_table = gk::parse(
          "Feature: F\nScenario Outline: O\nGiven <a>\nExamples:\n"
          "| a |\n| 1 |\nstray text\n");
      test_assert(1 == std::size(after_table.errors));
      test_assert(reported(after_table, 7, "unexpected 'stray text'"));
      test_assert(
          2 == std::size(after_table.feature->scenarios[0].examples[0].data));
    }

    {  // table row and Doc String delimiter are part of a description
      const auto row =
          gk::parse("Feature: F\n| not a table |\nScenario: S\nGiven x\n");
      test_assert(std::empty(row.errors));
      test_assert("| not a table |" == row.feature->description);
      test_assert(1 == std::size(row.feature->scenarios));

      const auto quotes =
          gk::parse("Feature: F\n  \"\"\" quoted\nScenario: S\nGiven x\n");
      test_assert(std::empty(quotes.errors));
      test_assert("\"\"\" quoted" == quotes.feature->description);
      test_assert(1 == std::size(quotes.feature->scenarios));
      test_assert(not quotes.feature->scenarios[0].steps[0].doc);
    }

    {  // table accessors
      test_assert(gk::table{}.empty());
      test_assert(gk::table{}.begin() == gk::table{}.end());

      const auto doc =
          gk::parse("Feature: F\nScenario: S\nGiven x\n| a | b |\n| c | d |\n");
      test_assert(std::empty(doc.errors));
      const auto& data = *doc.feature->scenarios[0].steps[0].data;
      test_assert(not data.empty());
      std::string cells{};
      for (const auto& row : data) {
        for (const auto& cell : row) {
          cells += cell;
        }
      }
      test_assert("abcd" == cells);
      test_assert(2 == data.end() - data.begin());
    }
  }
#endif

  {
    using namespace ut;

    test_assert(std::empty(bdd::gherkin::feature_file()));

    const auto argc = detail::cfg::largc;
    const auto argv = detail::cfg::largv;
    const char* args[] = {"ut", "--feature", "calculator.feature"};
    detail::cfg::parse(3, args);
    test_assert("calculator.feature" == bdd::gherkin::feature_file());
    test_assert(std::empty(detail::cfg::query_pattern));

    detail::cfg::feature_filename.clear();
    detail::cfg::parse(argc, argv);
    detail::cfg::largc = argc;
    detail::cfg::largv = argv;
    test_assert(std::empty(bdd::gherkin::feature_file()));
  }
}
