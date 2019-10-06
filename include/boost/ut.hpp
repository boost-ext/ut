//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#if not defined(__cpp_variadic_templates) or                               \
    not defined(__cpp_rvalue_references) or not defined(__cpp_decltype) or \
    not defined(__cpp_alias_templates) or                                  \
    not defined(__cpp_generic_lambdas) or not defined(__cpp_constexpr) or  \
    not defined(__cpp_return_type_deduction) or                            \
    not defined(__cpp_fold_expressions) or not defined(__cpp_deduction_guides)
#error "[Boost].UT requires C++20 support"
#else
#include <cstddef>
#include <cstdlib>
#include <experimental/source_location>
#include <experimental/type_traits>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ut {
inline namespace v1 {
namespace reflection {
template <class T>
constexpr auto type_name() {
#if defined(__clang__)
  constexpr auto REFLECTION_TYPE_NAME_OFFSET = 42;
#elif defined(__GNUC__)
  constexpr auto REFLECTION_TYPE_NAME_OFFSET = 57;
#else
#error "Compiler is not supported!"
#endif
  return std::string_view{
      &__PRETTY_FUNCTION__[REFLECTION_TYPE_NAME_OFFSET],
      sizeof(__PRETTY_FUNCTION__) - REFLECTION_TYPE_NAME_OFFSET - 2};
}
}  // namespace reflection

namespace events {
template <class Test>
struct test_run {
  constexpr test_run(std::string_view name, Test test)
      : name{name}, test{test} {}
  std::string_view name{};
  Test test{};
};
template <class Test>
struct test_skip {
  constexpr test_skip(std::string_view name, Test test)
      : name{name}, test{test} {}
  std::string_view name{};
  Test test{};
};
template <class TLocation, class TExpr>
struct assertion {
  constexpr assertion(TLocation location, TExpr expr)
      : location{location}, expr{expr} {}
  TLocation location{};
  TExpr expr{};
};
struct log {
  std::string_view msg{};
};
struct fatal_assertion {};
}  // namespace events

class default_cfg {
  auto test_begin(std::string_view name) {
    out_ << "Running \"" << name << "\"...";
    fails_ = asserts_.fail;
  }

  auto test_end() {
    if (asserts_.fail > fails_) {
      ++tests_.fail;
      out_ << "\nFAILED\n";
    } else {
      ++tests_.pass;
      out_ << "OK\n";
    }
  }

 public:
  template <class T>
  auto on(events::test_run<T> test) {
    if (const auto filter = std::getenv("BOOST_UT_FILTER");
        not filter or
        (filter and std::string_view{filter} == std::string_view{test.name})) {
      if (not level_++) {
        test_begin(test.name);
      } else {
        out_ << "\n \"" << test.name << "\"...";
      }

      test.test();

      if (not--level_) {
        test_end();
      }
    }
  }

  template <class Test>
  auto on(events::test_skip<Test> test) {
    out_ << test.name << "...SKIPPED\n";
    ++tests_.skip;
  }

  template <class TLocation, class TExpr>
  auto on(events::assertion<TLocation, TExpr> assertion) -> bool {
    if (const auto result = static_cast<bool>(assertion.expr); result) {
      ++asserts_.pass;
      return true;
    } else {
      constexpr auto short_name = [](std::string_view name) {
        return name.rfind('/') != std::string_view::npos
                   ? name.substr(name.rfind('/') + 1)
                   : name;
      };
      out_ << "\n  " << short_name(assertion.location.file_name()) << ':'
           << assertion.location.line() << ":FAILED [" << std::boolalpha
           << assertion.expr << ']';
      ++asserts_.fail;
      return false;
    }
  }

  auto on(events::fatal_assertion) {
    ++asserts_.fail;
    test_end();
    std::exit(-1);
  }

  auto on(events::log log) { out_ << ' ' << log.msg; }

  ~default_cfg() {
    if (tests_.fail) {
      out_ << "\n=============================================================="
              "=================\n"
           << "tests:   " << (tests_.pass + tests_.fail) << " | " << tests_.fail
           << " failed" << '\n'
           << "asserts: " << (asserts_.pass + asserts_.fail) << " | "
           << asserts_.pass << " passed"
           << " | " << asserts_.fail << " failed" << '\n';
      std::cerr << out_.str() << std::endl;
    } else if (tests_.pass) {
      std::cout << "All tests passed (" << asserts_.pass << " asserts in "
                << tests_.pass << " tests)\n";

      if (tests_.skip) {
        std::cout << tests_.skip << " tests skipped\n";
      }

      std::cout.flush();
    }
  }

 private:
  static inline std::stringstream out_{};

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
  std::size_t level_{};
};

struct override {};

template <class...>
[[maybe_unused]] inline auto cfg = default_cfg{};

namespace detail {
template <class T, class...>
struct identity {
  using type = T;
};
struct op {};
struct skip {};

class cfg {
  template <class... Ts>
  using override_t = typename identity<override, Ts...>::type;

 public:
  template <class T, class TEvent>
  static constexpr auto on(const TEvent& event) {
    return ut::cfg<override_t<T>>.on(event);
  }

  template <class T>
  auto& operator<<(T&& t) {
    on<T>(events::log{std::forward<T>(t)});
    return *this;
  }
};

class test {
 public:
  constexpr explicit test(std::string_view name) : name_{name} {}

  template <class Test>
  constexpr decltype(auto) operator=(Test test) {
    cfg::on<Test>(events::test_run{name_, test});
    return test;
  }

  constexpr auto name() const { return name_; }

 private:
  std::string_view name_{};
};

template <class T>
class test_skip {
 public:
  constexpr explicit test_skip(T t) : t_{t} {}

  template <class Test>
  constexpr decltype(auto) operator=(Test test) {
    cfg::on<Test>(events::test_skip{t_.name(), test});
    return test;
  }

 private:
  T t_;
};

template <class T>
class expect_ {
 public:
  constexpr explicit expect_(bool result) : result_{result} {}

  template <class TMsg>
  auto& operator<<(const TMsg& msg) {
    if (not result_) {
      detail::cfg::on<T>(events::log{msg});
    }
    return *this;
  }

  auto& operator!() {
    fatal_ = true;
    return *this;
  }

  ~expect_() {
    if (not result_ and fatal_) {
      detail::cfg::on<T>(events::fatal_assertion{});
    }
  }

 private:
  bool result_{};
  bool fatal_{};
};

template <class T>
class value : op {
 public:
  constexpr value(const T& value) : value_{value} {}
  constexpr operator T() const { return value_; }
  constexpr decltype(auto) get() const { return value_; }

 private:
  T value_{};
};

template <auto N>
class integral_constant : op {
 public:
  static constexpr auto value = N;
  constexpr operator auto() const { return N; }
  constexpr auto get() const { return N; }
};

template <class T>
constexpr T pow(const T base, const std::size_t exp) {
  return exp ? base * pow(base, exp - 1) : T(1);
}

template <class T, char... Cs>
constexpr auto num() {
  constexpr char cs[]{Cs...};
  T result = {};
  auto size = 0u, i = 0u;

  while (i < sizeof...(Cs) and cs[i] != '.') {
    if (cs[i++] != '\'') {
      ++size;
    }
  }

  auto skip = 1;
  i = 0u;
  while (i < sizeof...(Cs) and cs[i] != '.') {
    if (cs[i] == '\'') {
      --skip;
    } else {
      result += pow(T(10), size - i - skip) * (cs[i] - '0');
    }
    ++i;
  }
  return result;
}

template <char... Cs>
constexpr auto den() {
  constexpr char cs[]{Cs...};
  auto result = 0;
  auto i = 0u;
  while (cs[i++] != '.')
    ;
  for (auto j = i; j < sizeof...(Cs); ++j) {
    result += pow(10, sizeof...(Cs) - j) * (cs[j] - '0');
  }
  return result;
}

template <class T, char... Cs>
constexpr auto den_size() {
  constexpr char cs[]{Cs...};
  auto i = 0u;
  while (cs[i++] != '.')
    ;
  return sizeof...(Cs) - i + 1;
}

template <class T, auto N, auto D, auto Size, auto P = 1>
struct floating_point_constant : op {
  static constexpr auto epsilon = T(1) / pow(10, Size);
  static constexpr auto value = T(P) * (T(N) + (T(D) / pow(10, Size)));
  constexpr operator T() const { return value; }
  constexpr auto get() const { return value; }
};

template <class T>
constexpr auto is_op_v = std::is_base_of_v<op, T> or std::is_class_v<T>;

template <class T>
using is_floating_point_constant_t = decltype(T::epsilon);
template <class T>
constexpr auto is_floating_point_constant_v =
    std::experimental::is_detected_v<is_floating_point_constant_t, T>;

template <class T>
using is_integral_constant_t = decltype(T::value);
template <class T>
constexpr auto is_integral_constant_v =
    std::experimental::is_detected_v<is_integral_constant_t, T>;

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
  friend auto operator<<(std::ostream& os, const type_& t) -> std::ostream& {
    return (os << t.value);
  }
};

template <class T>
class bool_ : op {
 public:
  constexpr bool_(const T& t) : t_{t} {}

  friend auto operator<<(std::ostream& os, const bool_& op) -> std::ostream& {
    return (os << get(op.t_));
  }

  constexpr operator bool() const { return t_; }

 private:
  T t_{};
};

template <class TLhs, class TRhs>
class eq_ : op {
 public:
  constexpr eq_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator==;
    using std::operator<;
    if constexpr (is_integral_constant_v<TLhs> and
                  is_integral_constant_v<TRhs>) {
      return TLhs::value == TRhs::value;
    } else if constexpr (is_floating_point_constant_v<TLhs>) {
      return std::abs(get(lhs_) - get(rhs_)) < TLhs::epsilon;
    } else if constexpr (is_floating_point_constant_v<TRhs>) {
      return std::abs(get(lhs_) - get(rhs_)) < TRhs::epsilon;
    } else {
      return get(lhs_) == get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const eq_& op) -> std::ostream& {
    return (os << get(op.lhs_) << " == " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class neq_ : op {
 public:
  constexpr neq_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator!=;
    if constexpr (is_integral_constant_v<TLhs> and
                  is_integral_constant_v<TRhs>) {
      return TLhs::value != TRhs::value;
    } else {
      return get(lhs_) != get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const neq_& op) -> std::ostream& {
    return (os << get(op.lhs_) << " != " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class gt_ : op {
 public:
  constexpr gt_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator>;
    if constexpr (is_integral_constant_v<TLhs> and
                  is_integral_constant_v<TRhs>) {
      return TLhs::value > TRhs::value;
    } else {
      return get(lhs_) > get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const gt_& op) -> std::ostream& {
    return (os << get(op.lhs_) << " > " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class ge_ : op {
 public:
  constexpr ge_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator>=;
    if constexpr (is_integral_constant_v<TLhs> and
                  is_integral_constant_v<TRhs>) {
      return TLhs::value >= TRhs::value;
    } else {
      return get(lhs_) >= get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const ge_& op) -> std::ostream& {
    return (os << get(op.lhs_) << " >= " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class lt_ : op {
 public:
  constexpr lt_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator<;
    if constexpr (is_integral_constant_v<TLhs> and
                  is_integral_constant_v<TRhs>) {
      return TLhs::value < TRhs::value;
    } else {
      return get(lhs_) < get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const lt_& op) -> std::ostream& {
    return (os << get(op.lhs_) << " < " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class le_ : op {
 public:
  constexpr le_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    using std::operator<=;
    if constexpr (is_integral_constant_v<TLhs> and
                  is_integral_constant_v<TRhs>) {
      return TLhs::value <= TRhs::value;
    } else {
      return get(lhs_) <= get(rhs_);
    }
  }

  friend auto operator<<(std::ostream& os, const le_& op) -> std::ostream& {
    return (os << get(op.lhs_) << " <= " << get(op.rhs_));
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class and_ : op {
 public:
  constexpr and_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    return static_cast<bool>(lhs_) and static_cast<bool>(rhs_);
  }

  friend auto operator<<(std::ostream& os, const and_& op) -> std::ostream& {
    return (os << '(' << get(op.lhs_) << " and " << get(op.rhs_) << ')');
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class TLhs, class TRhs>
class or_ : op {
 public:
  constexpr or_(const TLhs& lhs, const TRhs& rhs) : lhs_{lhs}, rhs_{rhs} {}

  constexpr operator bool() const {
    return static_cast<bool>(lhs_) or static_cast<bool>(rhs_);
  }

  friend auto operator<<(std::ostream& os, const or_& op) -> std::ostream& {
    return (os << '(' << get(op.lhs_) << " or " << get(op.rhs_) << ')');
  }

 private:
  TLhs lhs_{};
  TRhs rhs_{};
};

template <class T>
class not_ : op {
 public:
  explicit constexpr not_(const T& t) : t_{t} {}

  constexpr operator bool() const { return not static_cast<bool>(t_); }

  friend auto operator<<(std::ostream& os, const not_& op) -> std::ostream& {
    return (os << " not " << get(op.t_));
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
  return detail::test{std::string_view{name, size}};
}

template <char... Cs>
constexpr auto operator""_i() {
  return detail::integral_constant<detail::num<int, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_s() {
  return detail::integral_constant<detail::num<short, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_c() {
  return detail::integral_constant<detail::num<char, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_l() {
  return detail::integral_constant<detail::num<long, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_ll() {
  return detail::integral_constant<detail::num<long long, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_u() {
  return detail::integral_constant<detail::num<unsigned, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_uc() {
  return detail::integral_constant<detail::num<unsigned char, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_us() {
  return detail::integral_constant<detail::num<unsigned short, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_ul() {
  return detail::integral_constant<detail::num<unsigned long, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_f() {
  return detail::floating_point_constant<float, detail::num<int, Cs...>(),
                                         detail::den<Cs...>(),
                                         detail::den_size<int, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_d() {
  return detail::floating_point_constant<double, detail::num<int, Cs...>(),
                                         detail::den<Cs...>(),
                                         detail::den_size<int, Cs...>()>{};
}

template <char... Cs>
constexpr auto operator""_ld() {
  return detail::floating_point_constant<long double, detail::num<int, Cs...>(),
                                         detail::den<Cs...>(),
                                         detail::den_size<int, Cs...>()>{};
}
}  // namespace literals

namespace operators {
template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> or detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator==(const TLhs& lhs, const TRhs& rhs) {
  return detail::eq_{lhs, rhs};
}

constexpr auto operator==(std::string_view lhs, std::string_view rhs) {
  return detail::eq_{lhs, rhs};
}

inline auto operator==(const std::string& lhs, const std::string& rhs) {
  return detail::eq_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> or detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator!=(const TLhs& lhs, const TRhs& rhs) {
  return detail::neq_{lhs, rhs};
}

inline constexpr auto operator!=(std::string_view lhs, std::string_view rhs) {
  return detail::neq_{lhs, rhs};
}

inline auto operator!=(const std::string& lhs, const std::string& rhs) {
  return detail::neq_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> or detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator>(const TLhs& lhs, const TRhs& rhs) {
  return detail::gt_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> or detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator>=(const TLhs& lhs, const TRhs& rhs) {
  return detail::ge_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> or detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator<(const TLhs& lhs, const TRhs& rhs) {
  return detail::lt_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> or detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator<=(const TLhs& lhs, const TRhs& rhs) {
  return detail::le_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> and detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator and(const TLhs& lhs, const TRhs& rhs) {
  return detail::and_{lhs, rhs};
}

template <
    class TLhs, class TRhs,
    std::enable_if_t<detail::is_op_v<TLhs> and detail::is_op_v<TRhs>, int> = 0>
constexpr auto operator or(const TLhs& lhs, const TRhs& rhs) {
  return detail::or_{lhs, rhs};
}

template <class T, std::enable_if_t<detail::is_op_v<T>, int> = 0>
constexpr auto operator not(const T& t) {
  return detail::not_{t};
}

template <class T>
constexpr auto operator|(const detail::skip&, const T& t) {
  return detail::test_skip{t};
}

template <class F, class TContainer,
          std::enable_if_t<
              std::is_invocable_v<F, typename TContainer::value_type>, int> = 0>
constexpr auto operator|(const F& f, const TContainer& container) {
  return [=] {
    for (const auto& arg : container) {
      f(arg);
    }
  };
}

namespace detail {
template <class F, class TArg>
constexpr auto call(const F& f, const TArg& arg) {
  if constexpr (std::is_invocable_v<F, TArg>) {
    f(arg);
  } else {
    f.template operator()<TArg>();
  }
}
}  // namespace detail

template <class F, class... Ts>
constexpr auto operator|(const F& f, const std::tuple<Ts...>& t) {
  return [=] {
    return std::apply(
        [=](const auto&... args) { (detail::call(f, args), ...); }, t);
  };
}
}  //  namespace operators

template <class TExpr, std::enable_if_t<detail::is_op_v<TExpr>, int> = 0>
constexpr auto expect(const TExpr& expr,
                      const std::experimental::source_location& location =
                          std::experimental::source_location::current()) {
  return detail::expect_<TExpr>{
      detail::cfg::on<TExpr>(events::assertion{location, expr})};
}

template <class T, std::enable_if_t<std::is_same_v<bool, T>, int> = 0>
constexpr auto expect(T result,
                      const std::experimental::source_location& location =
                          std::experimental::source_location::current()) {
  return detail::expect_<T>{
      detail::cfg::on<T>(events::assertion{location, detail::bool_{result}})};
}

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

[[maybe_unused]] constexpr auto true_b = detail::integral_constant<true>{};
[[maybe_unused]] constexpr auto false_b = detail::integral_constant<false>{};

[[maybe_unused]] inline auto log = detail::cfg{};
[[maybe_unused]] constexpr auto skip = detail::skip{};
[[maybe_unused]] constexpr auto given = [](std::string_view name) {
  return detail::test{name};
};
[[maybe_unused]] constexpr auto when = [](std::string_view name) {
  return detail::test{name};
};
[[maybe_unused]] constexpr auto then = [](std::string_view name) {
  return detail::test{name};
};
template <class T = void>
[[maybe_unused]] constexpr auto type = detail::type_<T>{};
template <class T>
[[maybe_unused]] constexpr auto exception = detail::type_<T>{};

using namespace literals;
using namespace operators;
}  // namespace v1
}  // namespace ut
#endif
