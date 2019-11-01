//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <ciso646>  // and, or, not

#if not defined(__cpp_variadic_templates) or                                  \
    not defined(__cpp_rvalue_references) or not defined(__cpp_decltype) or    \
    not defined(__cpp_alias_templates) or not defined(__cpp_static_assert) or \
    not defined(__cpp_generic_lambdas) or not defined(__cpp_constexpr) or     \
    not defined(__cpp_return_type_deduction) or                               \
    not defined(__cpp_fold_expressions) or not defined(__cpp_deduction_guides)
#error "[Boost].UT requires C++20 support"
#else
#if __has_include(<experimental/source_location>)
#include <experimental/source_location>
#else
namespace std::experimental {
struct source_location {
  static constexpr auto current() noexcept { return source_location{}; }
  constexpr auto file_name() const noexcept { return __FILE__; }
  constexpr auto line() const noexcept { return __LINE__; }
};
}  // namespace std::experimental
#endif
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace boost::ut {
inline namespace v1_0_0 {
namespace reflection {
template <class T>
constexpr auto type_name() -> std::string_view {
#if defined(_MSC_VER) and not defined(__clang__)
  return {&__FUNCSIG__[115], sizeof(__FUNCSIG__) - 123};
#elif defined(__clang__)
  return {&__PRETTY_FUNCTION__[65], sizeof(__PRETTY_FUNCTION__) - 67};
#elif defined(__GNUC__)
  return {&__PRETTY_FUNCTION__[80], sizeof(__PRETTY_FUNCTION__) - 131};
#endif
}
}  // namespace reflection

namespace math {
template <class T>
constexpr auto abs(const T t) -> T {
  return t < T{} ? -t : t;
}

template <class T, class TExp>
constexpr auto pow(const T base, const TExp exp) -> T {
  return exp ? T(base * pow(base, exp - TExp(1))) : T(1);
}

template <class T, char... Cs>
constexpr auto num() -> T {
  constexpr const char cs[]{Cs...};
  T result{};
  auto size = 0u, i = 0u, tmp = 1u;

  while (i < sizeof...(Cs) and cs[i] != '.') {
    if (cs[i++] != '\'') {
      ++size;
    }
  }

  i = {};
  while (i < sizeof...(Cs) and cs[i] != '.') {
    if (cs[i] == '\'') {
      --tmp;
    } else {
      result += pow(T(10), size - i - tmp) * T(cs[i] - '0');
    }
    ++i;
  }
  return result;
}

template <class T, char... Cs>
constexpr auto den() -> T {
  constexpr const char cs[]{Cs...};
  T result{};
  auto i = 0u;
  while (cs[i++] != '.')
    ;
  for (auto j = i; j < sizeof...(Cs); ++j) {
    result += pow(T(10), sizeof...(Cs) - j) * T(cs[j] - '0');
  }
  return result;
}

template <class T, char... Cs>
constexpr auto den_size() -> T {
  constexpr const char cs[]{Cs...};
  T i{};
  while (cs[i++] != '.')
    ;
  return T(sizeof...(Cs)) - i + T(1);
}

template <class T, class TValue>
constexpr auto den_size(TValue value) -> T {
  constexpr auto precision = TValue(1e-7);
  T result{};
  TValue tmp{};
  do {
    value *= 10;
    tmp = value - T(value);
    ++result;
  } while (tmp > precision and result < std::numeric_limits<TValue>::digits10);

  return result;
}

}  // namespace math

namespace type_traits {
template <class T, class...>
struct identity {
  using type = T;
};

template <class... Ts, class TExpr>
constexpr auto is_valid(TExpr expr)
    -> decltype(expr(std::declval<Ts...>()), bool()) {
  return true;
}
template <class...>
constexpr auto is_valid(...) -> bool {
  return false;
}

template <class T>
static constexpr auto is_container_v =
    is_valid<T>([](auto t) -> decltype(t.begin(), t.end(), void()) {});

template <class T>
static constexpr auto has_value_v =
    is_valid<T>([](auto t) -> decltype(t.value, void()) {});

template <class T>
static constexpr auto has_epsilon_v =
    is_valid<T>([](auto t) -> decltype(t.epsilon, void()) {});

template <bool Cond, class T = int>
using requires_t = std::enable_if_t<Cond, T>;
}  // namespace type_traits

namespace utility {
inline auto is_match(std::string_view input, std::string_view pattern) -> bool {
  if (std::empty(pattern)) {
    return std::empty(input);
  }

  if (std::empty(input)) {
    return std::empty(pattern) or pattern[0] == '*'
               ? is_match(input, pattern.substr(1))
               : false;
  }

  if (pattern[0] != '?' and pattern[0] != '*' and pattern[0] != input[0]) {
    return false;
  }

  if (pattern[0] == '*') {
    for (auto i = 0u; i <= std::size(input); ++i) {
      if (is_match(input.substr(i), pattern.substr(1))) {
        return true;
      }
    }
    return false;
  }

  return is_match(input.substr(1), pattern.substr(1));
}

inline auto split(std::string_view input, std::string_view delim)
    -> std::vector<std::string_view> {
  std::vector<std::string_view> output;
  std::size_t first{};
  while (first < std::size(input)) {
    const auto second = input.find_first_of(delim, first);
    if (first != second) {
      output.emplace_back(input.substr(first, second - first));
    }
    if (second == std::string_view::npos) {
      break;
    }
    first = second + 1;
  }
  return output;
}
}  // namespace utility

namespace operators {
template <class TOs, class T,
          type_traits::requires_t<type_traits::is_container_v<T>> = 0>
constexpr auto& operator<<(TOs& os, const T& t) {
  os << '{';
  auto first = true;
  for (const auto& arg : t) {
    os << (first ? "" : ", ") << arg;
    first = false;
  }
  os << '}';
  return os;
}
}  // namespace operators

struct none {};

namespace events {
struct test_begin {
  std::string_view type{};
  std::string_view name{};
};
template <class Test, class TArg = none>
struct test {
  std::string_view type{};
  std::string_view name{};
  TArg arg{};
  Test run{};

  constexpr auto operator()() { run_impl(run, arg); }
  constexpr auto operator()() const { run_impl(run, arg); }

 private:
  static constexpr auto run_impl(Test test, [[maybe_unused]] const TArg& arg)
      -> void {
    if constexpr (std::is_same_v<none, TArg>) {
      test();
    } else if constexpr (std::is_invocable_v<Test, TArg>) {
      test(arg);
    } else {
      test.template operator()<TArg>();
    }
  }
};
template <class Test, class TArg>
test(std::string_view, std::string_view, TArg, Test)->test<Test, TArg>;
template <class TSuite>
struct suite {
  TSuite run{};
  constexpr auto operator()() { run(); }
  constexpr auto operator()() const { run(); }
};
template <class TSuite>
suite(TSuite)->suite<TSuite>;
struct test_run {
  std::string_view type{};
  std::string_view name{};
};
template <class TArg = none>
struct skip {
  std::string_view type{};
  std::string_view name{};
  TArg arg{};
};
template <class TArg>
skip(std::string_view, std::string_view, TArg)->skip<TArg>;
struct test_skip {
  std::string_view type{};
  std::string_view name{};
};
template <class TLocation, class TExpr>
struct assertion {
  TLocation location{};
  TExpr expr{};
};
template <class TLocation, class TExpr>
assertion(TLocation, TExpr)->assertion<TLocation, TExpr>;
template <class TLocation, class TExpr>
struct assertion_pass {
  TLocation location{};
  TExpr expr{};
};
template <class TLocation, class TExpr>
assertion_pass(TLocation, TExpr)->assertion_pass<TLocation, TExpr>;
template <class TLocation, class TExpr>
struct assertion_fail {
  TLocation location{};
  TExpr expr{};
};
template <class TLocation, class TExpr>
assertion_fail(TLocation, TExpr)->assertion_fail<TLocation, TExpr>;
struct test_end {
  std::string_view type{};
  std::string_view name{};
};
struct log {
  std::string_view msg{};
};
struct fatal_assertion {};
struct exception {};
struct summary {};
}  // namespace events

class reporter {
 public:
  auto on(events::test_begin test) -> void {
    out_ << "Running \"" << test.name << "\"...";
    fails_ = asserts_.fail;
    exception_ = false;
  }

  auto on(events::test_run test) -> void {
    out_ << "\n \"" << test.name << "\"...";
  }

  auto on(events::test_skip test) -> void {
    out_ << test.name << "...SKIPPED\n";
    ++tests_.skip;
  }

  auto on(events::test_end) -> void {
    if (asserts_.fail > fails_ or exception_) {
      ++tests_.fail;
      out_ << "\nFAILED\n";
    } else {
      ++tests_.pass;
      out_ << "OK\n";
    }
  }

  auto on(events::log l) -> void { out_ << l.msg; }

  auto on(events::exception) -> void {
    exception_ = true;
    out_ << "\n  Unexpected exception!";
  }

  template <class TLocation, class TExpr>
  auto on(events::assertion_pass<TLocation, TExpr>) -> void {
    ++asserts_.pass;
  }

  template <class TLocation, class TExpr>
  auto on(events::assertion_fail<TLocation, TExpr> assertion) -> void {
    constexpr auto short_name = [](std::string_view name) {
      return name.rfind('/') != std::string_view::npos
                 ? name.substr(name.rfind('/') + 1)
                 : name;
    };
    out_ << "\n  " << short_name(assertion.location.file_name()) << ':'
         << assertion.location.line() << ":FAILED [" << std::boolalpha
         << assertion.expr << ']';
    ++asserts_.fail;
  }

  auto on(events::fatal_assertion) -> void { ++asserts_.fail; }

  auto on(events::summary) -> void {
    if (static auto once = true; once) {
      once = false;
      if (tests_.fail or asserts_.fail) {
        out_ << "\n============================================================"
                "=="
                "=================\n"
             << "tests:   " << (tests_.pass + tests_.fail) << " | "
             << tests_.fail << " failed" << '\n'
             << "asserts: " << (asserts_.pass + asserts_.fail) << " | "
             << asserts_.pass << " passed"
             << " | " << asserts_.fail << " failed" << '\n';
        std::cerr << out_.str() << std::endl;
      } else {
        std::cout << "All tests passed (" << asserts_.pass << " asserts in "
                  << tests_.pass << " tests)\n";

        if (tests_.skip) {
          std::cout << tests_.skip << " tests skipped\n";
        }

        std::cout.flush();
      }
    }
  }

 protected:
  struct {
    std::size_t pass{};
    std::size_t fail{};
    std::size_t skip{};
  } tests_{};

  struct {
    std::size_t pass{};
    std::size_t fail{};
  } asserts_{};

  std::size_t fails_{};
  bool exception_{};

  std::stringstream out_{};
};

struct options {
  std::string_view filter{};
  bool dry_run{};
};

template <class TReporter = reporter, auto MaxPathSize = 16>
class runner {
  class filter {
    static constexpr auto delim = ".";

   public:
    constexpr explicit(false) filter(std::string_view filter = {})
        : path_{utility::split(filter, delim)} {}

    template <class TPath>
    constexpr auto operator()(const std::size_t level, const TPath& path) const
        -> bool {
      for (auto i = 0u; i < std::min(level + 1, std::size(path_)); ++i) {
        if (not utility::is_match(path[i], path_[i])) {
          return false;
        }
      }
      return true;
    }

   private:
    std::vector<std::string_view> path_{};
  };

 public:
  constexpr runner() = default;
  constexpr runner(TReporter reporter, std::size_t suites_size)
      : reporter_{std::move(reporter)}, suites_(suites_size) {}

  ~runner() {
    const auto should_run = not run_;

    if (should_run) {
      static_cast<void>(run());
    }

    if (not dry_run_) {
      reporter_.on(events::summary{});
    }

    if (should_run and fails_) {
      std::exit(-1);
    }
  }

  constexpr auto operator=(options options) {
    filter_ = options.filter;
    dry_run_ = options.dry_run;
  }

  template <class TSuite>
  auto on(events::suite<TSuite> suite) {
    suites_.push_back(suite.run);
  }

  template <class... Ts>
  auto on(events::test<Ts...> test) {
    path_[level_] = test.name;

    if (filter_(level_, path_)) {
      if (not level_++) {
        reporter_.on(events::test_begin{test.type, test.name});
      } else {
        reporter_.on(events::test_run{test.type, test.name});
      }

      if (dry_run_) {
        for (auto i = 0u; i < level_; ++i) {
          std::cout << (i ? "." : "") << path_[i];
        }
        std::cout << '\n';
      }

      active_exception_ = false;
      try {
        test();
      } catch (...) {
        reporter_.on(events::exception{});
        active_exception_ = true;
      }

      if (not--level_) {
        reporter_.on(events::test_end{test.type, test.name});
      }
    }
  }

  template <class... Ts>
  auto on(events::skip<Ts...> test) {
    reporter_.on(events::test_skip{test.type, test.name});
  }

  template <class TLocation, class TExpr>
  [[nodiscard]] auto on(events::assertion<TLocation, TExpr> assertion) -> bool {
    if (dry_run_) {
      return true;
    }

    if (static_cast<bool>(assertion.expr)) {
      reporter_.on(events::assertion_pass{assertion.location, assertion.expr});
      return true;
    } else {
      ++fails_;
      reporter_.on(events::assertion_fail{assertion.location, assertion.expr});
      return false;
    }
  }

  [[noreturn]] auto on(events::fatal_assertion fatal_assertion) {
    reporter_.on(fatal_assertion);
    reporter_.on(events::test_end{});
    reporter_.on(events::summary{});
    std::abort();
  }

  auto on(events::log l) { reporter_.on(l); }

  [[nodiscard]] auto run() -> bool {
    run_ = true;
    for (auto& suite : suites_) {
      suite();
    }
    suites_.clear();

    return fails_ > 0;
  }

 protected:
  TReporter reporter_{};
  std::vector<void (*)()> suites_{};
  std::size_t level_{};
  bool run_{};
  bool active_exception_{};
  std::size_t fails_{};
  std::array<std::string_view, MaxPathSize> path_{};
  filter filter_{};
  bool dry_run_{};
};

struct override {};

template <class = override, class...>
[[maybe_unused]] static auto cfg = runner<reporter>{};

namespace detail {
struct op {};
struct skip {};

template <class... Ts, class TEvent>
[[nodiscard]] constexpr decltype(auto) on(const TEvent& event) {
  return ut::cfg<typename type_traits::identity<override, Ts...>::type>.on(
      event);
}

struct test {
  std::string_view type{};
  std::string_view name{};

  template <class... Ts>
  constexpr auto operator=(void (*test)()) {
    on<Ts...>(events::test{type, name, none{}, test});
    return test;
  }

  constexpr auto operator=(void (*test)(std::string_view)) {
    return test(name);
  }

  template <class Test, type_traits::requires_t<(sizeof(Test) > 1)> = 0>
  constexpr auto operator=(Test test) {
    if constexpr (std::is_invocable_v<Test, std::string_view>) {
      return test(name);
    } else {
      on<Test>(events::test{type, name, none{}, test});
      return test;
    }
  }
};

template <class T>
class test_skip {
 public:
  constexpr explicit test_skip(T t) : t_{t} {}

  template <class Test>
  constexpr auto operator=(Test test) {
    on<Test>(events::skip{t_.type, t_.name, none{}});
    return test;
  }

 private:
  T t_;
};

struct log {
  template <class TMsg>
  auto& operator<<(const TMsg& msg) {
    on<TMsg>(events::log{"\n"});
    on<TMsg>(events::log{msg});
    return *this;
  }
};

template <class T>
class expect_ {
 public:
  constexpr explicit expect_(bool result) : result_{result} {}

  template <class TMsg>
  auto& operator<<(const TMsg& msg) {
    if (not result_) {
      detail::on<T>(events::log{" "});
      detail::on<T>(events::log{msg});
    }
    return *this;
  }

  constexpr auto& operator!() {
    fatal_ = true;
    return *this;
  }

  ~expect_() {
    if (not result_ and fatal_) {
      detail::on<T>(events::fatal_assertion{});
    }
  }

 private:
  bool result_{}, fatal_{};
};

template <class T, class = int>
class value : op {
 public:
  constexpr explicit value(const T& value) : value_(value) {}
  constexpr operator T() const { return value_; }
  constexpr decltype(auto) get() const { return value_; }

 private:
  T value_{};
};

template <class T>
class value<T, type_traits::requires_t<std::is_floating_point_v<T>>> : op {
 public:
  static inline auto epsilon = T{};

  constexpr value(const T& value, const T precision) : value_{value} {
    epsilon = precision;
  }

  constexpr explicit value(const T& val)
      : value{val, T(1) / math::pow(10, math::den_size<int>(val))} {}
  constexpr operator T() const { return value_; }
  constexpr decltype(auto) get() const { return value_; }

 private:
  T value_{};
};

template <auto N>
class integral_constant : op {
 public:
  static constexpr auto value = N;

  constexpr auto operator-() const { return integral_constant<-N>{}; }
  constexpr operator decltype(N)() const { return N; }
  constexpr auto get() const { return N; }
};

template <class T, auto N, auto D, auto Size, auto P = 1>
struct floating_point_constant : op {
  static constexpr auto epsilon = T(1) / math::pow(10, Size - 1);
  static constexpr auto value = T(P) * (T(N) + (T(D) / math::pow(10, Size)));

  constexpr auto operator-() const {
    return floating_point_constant<T, N, D, Size, -1>{};
  }
  constexpr operator T() const { return value; }
  constexpr auto get() const { return value; }
};

template <class T>
constexpr auto get_impl(const T& t, int) -> decltype(t.get()) {
  return t.get();
}
template <class T>
constexpr auto get_impl(const T& t, ...) -> decltype(auto) {
  return t;
}
template <class T>
constexpr auto get(const T& t) {
  return get_impl(t, 0);
}

template <class T>
struct type_ : op {
  static constexpr auto value = reflection::type_name<T>();
  constexpr auto get() const { return value; }
  friend auto operator<<(std::ostream& os, const type_& t) -> std::ostream& {
    return (os << t.value);
  }
};

template <class TLhs, class TRhs>
class eq_ : op {
 public:
  constexpr eq_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator==;
    using std::operator<;
    if constexpr (type_traits::has_value_v<TLhs> and
                  type_traits::has_value_v<TRhs>) {
      return TLhs::value == TRhs::value;
    } else if constexpr (type_traits::has_epsilon_v<TLhs> and
                         type_traits::has_epsilon_v<TRhs>) {
      return math::abs(get(lhs_) - get(rhs_)) <
             std::min(TLhs::epsilon, TRhs::epsilon);
    } else if constexpr (type_traits::has_epsilon_v<TLhs>) {
      return math::abs(get(lhs_) - get(rhs_)) < TLhs::epsilon;
    } else if constexpr (type_traits::has_epsilon_v<TRhs>) {
      return math::abs(get(lhs_) - get(rhs_)) < TRhs::epsilon;
    } else {
      return get(lhs_) == get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const eq_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << get(op.lhs_) << " == " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class neq_ : op {
 public:
  constexpr neq_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator!=;
    using std::operator>;
    if constexpr (type_traits::has_value_v<TLhs> and
                  type_traits::has_value_v<TRhs>) {
      return TLhs::value != TRhs::value;
    } else if constexpr (type_traits::has_epsilon_v<TLhs> and
                         type_traits::has_epsilon_v<TRhs>) {
      return math::abs(get(lhs_) - get(rhs_)) >
             std::min(TLhs::epsilon, TRhs::epsilon);
    } else if constexpr (type_traits::has_epsilon_v<TLhs>) {
      return math::abs(get(lhs_) - get(rhs_)) > TLhs::epsilon;
    } else if constexpr (type_traits::has_epsilon_v<TRhs>) {
      return math::abs(get(lhs_) - get(rhs_)) > TRhs::epsilon;
    } else {
      return get(lhs_) != get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const neq_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << get(op.lhs_) << " != " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class gt_ : op {
 public:
  constexpr gt_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator>;
    if constexpr (type_traits::has_value_v<TLhs> and
                  type_traits::has_value_v<TRhs>) {
      return TLhs::value > TRhs::value;
    } else {
      return get(lhs_) > get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const gt_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << get(op.lhs_) << " > " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class ge_ : op {
 public:
  constexpr ge_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator>=;
    if constexpr (type_traits::has_value_v<TLhs> and
                  type_traits::has_value_v<TRhs>) {
      return TLhs::value >= TRhs::value;
    } else {
      return get(lhs_) >= get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const ge_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << get(op.lhs_) << " >= " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class lt_ : op {
 public:
  constexpr lt_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator<;
    if constexpr (type_traits::has_value_v<TLhs> and
                  type_traits::has_value_v<TRhs>) {
      return TLhs::value < TRhs::value;
    } else {
      return get(lhs_) < get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const lt_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << get(op.lhs_) << " < " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class le_ : op {
 public:
  constexpr le_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator<=;
    if constexpr (type_traits::has_value_v<TLhs> and
                  type_traits::has_value_v<TRhs>) {
      return TLhs::value <= TRhs::value;
    } else {
      return get(lhs_) <= get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const le_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << get(op.lhs_) << " <= " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class and_ : op {
 public:
  constexpr and_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    return static_cast<bool>(lhs_) and static_cast<bool>(rhs_);
  }

  friend auto operator<<(std::ostream& os, const and_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << '(' << get(op.lhs_) << " and " << get(op.rhs_) << ')');
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class or_ : op {
 public:
  constexpr or_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    return static_cast<bool>(lhs_) or static_cast<bool>(rhs_);
  }

  friend auto operator<<(std::ostream& os, const or_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << '(' << get(op.lhs_) << " or " << get(op.rhs_) << ')');
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class T>
class not_ : op {
 public:
  explicit constexpr not_(const T& t = {}) : t_{t} {}

  constexpr operator bool() const { return not static_cast<bool>(t_); }

  friend auto operator<<(std::ostream& os, const not_& op) -> std::ostream& {
    using operators::operator<<;
    return (os << "not " << get(op.t_));
  }

 private:
  T t_{};
};

template <class TExpr, class TException = void>
class throws_ : op {
 public:
  constexpr explicit throws_(const TExpr& expr) : expr_{expr} {}

  constexpr operator bool() const {
    try {
      expr_();
    } catch (const TException&) {
      return true;
    } catch (...) {
      return false;
    }
    return false;
  }

 private:
  TExpr expr_{};
};

template <class TExpr>
class throws_<TExpr, void> : op {
 public:
  constexpr explicit throws_(const TExpr& expr) : expr_{expr} {}

  constexpr operator bool() const {
    try {
      expr_();
    } catch (...) {
      return true;
    }
    return false;
  }

 private:
  TExpr expr_{};
};

template <class TExpr>
class nothrow_ : op {
 public:
  constexpr explicit nothrow_(const TExpr& expr) : expr_{expr} {}

  constexpr operator bool() const {
    try {
      expr_();
    } catch (...) {
      return false;
    }
    return true;
  }

 private:
  TExpr expr_{};
};
}  // namespace detail

namespace literals {
constexpr auto operator""_test(const char* name, std::size_t size) {
  return detail::test{"test", std::string_view{name, size}};
}

template <char... Cs>
constexpr auto operator""_i() {
  return detail::integral_constant<math::num<int, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_s() {
  return detail::integral_constant<math::num<short, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_c() {
  return detail::integral_constant<math::num<char, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_l() {
  return detail::integral_constant<math::num<long, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_ll() {
  return detail::integral_constant<math::num<long long, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_u() {
  return detail::integral_constant<math::num<unsigned, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_uc() {
  return detail::integral_constant<math::num<unsigned char, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_us() {
  return detail::integral_constant<math::num<unsigned short, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_ul() {
  return detail::integral_constant<math::num<unsigned long, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_f() {
  return detail::floating_point_constant<
      float, math::num<unsigned long, Cs...>(), math::den<int, Cs...>(),
      math::den_size<int, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_d() {
  return detail::floating_point_constant<
      double, math::num<unsigned long, Cs...>(), math::den<int, Cs...>(),
      math::den_size<int, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_ld() {
  return detail::floating_point_constant<
      long double, math::num<unsigned long, Cs...>(), math::den<int, Cs...>(),
      math::den_size<int, Cs...>()>{};
}
}  // namespace literals

namespace type_traits {
template <class T>
static constexpr auto is_op_v = std::is_base_of_v<detail::op, T>;
}  // namespace type_traits

namespace operators {
constexpr auto operator==(std::string_view lhs, std::string_view rhs) {
  return detail::eq_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
constexpr auto operator==(const TLhs& lhs, const TRhs& rhs) {
  return detail::eq_{lhs, rhs};
}

template <class T, type_traits::requires_t<type_traits::is_container_v<T>> = 0>
constexpr auto operator==(T&& lhs, T&& rhs) {
  return detail::eq_{std::forward<T>(lhs), std::forward<T>(rhs)};
}

constexpr auto operator!=(std::string_view lhs, std::string_view rhs) {
  return detail::neq_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
constexpr auto operator!=(const TLhs& lhs, const TRhs& rhs) {
  return detail::neq_{lhs, rhs};
}

template <class T, type_traits::requires_t<type_traits::is_container_v<T>> = 0>
constexpr auto operator!=(T&& lhs, T&& rhs) {
  return detail::neq_{std::forward<T>(lhs), std::forward<T>(rhs)};
}

template <
    class TLhs, class TRhs,
    type_traits::requires_t<
        type_traits::is_op_v<TLhs> or type_traits::is_op_v<TRhs>, int> = 0>
constexpr auto operator>(const TLhs& lhs, const TRhs& rhs) {
  return detail::gt_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    type_traits::requires_t<
        type_traits::is_op_v<TLhs> or type_traits::is_op_v<TRhs>, int> = 0>
constexpr auto operator>=(const TLhs& lhs, const TRhs& rhs) {
  return detail::ge_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    type_traits::requires_t<
        type_traits::is_op_v<TLhs> or type_traits::is_op_v<TRhs>, int> = 0>
constexpr auto operator<(const TLhs& lhs, const TRhs& rhs) {
  return detail::lt_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    type_traits::requires_t<
        type_traits::is_op_v<TLhs> or type_traits::is_op_v<TRhs>, int> = 0>
constexpr auto operator<=(const TLhs& lhs, const TRhs& rhs) {
  return detail::le_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    type_traits::requires_t<
        type_traits::is_op_v<TLhs> or type_traits::is_op_v<TRhs>, int> = 0>
constexpr auto operator and(const TLhs& lhs, const TRhs& rhs) {
  return detail::and_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    type_traits::requires_t<
        type_traits::is_op_v<TLhs> or type_traits::is_op_v<TRhs>, int> = 0>
constexpr auto operator or(const TLhs& lhs, const TRhs& rhs) {
  return detail::or_{lhs, rhs};
}

template <class T, type_traits::requires_t<type_traits::is_op_v<T>> = 0>
constexpr auto operator not(const T& t) {
  return detail::not_{t};
}

template <class T>
constexpr auto operator|(const detail::skip&, const T& t) {
  return detail::test_skip{t};
}

template <class F, class T,
          type_traits::requires_t<type_traits::is_container_v<T>> = 0>
constexpr auto operator|(const F& f, const T& t) {
  return [f, t](auto name) {
    for (const auto& arg : t) {
      detail::on<F>(events::test{"test", name, arg, f});
    }
  };
}

template <class F, class... Ts>
constexpr auto operator|(const F& f, const std::tuple<Ts...>& t) {
  return [f, t](auto name) {
    std::apply(
        [f, name](const auto&... args) {
          (detail::on<F>(events::test{"test", name, args, f}), ...);
        },
        t);
  };
}
}  //  namespace operators

template <class TExpr, type_traits::requires_t<type_traits::is_op_v<TExpr> or
                                               std::is_same_v<bool, TExpr>> = 0>
constexpr auto expect(const TExpr& expr,
                      const std::experimental::source_location& location =
                          std::experimental::source_location::current()) {
  return detail::expect_<TExpr>{
      detail::on<TExpr>(events::assertion{location, expr})};
}

#if defined(__cpp_nontype_template_parameter_class)
template <auto Constant>
#else
template <bool Constant>
#endif
constexpr auto constant = Constant;

template <class TException, class TExpr>
constexpr auto throws(const TExpr& expr) {
  return detail::throws_<TExpr, TException>{expr};
}

template <class TExpr>
constexpr auto throws(const TExpr& expr) {
  return detail::throws_<TExpr>{expr};
}

template <class TExpr>
constexpr auto nothrow(const TExpr& expr) {
  return detail::nothrow_{expr};
}

using _i = detail::value<int>;
using _b = detail::value<bool>;
using _c = detail::value<char>;
using _s = detail::value<short>;
using _l = detail::value<long>;
using _ll = detail::value<long long>;
using _f = detail::value<float>;
using _d = detail::value<double>;
using _ld = detail::value<long double>;
using _u = detail::value<unsigned>;
using _uc = detail::value<unsigned char>;
using _us = detail::value<unsigned short>;
using _ul = detail::value<unsigned long>;
using _f = detail::value<float>;
using _d = detail::value<double>;
using _ld = detail::value<long double>;

template <class T>
struct _t : detail::value<T> {
  constexpr explicit _t(const T& t) : detail::value<T>{t} {}
};

struct suite {
  template <class TSuite>
  constexpr explicit(false) suite(TSuite suite) {
    static_assert(std::is_empty_v<TSuite>);
    detail::on<TSuite>(events::suite{suite});
  }
};

[[maybe_unused]] constexpr auto true_b = detail::integral_constant<true>{};
[[maybe_unused]] constexpr auto false_b = detail::integral_constant<false>{};

[[maybe_unused]] static auto log = detail::log{};
[[maybe_unused]] constexpr auto skip = detail::skip{};
[[maybe_unused]] constexpr auto given = [](std::string_view name) {
  return detail::test{"given", name};
};
[[maybe_unused]] constexpr auto when = [](std::string_view name) {
  return detail::test{"when", name};
};
[[maybe_unused]] constexpr auto then = [](std::string_view name) {
  return detail::test{"then", name};
};
template <class T>
[[maybe_unused]] static constexpr auto type = detail::type_<T>{};

using namespace literals;
using namespace operators;
}  // namespace v1_0_0
}  // namespace boost::ut
#endif
