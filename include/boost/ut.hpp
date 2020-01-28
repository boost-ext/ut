//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#if defined(__cpp_modules)
export module boost.ut;
export import std;
#else
#pragma once
#endif

#if defined(_MSC_VER)
#include <ciso646>  // and, or, not
#endif

#if not defined(__cpp_rvalue_references)
#error "[Boost].UT requires support for rvalue references";
#elif not defined(__cpp_decltype)
#error "[Boost].UT requires support for decltype";
#elif not defined(__cpp_return_type_deduction)
#error "[Boost].UT requires support for return type deduction";
#elif not defined(__cpp_deduction_guides)
#error "[Boost].UT requires support for return deduction guides";
#elif not defined(__cpp_generic_lambdas)
#error "[Boost].UT requires support for generic lambdas";
#elif not defined(__cpp_constexpr)
#error "[Boost].UT requires support for constexpr";
#elif not defined(__cpp_alias_templates)
#error "[Boost].UT requires support for alias templates";
#elif not defined(__cpp_variadic_templates)
#error "[Boost].UT requires support for variadic templates";
#elif not defined(__cpp_fold_expressions)
#error "[Boost].UT requires support for return fold expressions";
#elif not defined(__cpp_static_assert)
#error "[Boost].UT requires support for static assert";
#else
#define BOOST_UT_VERSION 1'1'6

#if defined(__has_builtin) and (__GNUC__ < 10) and not defined(__clang__)
#undef __has_builtin
#endif

#if not defined(__has_builtin)
#if (__GNUC__ >= 9)
#define __has___builtin_FILE 1
#define __has___builtin_LINE 1
#endif
#define __has_builtin(...) __has_##__VA_ARGS__
#endif

#if defined(BOOST_UT_FORWARD) and (__GNUC__ < 10)
namespace std {
template <class TLhs, class TRhs>
auto operator==(TLhs, TRhs) -> bool;
template <class TLhs, class TRhs>
auto operator<(TLhs, TRhs) -> bool;
template <class TLhs, class TRhs>
auto operator<=(TLhs, TRhs) -> bool;
template <class TLhs, class TRhs>
auto operator!=(TLhs, TRhs) -> bool;
template <class TLhs, class TRhs>
auto operator>(TLhs, TRhs) -> bool;
template <class TLhs, class TRhs>
auto operator>=(TLhs, TRhs) -> bool;
}  // namespace std
#else
#include <array>
#include <iostream>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>
#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>)
#include <sys/wait.h>
#include <unistd.h>
#endif
#if defined(__cpp_exceptions)
#include <exception>
#endif
#endif

#if defined(__cpp_modules)
export namespace boost::ut {
#else
namespace boost::ut {
#endif

inline namespace v1_1_6 {
namespace utility {
class string_view {
 public:
  constexpr string_view() = default;
  template <class TStr>
  constexpr /*explicit(false)*/ string_view(const TStr& str)
      : data_{str.c_str()}, size_{str.size()} {}
  constexpr /*explicit(false)*/ string_view(const char* const data)
      : data_{data}, size_{[data] {
          decltype(sizeof("")) size{};
          for (size = {}; data[size] != '\0'; ++size)
            ;
          return size;
        }()} {}
  constexpr string_view(const char* const data, decltype(sizeof("")) size)
      : data_{data}, size_{size} {}
  template <auto N>
  constexpr string_view(const char (&data)[N]) : data_{data}, size_{N - 1} {}

  template <class T>
  [[nodiscard]] constexpr operator T() const {
    return T{data_, size_};
  }

  template <class TOs>
  friend auto operator<<(TOs& os, const string_view& sv) -> TOs& {
    os.write(sv.data_, long(sv.size_));
    return os;
  }

 private:
  const char* data_{};
  decltype(sizeof("")) size_{};
};

#if defined(BOOST_UT_FORWARD) or defined(BOOST_UT_IMPLEMENTATION)
template <class>
class function;
template <class R, class... TArgs>
class function<R(TArgs...)> {
 public:
  template <class T>
  constexpr /*explicit(false)*/ function(T data)
      : invoke_{invoke_impl<T>},
        destroy_{destroy_impl<T>},
        data_{new T{static_cast<T&&>(data)}} {}
  constexpr function(function&& other)
      : invoke_{static_cast<decltype(other.invoke_)&&>(other.invoke_)},
        destroy_{static_cast<decltype(other.destroy_)&&>(other.destroy_)},
        data_{static_cast<decltype(other.data_)&&>(other.data_)} {
    other.data_ = {};
  }
  constexpr function(const function&) = delete;
  ~function() { destroy_(data_); }

  constexpr auto& operator=(const function&) = delete;
  constexpr auto& operator=(function&&) = delete;
  [[nodiscard]] constexpr auto operator()(TArgs... args) -> R {
    return invoke_(data_, args...);
  }
  [[nodiscard]] constexpr auto operator()(TArgs... args) const -> R {
    return invoke_(data_, args...);
  }

 private:
  template <class T>
  [[nodiscard]] static auto invoke_impl(void* data, TArgs... args) -> R {
    return (*static_cast<T*>(data))(args...);
  }

  template <class T>
  static auto destroy_impl(void* data) -> void {
    delete static_cast<T*>(data);
  }

  R (*invoke_)(void*, TArgs...){};
  void (*destroy_)(void*){};
  void* data_{};
};
#endif

#if not defined(BOOST_UT_FORWARD)
[[nodiscard]] inline auto is_match(std::string_view input,
                                   std::string_view pattern) -> bool {
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

[[nodiscard]] inline auto split(std::string_view input, std::string_view delim)
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
#endif
}  // namespace utility

namespace reflection {
class source_location {
 public:
  [[nodiscard]] static constexpr auto current(

#if (__has_builtin(__builtin_FILE) and __has_builtin(__builtin_LINE))
      const char* file = __builtin_FILE(), int line = __builtin_LINE()
#else
      const char* file = "unknown", int line = {}
#endif
          ) noexcept {
    source_location sl{};
    sl.file_ = file;
    sl.line_ = line;
    return sl;
  }
  [[nodiscard]] constexpr auto file_name() const noexcept { return file_; }
  [[nodiscard]] constexpr auto line() const noexcept { return line_; }

 private:
  const char* file_{"unknown"};
  int line_{};
};

template <class T>
[[nodiscard]] constexpr auto type_name() -> utility::string_view {
#if defined(_MSC_VER) and not defined(__clang__)
  return {&__FUNCSIG__[95], sizeof(__FUNCSIG__) - 103};
#elif defined(__clang__)
  return {&__PRETTY_FUNCTION__[69], sizeof(__PRETTY_FUNCTION__) - 71};
#elif defined(__GNUC__)
  return {&__PRETTY_FUNCTION__[103], sizeof(__PRETTY_FUNCTION__) - 105};
#endif
}
}  // namespace reflection

namespace math {
template <class T>
[[nodiscard]] constexpr auto abs(const T t) -> T {
  return t < T{} ? -t : t;
}

template <class T>
[[nodiscard]] constexpr auto min_value(const T& lhs, const T& rhs) -> const T& {
  return (rhs < lhs) ? rhs : lhs;
}

template <class T, class TExp>
[[nodiscard]] constexpr auto pow(const T base, const TExp exp) -> T {
  return exp ? T(base * pow(base, exp - TExp(1))) : T(1);
}

template <class T, char... Cs>
[[nodiscard]] constexpr auto num() -> T {
  static_assert(
      ((Cs == '.' or Cs == '\'' or (Cs >= '0' and Cs <= '9')) and ...));
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
[[nodiscard]] constexpr auto den() -> T {
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
[[nodiscard]] constexpr auto den_size() -> T {
  constexpr const char cs[]{Cs...};
  T i{};
  while (cs[i++] != '.')
    ;
  return T(sizeof...(Cs)) - i + T(1);
}

template <class T, class TValue>
[[nodiscard]] constexpr auto den_size(TValue value) -> T {
  constexpr auto precision = TValue(1e-7);
  T result{};
  TValue tmp{};
  do {
    value *= 10;
    tmp = value - T(value);
    ++result;
  } while (tmp > precision);

  return result;
}

}  // namespace math

namespace type_traits {
template <class T, class...>
struct identity {
  using type = T;
};

template <class T>
T&& declval();
template <class... Ts, class TExpr>
constexpr auto is_valid(TExpr expr)
    -> decltype(expr(declval<Ts...>()), bool()) {
  return true;
}
template <class...>
constexpr auto is_valid(...) -> bool {
  return false;
}

template <class T>
constexpr auto is_container_v =
    is_valid<T>([](auto t) -> decltype(t.begin(), t.end(), void()) {});

template <class T>
constexpr auto has_npos_v = is_valid<T>([](auto t) -> decltype(void(t.npos)) {
});

template <class T>
constexpr auto has_value_v = is_valid<T>([](auto t) -> decltype(void(t.value)) {
});

template <class T>
constexpr auto has_epsilon_v =
    is_valid<T>([](auto t) -> decltype(void(t.epsilon)) {});

template <class T>
inline constexpr auto is_floating_point_v = false;
template <>
inline constexpr auto is_floating_point_v<float> = true;
template <>
inline constexpr auto is_floating_point_v<double> = true;
template <>
inline constexpr auto is_floating_point_v<long double> = true;

#if defined(__clang__) or defined(_MSC_VER)
template <class From, class To>
constexpr auto is_convertible_v = __is_convertible_to(From, To);
#else
template <class From, class To>
constexpr auto is_convertible(int) -> decltype(bool(To(declval<From>()))) {
  return true;
}
template <class...>
constexpr auto is_convertible(...) {
  return false;
}
template <class From, class To>
constexpr auto is_convertible_v = is_convertible<From, To>(0);
#endif

template <bool>
struct requires_ {};
template <>
struct requires_<true> {
  using type = int;
};

template <bool Cond>
using requires_t = typename requires_<Cond>::type;
}  // namespace type_traits

namespace io {
#if defined(BOOST_UT_FORWARD)
struct ostream;
extern auto operator<<(ostream& os, bool) -> ostream&;
extern auto operator<<(ostream& os, char) -> ostream&;
extern auto operator<<(ostream& os, short) -> ostream&;
extern auto operator<<(ostream& os, int) -> ostream&;
extern auto operator<<(ostream& os, long) -> ostream&;
extern auto operator<<(ostream& os, long long) -> ostream&;
extern auto operator<<(ostream& os, unsigned) -> ostream&;
extern auto operator<<(ostream& os, unsigned char) -> ostream&;
extern auto operator<<(ostream& os, unsigned short) -> ostream&;
extern auto operator<<(ostream& os, unsigned long) -> ostream&;
extern auto operator<<(ostream& os, float) -> ostream&;
extern auto operator<<(ostream& os, double) -> ostream&;
extern auto operator<<(ostream& os, long double) -> ostream&;
extern auto operator<<(ostream& os, char const*) -> ostream&;
#elif defined(BOOST_UT_IMPLEMENTATION)
struct ostream : std::ostream {
  using std::ostream::ostream;
};
auto operator<<(ostream& os, bool b) -> ostream& {
  static_cast<std::ostream&>(os) << b;
  return os;
}
auto operator<<(ostream& os, char c) -> ostream& {
  static_cast<std::ostream&>(os) << c;
  return os;
}
auto operator<<(ostream& os, short s) -> ostream& {
  static_cast<std::ostream&>(os) << s;
  return os;
}
auto operator<<(ostream& os, int i) -> ostream& {
  static_cast<std::ostream&>(os) << i;
  return os;
}
auto operator<<(ostream& os, long l) -> ostream& {
  static_cast<std::ostream&>(os) << l;
  return os;
}
auto operator<<(ostream& os, long long ll) -> ostream& {
  static_cast<std::ostream&>(os) << ll;
  return os;
}
auto operator<<(ostream& os, unsigned u) -> ostream& {
  static_cast<std::ostream&>(os) << u;
  return os;
}
auto operator<<(ostream& os, unsigned char uc) -> ostream& {
  static_cast<std::ostream&>(os) << uc;
  return os;
}
auto operator<<(ostream& os, unsigned short us) -> ostream& {
  static_cast<std::ostream&>(os) << us;
  return os;
}
auto operator<<(ostream& os, unsigned long ul) -> ostream& {
  static_cast<std::ostream&>(os) << ul;
  return os;
}
auto operator<<(ostream& os, float f) -> ostream& {
  static_cast<std::ostream&>(os) << f;
  return os;
}
auto operator<<(ostream& os, double d) -> ostream& {
  static_cast<std::ostream&>(os) << d;
  return os;
}
auto operator<<(ostream& os, long double ld) -> ostream& {
  static_cast<std::ostream&>(os) << ld;
  return os;
}
auto operator<<(ostream& os, char const* s) -> ostream& {
  static_cast<std::ostream&>(os) << s;
  return os;
}
#endif
}  // namespace io

namespace detail {
struct op {};

template <class T>
[[nodiscard]] constexpr auto get_impl(const T& t, int) -> decltype(t.get()) {
  return t.get();
}
template <class T>
[[nodiscard]] constexpr auto get_impl(const T& t, ...) -> decltype(auto) {
  return t;
}
template <class T>
[[nodiscard]] constexpr auto get(const T& t) {
  return get_impl(t, 0);
}

template <class T>
struct type_ : op {
  template <class TOther>
  [[nodiscard]] constexpr auto operator()(TOther) const -> const type_<TOther> {
    return {};
  }
  [[nodiscard]] constexpr auto operator==(type_<T>) -> bool { return true; }
  template <class TOther>
  [[nodiscard]] constexpr auto operator==(type_<TOther>) -> bool {
    return false;
  }
  [[nodiscard]] constexpr auto operator!=(type_<T>) -> bool { return true; }
  template <class TOther>
  [[nodiscard]] constexpr auto operator!=(type_<TOther>) -> bool {
    return true;
  }
};

template <class T, class = int>
class value : op {
 public:
  constexpr explicit value(const T& value) : value_(value) {}
  [[nodiscard]] constexpr operator T() const { return value_; }
  [[nodiscard]] constexpr decltype(auto) get() const { return value_; }

 private:
  T value_{};
};

template <class T>
class value<T, type_traits::requires_t<type_traits::is_floating_point_v<T>>>
    : op {
 public:
  static inline auto epsilon = T{};

  constexpr value(const T& value, const T precision) : value_{value} {
    epsilon = precision;
  }

  constexpr explicit value(const T& val)
      : value{val, T(1) / math::pow(10, math::den_size<int>(val))} {}
  [[nodiscard]] constexpr operator T() const { return value_; }
  [[nodiscard]] constexpr decltype(auto) get() const { return value_; }

 private:
  T value_{};
};

template <auto N>
class integral_constant : op {
 public:
  static constexpr auto value = N;

  [[nodiscard]] constexpr auto operator-() const {
    return integral_constant<-N>{};
  }
  [[nodiscard]] constexpr operator decltype(N)() const { return N; }
  [[nodiscard]] constexpr auto get() const { return N; }
};

template <class T, auto N, auto D, auto Size, auto P = 1>
struct floating_point_constant : op {
  static constexpr auto epsilon = T(1) / math::pow(10, Size - 1);
  static constexpr auto value = T(P) * (T(N) + (T(D) / math::pow(10, Size)));

  [[nodiscard]] constexpr auto operator-() const {
    return floating_point_constant<T, N, D, Size, -1>{};
  }
  [[nodiscard]] constexpr operator T() const { return value; }
  [[nodiscard]] constexpr auto get() const { return value; }
};

template <class TLhs, class TRhs>
class eq_ : op {
 public:
  constexpr eq_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs}, value_{[&] {
#if (__GNUC__ < 10)
          using std::operator==;
          using std::operator<;
#endif
          if constexpr (type_traits::has_value_v<TLhs> and
                        type_traits::has_value_v<TRhs>) {
            return TLhs::value == TRhs::value;
          } else if constexpr (type_traits::has_epsilon_v<TLhs> and
                               type_traits::has_epsilon_v<TRhs>) {
            return math::abs(get(lhs) - get(rhs)) <
                   math::min_value(TLhs::epsilon, TRhs::epsilon);
          } else if constexpr (type_traits::has_epsilon_v<TLhs>) {
            return math::abs(get(lhs) - get(rhs)) < TLhs::epsilon;
          } else if constexpr (type_traits::has_epsilon_v<TRhs>) {
            return math::abs(get(lhs) - get(rhs)) < TRhs::epsilon;
          } else {
            return get(lhs) == get(rhs);
          }
        }()} {
  }

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class neq_ : op {
 public:
  constexpr neq_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs}, value_{[&] {
#if (__GNUC__ < 10)
          using std::operator!=;
          using std::operator>;
#endif
          if constexpr (type_traits::has_value_v<TLhs> and
                        type_traits::has_value_v<TRhs>) {
            return TLhs::value != TRhs::value;
          } else if constexpr (type_traits::has_epsilon_v<TLhs> and
                               type_traits::has_epsilon_v<TRhs>) {
            return math::abs(get(lhs_) - get(rhs_)) >
                   math::min_value(TLhs::epsilon, TRhs::epsilon);
          } else if constexpr (type_traits::has_epsilon_v<TLhs>) {
            return math::abs(get(lhs_) - get(rhs_)) > TLhs::epsilon;
          } else if constexpr (type_traits::has_epsilon_v<TRhs>) {
            return math::abs(get(lhs_) - get(rhs_)) > TRhs::epsilon;
          } else {
            return get(lhs_) != get(rhs_);
          }
        }()} {
  }

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class gt_ : op {
 public:
  constexpr gt_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs}, value_{[&] {
#if (__GNUC__ < 10)
          using std::operator>;
#endif
          if constexpr (type_traits::has_value_v<TLhs> and
                        type_traits::has_value_v<TRhs>) {
            return TLhs::value > TRhs::value;
          } else {
            return get(lhs_) > get(rhs_);
          }
        }()} {
  }

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class ge_ : op {
 public:
  constexpr ge_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs}, value_{[&] {
#if (__GNUC__ < 10)
          using std::operator>=;
#endif
          if constexpr (type_traits::has_value_v<TLhs> and
                        type_traits::has_value_v<TRhs>) {
            return TLhs::value >= TRhs::value;
          } else {
            return get(lhs_) >= get(rhs_);
          }
        }()} {
  }

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class lt_ : op {
 public:
  constexpr lt_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs}, value_{[&] {
#if (__GNUC__ < 10)
          using std::operator<;
#endif
          if constexpr (type_traits::has_value_v<TLhs> and
                        type_traits::has_value_v<TRhs>) {
            return TLhs::value < TRhs::value;
          } else {
            return get(lhs_) < get(rhs_);
          }
        }()} {
  }

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class le_ : op {
 public:
  constexpr le_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs}, rhs_{rhs}, value_{[&] {
#if (__GNUC__ < 10)
          using std::operator<=;
#endif
          if constexpr (type_traits::has_value_v<TLhs> and
                        type_traits::has_value_v<TRhs>) {
            return TLhs::value <= TRhs::value;
          } else {
            return get(lhs_) <= get(rhs_);
          }
        }()} {
  }

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class and_ : op {
 public:
  constexpr and_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs},
        rhs_{rhs},
        value_{static_cast<bool>(lhs) and static_cast<bool>(rhs)} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class TLhs, class TRhs>
class or_ : op {
 public:
  constexpr or_(const TLhs& lhs = {}, const TRhs& rhs = {})
      : lhs_{lhs},
        rhs_{rhs},
        value_{static_cast<bool>(lhs) or static_cast<bool>(rhs)} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto lhs() const { return get(lhs_); }
  [[nodiscard]] constexpr auto rhs() const { return get(rhs_); }

 private:
  const TLhs lhs_{};
  const TRhs rhs_{};
  const bool value_{};
};

template <class T>
class not_ : op {
 public:
  explicit constexpr not_(const T& t = {})
      : t_{t}, value_{not static_cast<bool>(t)} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }
  [[nodiscard]] constexpr auto value() const { return get(t_); }

 private:
  const T t_{};
  const bool value_{};
};

#if defined(__cpp_exceptions)
template <class TExpr, class TException = void>
class throws_ : op {
 public:
  constexpr explicit throws_(const TExpr& expr)
      : value_{[&expr] {
          try {
            expr();
          } catch (const TException&) {
            return true;
          } catch (...) {
            return false;
          }
          return false;
        }()} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }

 private:
  const bool value_{};
};

template <class TExpr>
class throws_<TExpr, void> : op {
 public:
  constexpr explicit throws_(const TExpr& expr)
      : value_{[&expr] {
          try {
            expr();
          } catch (...) {
            return true;
          }
          return false;
        }()} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }

 private:
  const bool value_{};
};

template <class TExpr>
class nothrow_ : op {
 public:
  constexpr explicit nothrow_(const TExpr& expr)
      : value_{[&expr] {
          try {
            expr();
          } catch (...) {
            return false;
          }
          return true;
        }()} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }

 private:
  const bool value_{};
};
#endif

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>) and not defined(BOOST_UT_FORWARD)
template <class TExpr>
class aborts_ : op {
 public:
  constexpr explicit aborts_(const TExpr& expr)
      : value_{[&expr]() -> bool {
          if (const auto pid = fork(); not pid) {
            expr();
            exit(0);
          }
          auto exit_status = 0;
          wait(&exit_status);
          return exit_status;
        }()} {}

  [[nodiscard]] constexpr operator bool() const { return value_; }

 private:
  const bool value_{};
};
#endif
}  // namespace detail

struct none {};

namespace events {
struct test_begin {
  utility::string_view type{};
  utility::string_view name{};
  reflection::source_location location{};
};
template <class Test, class TArg = none>
struct test {
  utility::string_view type{};
  utility::string_view name{};
  reflection::source_location location{};
  TArg arg{};
  Test run{};

  constexpr auto operator()() { run_impl(static_cast<Test&&>(run), arg); }
  constexpr auto operator()() const { run_impl(static_cast<Test&&>(run), arg); }

 private:
  static constexpr auto run_impl(Test test, const none&) { test(); }

  template <class T>
  static constexpr auto run_impl(T test, const TArg& arg)
      -> decltype(test(arg), void()) {
    test(arg);
  }

  template <class T>
  static constexpr auto run_impl(T test, const TArg&)
      -> decltype(test.template operator()<TArg>(), void()) {
    test.template operator()<TArg>();
  }
};
template <class Test, class TArg>
test(utility::string_view, utility::string_view, reflection::source_location,
     TArg, Test)
    ->test<Test, TArg>;
template <class TSuite>
struct suite {
  TSuite run{};
  constexpr auto operator()() { run(); }
  constexpr auto operator()() const { run(); }
};
template <class TSuite>
suite(TSuite)->suite<TSuite>;
struct test_run {
  utility::string_view type{};
  utility::string_view name{};
};
template <class TArg = none>
struct skip {
  utility::string_view type{};
  utility::string_view name{};
  TArg arg{};
};
template <class TArg>
skip(utility::string_view, utility::string_view, TArg)->skip<TArg>;
struct test_skip {
  utility::string_view type{};
  utility::string_view name{};
};
template <class TExpr>
struct assertion {
  TExpr expr{};
  reflection::source_location location{};
};
template <class TExpr>
assertion(TExpr, reflection::source_location)->assertion<TExpr>;
#if defined(BOOST_UT_FORWARD) or defined(BOOST_UT_IMPLEMENTATION)
struct expr {
  bool result;
  utility::function<io::ostream&(io::ostream&)> out;
};
#endif
template <class TExpr>
struct assertion_pass {
  TExpr expr{};
  reflection::source_location location{};
};
template <class TExpr>
assertion_pass(TExpr)->assertion_pass<TExpr>;
template <class TExpr>
struct assertion_fail {
  TExpr expr{};
  reflection::source_location location{};
};
template <class TExpr>
assertion_fail(TExpr)->assertion_fail<TExpr>;
struct test_end {
  utility::string_view type{};
  utility::string_view name{};
};
template <class TMsg>
struct log {
  TMsg msg{};
};
template <class TMsg = utility::string_view>
log(TMsg)->log<TMsg>;
struct fatal_assertion {};
struct exception {
  const char* msg{};
  auto what() const -> const char* { return msg; }
};
struct summary {};
}  // namespace events

#if not defined(BOOST_UT_FORWARD)
struct colors {
  std::string_view none = "\033[0m";
  std::string_view pass = "\033[32m";
  std::string_view fail = "\033[31m";
};

class printer {
  [[nodiscard]] inline auto color(const bool cond) {
    const std::string_view colors[] = {colors_.fail, colors_.pass};
    return colors[cond];
  }

 public:
  printer() = default;
  /*explicit(false)*/ printer(const colors colors) : colors_{colors} {}

  template <class T>
  auto& operator<<(const T& t) {
    out_ << t;
    return *this;
  }

  template <class T,
            type_traits::requires_t<type_traits::is_container_v<T> and
                                    not type_traits::has_npos_v<T>> = 0>
  auto& operator<<(T&& t) {
    *this << '{';
    auto first = true;
    for (const auto& arg : t) {
      *this << (first ? "" : ", ") << arg;
      first = false;
    }
    *this << '}';
    return *this;
  }

  auto& operator<<(utility::string_view sv) {
    out_ << sv;
    return *this;
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::eq_<TLhs, TRhs>& op) {
    return (*this << color(op) << op.lhs() << " == " << op.rhs()
                  << colors_.none);
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::neq_<TLhs, TRhs>& op) {
    return (*this << color(op) << op.lhs() << " != " << op.rhs()
                  << colors_.none);
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::gt_<TLhs, TRhs>& op) {
    return (*this << color(op) << op.lhs() << " > " << op.rhs()
                  << colors_.none);
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::ge_<TLhs, TRhs>& op) {
    return (*this << color(op) << op.lhs() << " >= " << op.rhs()
                  << colors_.none);
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::lt_<TRhs, TLhs>& op) {
    return (*this << color(op) << op.lhs() << " < " << op.rhs()
                  << colors_.none);
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::le_<TRhs, TLhs>& op) {
    return (*this << color(op) << op.lhs() << " <= " << op.rhs()
                  << colors_.none);
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::and_<TLhs, TRhs>& op) {
    return (*this << '(' << op.lhs() << color(op) << " and " << colors_.none
                  << op.rhs() << ')');
  }

  template <class TLhs, class TRhs>
  auto& operator<<(const detail::or_<TLhs, TRhs>& op) {
    return (*this << '(' << op.lhs() << color(op) << " or " << colors_.none
                  << op.rhs() << ')');
  }

  template <class T>
  auto& operator<<(const detail::not_<T>& op) {
    return (*this << color(op) << "not " << op.value() << colors_.none);
  }

#if defined(__cpp_exceptions)
  template <class TExpr, class TException>
  auto& operator<<(const detail::throws_<TExpr, TException>& op) {
    return (*this << color(op) << "throws<"
                  << std::string_view{reflection::type_name<TException>()}
                  << ">" << colors_.none);
  }

  template <class TExpr>
  auto& operator<<(const detail::throws_<TExpr, void>& op) {
    return (*this << color(op) << "throws" << colors_.none);
  }

  template <class TExpr>
  auto& operator<<(const detail::nothrow_<TExpr>& op) {
    return (*this << color(op) << "nothrow" << colors_.none);
  }
#endif

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>) and not defined(BOOST_UT_FORWARD)
  template <class TExpr>
  auto& operator<<(const detail::aborts_<TExpr>& op) {
    return (*this << color(op) << "aborts" << colors_.none);
  }
#endif

  template <class T>
  auto& operator<<(const detail::type_<T>&) {
    return (*this << std::string_view{reflection::type_name<T>()});
  }

#if defined(BOOST_UT_FORWARD) or defined(BOOST_UT_IMPLEMENTATION)
  template <class TExpr>
  auto& operator<<(utility::function<TExpr>& expr) {
    void(expr(reinterpret_cast<io::ostream&>(out_)));
    return *this;
  }
#endif
  auto str() const { return out_.str(); }
  const auto& colors() const { return colors_; }

 private:
  ut::colors colors_{};
  std::stringstream out_{};
};

template <class TPrinter = printer>
class reporter {
 public:
  constexpr auto operator=(TPrinter printer) {
    printer_ = static_cast<TPrinter&&>(printer);
  }

  auto on(events::test_begin test_begin) -> void {
    printer_ << "Running \"" << test_begin.name << "\"...";
    fails_ = asserts_.fail;
    exception_ = false;
  }

  auto on(events::test_run test_run) -> void {
    printer_ << "\n \"" << test_run.name << "\"...";
  }

  auto on(events::test_skip test_skip) -> void {
    printer_ << test_skip.name << "...SKIPPED\n";
    ++tests_.skip;
  }

  auto on(events::test_end) -> void {
    if (asserts_.fail > fails_ or exception_) {
      ++tests_.fail;
      printer_ << '\n'
               << printer_.colors().fail << "FAILED" << printer_.colors().none
               << '\n';
    } else {
      ++tests_.pass;
      printer_ << printer_.colors().pass << "PASSED" << printer_.colors().none
               << '\n';
    }
  }

  template <class TMsg>
  auto on(events::log<TMsg> l) -> void {
    printer_ << l.msg;
  }

  auto on(events::exception exception) -> void {
    exception_ = true;
    printer_ << "\n  " << printer_.colors().fail
             << "Unexpected exception with message:\n"
             << exception.what() << printer_.colors().none;
    ++tests_.except;
  }

  template <class TExpr>
  auto on(events::assertion_pass<TExpr>) -> void {
    ++asserts_.pass;
  }

  template <class TExpr>
  auto on(events::assertion_fail<TExpr> assertion) -> void {
    constexpr auto short_name = [](std::string_view name) {
      return name.rfind('/') != std::string_view::npos
                 ? name.substr(name.rfind('/') + 1)
                 : name;
    };
    printer_ << "\n  " << short_name(assertion.location.file_name()) << ':'
             << assertion.location.line() << ':' << printer_.colors().fail
             << "FAILED" << printer_.colors().none << " [" << std::boolalpha
             << assertion.expr << printer_.colors().none << ']';
    ++asserts_.fail;
  }

  auto on(events::fatal_assertion) -> void {}

  auto on(events::summary) -> void {
    if (static auto once = true; once) {
      once = false;
      if (tests_.fail or asserts_.fail) {
        printer_
            << "\n============================================================"
               "=="
               "=================\n"
            << "tests:   " << (tests_.pass + tests_.fail) << " | "
            << printer_.colors().fail << tests_.fail << " failed"
            << printer_.colors().none << '\n'
            << "asserts: " << (asserts_.pass + asserts_.fail) << " | "
            << asserts_.pass << " passed"
            << " | " << printer_.colors().fail << asserts_.fail << " failed"
            << printer_.colors().none << '\n';
        std::cerr << printer_.str() << std::endl;
      } else {
        std::cout << printer_.colors().pass << "All tests passed"
                  << printer_.colors().none << " (" << asserts_.pass
                  << " asserts in " << tests_.pass << " tests)\n";

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
    std::size_t except{};
  } tests_{};

  struct {
    std::size_t pass{};
    std::size_t fail{};
  } asserts_{};

  std::size_t fails_{};
  bool exception_{};

  TPrinter printer_{};
};

struct options {
  std::string_view filter{};
  ut::colors colors{};
  bool dry_run{};
};

struct run_cfg {
  bool report_errors{false};
};

template <class TReporter = reporter<printer>, auto MaxPathSize = 16>
class runner {
  class filter {
    static constexpr auto delim = ".";

   public:
    constexpr /*explicit(false)*/ filter(std::string_view filter = {})
        : path_{utility::split(filter, delim)} {}

    template <class TPath>
    constexpr auto operator()(const std::size_t level, const TPath& path) const
        -> bool {
      for (auto i = 0u; i < math::min_value(level + 1, std::size(path_)); ++i) {
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
    reporter_ = {options.colors};
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
        reporter_.on(events::test_begin{
            .type = test.type, .name = test.name, .location = test.location});
      } else {
        reporter_.on(events::test_run{.type = test.type, .name = test.name});
      }

      if (dry_run_) {
        for (auto i = 0u; i < level_; ++i) {
          std::cout << (i ? "." : "") << path_[i];
        }
        std::cout << '\n';
      }

      active_exception_ = false;
#if defined(__cpp_exceptions)
      try {
#endif
        test();
#if defined(__cpp_exceptions)
      } catch (const events::fatal_assertion&) {
      } catch (const std::exception& e) {
        reporter_.on(events::exception{e.what()});
        active_exception_ = true;
        ++fails_;
      } catch (...) {
        reporter_.on(events::exception{"Unknown exception"});
        active_exception_ = true;
        ++fails_;
      }
#endif

      if (not--level_) {
        reporter_.on(events::test_end{.type = test.type, .name = test.name});
      }
    }
  }

  template <class... Ts>
  auto on(events::skip<Ts...> test) {
    reporter_.on(events::test_skip{.type = test.type, .name = test.name});
  }

  template <class TExpr>
  [[nodiscard]] auto on(events::assertion<TExpr> assertion) -> bool {
    if (dry_run_) {
      return true;
    }

    if (static_cast<bool>(assertion.expr)) {
      reporter_.on(events::assertion_pass<TExpr>{
          .expr = assertion.expr, .location = assertion.location});
      return true;
    } else {
      ++fails_;
      reporter_.on(events::assertion_fail<TExpr>{
          .expr = assertion.expr, .location = assertion.location});
      return false;
    }
  }

#if defined(BOOST_UT_IMPLEMENTATION)
  [[nodiscard]] auto on(events::assertion<events::expr> assertion) -> bool {
    if (dry_run_) {
      return true;
    }

    if (assertion.expr.result) {
      reporter_.on(events::assertion_pass<decltype(assertion.expr.out)>{
          .expr =
              static_cast<decltype(assertion.expr.out)&&>(assertion.expr.out),
          .location = assertion.location});
      return true;
    } else {
      ++fails_;
      reporter_.on(events::assertion_fail<decltype(assertion.expr.out)>{
          .expr =
              static_cast<decltype(assertion.expr.out)&&>(assertion.expr.out),
          .location = assertion.location});
      return false;
    }
  }
#endif

  auto on(events::fatal_assertion fatal_assertion) {
    reporter_.on(fatal_assertion);

#if defined(__cpp_exceptions)
    if (not level_) {
      reporter_.on(events::summary{});
    }
    throw fatal_assertion;
#else
    if (level_) {
      reporter_.on(events::test_end{});
    }
    reporter_.on(events::summary{});
    std::abort();
#endif
  }

  template <class TMsg>
  auto on(events::log<TMsg> l) {
    reporter_.on(l);
  }

  [[nodiscard]] auto run(run_cfg rc = {}) -> bool {
    run_ = true;
    for (const auto& suite : suites_) {
      suite();
    }
    suites_.clear();

    if (rc.report_errors) {
      reporter_.on(events::summary{});
    }

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
[[maybe_unused]] inline auto cfg = runner<reporter<printer>>{};
#endif

namespace link {
#if defined(BOOST_UT_FORWARD) or defined(BOOST_UT_IMPLEMENTATION)
extern void on(events::suite<void (*)()>);
extern void on(events::test<void (*)()>);
extern void on(events::test<utility::function<void()>>);
extern void on(events::skip<>);
[[nodiscard]] extern auto on(events::assertion<events::expr>) -> bool;
extern void on(events::fatal_assertion);
extern void on(events::log<utility::string_view>);
#endif

#if defined(BOOST_UT_IMPLEMENTATION)
void on(events::suite<void (*)()> suite) { cfg<override>.on(suite); }
void on(events::test<void (*)()> test) { cfg<override>.on(test); }
void on(events::test<utility::function<void()>> test) {
  cfg<override>.on(static_cast<decltype(test)&&>(test));
}
void on(events::skip<> skip) { cfg<override>.on(skip); }
[[nodiscard]] auto on(events::assertion<events::expr> assertion) -> bool {
  return cfg<override>.on(static_cast<decltype(assertion)&&>(assertion));
}
void on(events::fatal_assertion assertion) { cfg<override>.on(assertion); }
void on(events::log<utility::string_view> l) { cfg<override>.on(l); }
#endif
}  // namespace link

namespace detail {
struct skip {};

#if defined(BOOST_UT_FORWARD)
template <class..., class TEvent>
constexpr auto on(TEvent&& event) {
  link::on(static_cast<TEvent&&>(event));
}

template <class..., class Test, class TArg>
auto on(events::test<Test, TArg> test) -> void {
  link::on(events::test<utility::function<void()>, TArg>{
      .type = test.type, .name = test.name, .arg = TArg{}, .run = test.run});
}

template <class..., class TExpr>
[[nodiscard]] auto on(events::assertion<TExpr> assertion) -> bool {
  return link::on(events::assertion<events::expr>{
      .expr = {assertion.expr,
               [assertion](io::ostream& os) -> io::ostream& {
                 return (os << assertion.expr);
               }},
      .location = assertion.location,
  });
}
#else
template <class... Ts, class TEvent>
[[nodiscard]] constexpr decltype(auto) on(TEvent&& event) {
  return ut::cfg<typename type_traits::identity<override, Ts...>::type>.on(
      static_cast<TEvent&&>(event));
}
#endif

template <class Test>
struct test_location {
  template <class T>
  constexpr test_location(T t, const reflection::source_location& sl =
                                   reflection::source_location::current())
      : test{t}, location{sl} {}

  Test test{};
  reflection::source_location location{};
};

struct test {
  utility::string_view type{};
  utility::string_view name{};

  template <class... Ts>
  constexpr auto operator=(test_location<void (*)()> test) {
    on<Ts...>(events::test<void (*)()>{.type = type,
                                       .name = name,
                                       .location = test.location,
                                       .arg = none{},
                                       .run = test.test});
    return test.test;
  }

  template <class Test,
            type_traits::requires_t<
                not type_traits::is_convertible_v<Test, void (*)()>> = 0>
  constexpr auto operator=(Test test) ->
      typename type_traits::identity<Test, decltype(test())>::type {
    on<Test>(events::test<Test>{.type = type,
                                .name = name,
                                .location = {},
                                .arg = {},
                                .run = static_cast<Test&&>(test)});
    return test;
  }

  constexpr auto operator=(void (*test)(utility::string_view)) {
    return test(name);
  }

  template <class Test,
            type_traits::requires_t<not type_traits::is_convertible_v<
                Test, void (*)(utility::string_view)>> = 0>
  constexpr auto operator=(Test test)
      -> decltype(test(type_traits::declval<utility::string_view>())) {
    return test(name);
  }
};

template <class T>
class test_skip {
 public:
  constexpr explicit test_skip(T t) : t_{t} {}

  template <class... Ts>
  constexpr auto operator=(void (*test)()) {
    on<Ts...>(events::skip<>{.type = t_.type, .name = t_.name, .arg = none{}});
    return test;
  }

  template <class Test,
            type_traits::requires_t<
                not type_traits::is_convertible_v<Test, void (*)()>> = 0>
  constexpr auto operator=(Test test) ->
      typename type_traits::identity<Test, decltype(test())>::type {
    on<Test>(events::skip{.type = t_.type, .name = t_.name, .arg = none{}});
    return test;
  }

 private:
  T t_;
};

struct log {
  struct next {
    template <class TMsg>
    auto& operator<<(const TMsg& msg) {
      on<TMsg>(events::log{' '});
      on<TMsg>(events::log{msg});
      return *this;
    }
  };

  template <class TMsg>
  auto operator<<(const TMsg& msg) -> next {
    on<TMsg>(events::log{'\n'});
    on<TMsg>(events::log{msg});
    return next{};
  }
};

struct that_ {
  template <class TLhs>
  class expr {
   public:
    constexpr explicit expr(const TLhs& lhs) : lhs_{lhs} {}

    template <class TRhs>
    [[nodiscard]] constexpr auto operator==(const TRhs& rhs) const {
      return eq_{lhs_, rhs};
    }

    template <class TRhs>
    [[nodiscard]] constexpr auto operator!=(const TRhs& rhs) const {
      return neq_{lhs_, rhs};
    }

    template <class TRhs>
    [[nodiscard]] constexpr auto operator>(const TRhs& rhs) const {
      return gt_{lhs_, rhs};
    }

    template <class TRhs>
    [[nodiscard]] constexpr auto operator>=(const TRhs& rhs) const {
      return ge_{lhs_, rhs};
    }

    template <class TRhs>
    [[nodiscard]] constexpr auto operator<(const TRhs& rhs) const {
      return lt_{lhs_, rhs};
    }

    template <class TRhs>
    [[nodiscard]] constexpr auto operator<=(const TRhs& rhs) const {
      return le_{lhs_, rhs};
    }

   private:
    const TLhs lhs_{};
  };

  template <class TLhs>
  [[nodiscard]] constexpr auto operator%(const TLhs& lhs) const {
    return expr{lhs};
  }
};

template <class T>
class expect_ {
 public:
  constexpr explicit expect_(bool result) : result_{result} {}

  template <class TMsg>
  auto& operator<<(const TMsg& msg) {
    if (not result_) {
      on<T>(events::log{' '});
      on<T>(events::log{msg});
    }
    return *this;
  }

  constexpr auto& operator!() {
    fatal_ = true;
    return *this;
  }

  ~expect_() noexcept(false) {
    if (not result_ and fatal_) {
      on<T>(events::fatal_assertion{});
    }
  }

 private:
  bool result_{}, fatal_{};
};
}  // namespace detail

namespace literals {
[[nodiscard]] constexpr auto operator""_test(const char* name,
                                             decltype(sizeof("")) size) {
  return detail::test{"test", utility::string_view{name, size}};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_i() {
  return detail::integral_constant<math::num<int, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_s() {
  return detail::integral_constant<math::num<short, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_c() {
  return detail::integral_constant<math::num<char, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_l() {
  return detail::integral_constant<math::num<long, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_ll() {
  return detail::integral_constant<math::num<long long, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_u() {
  return detail::integral_constant<math::num<unsigned, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_uc() {
  return detail::integral_constant<math::num<unsigned char, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_us() {
  return detail::integral_constant<math::num<unsigned short, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_ul() {
  return detail::integral_constant<math::num<unsigned long, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_f() {
  return detail::floating_point_constant<
      float, math::num<unsigned long, Cs...>(), math::den<int, Cs...>(),
      math::den_size<int, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_d() {
  return detail::floating_point_constant<
      double, math::num<unsigned long, Cs...>(), math::den<int, Cs...>(),
      math::den_size<int, Cs...>()>{};
}

template <char... Cs>
[[nodiscard]] constexpr auto operator""_ld() {
  return detail::floating_point_constant<
      long double, math::num<unsigned long, Cs...>(), math::den<int, Cs...>(),
      math::den_size<int, Cs...>()>{};
}
}  // namespace literals

namespace type_traits {
template <class T>
constexpr auto is_op_v = __is_base_of(detail::op, T);
}  // namespace type_traits

namespace operators {
#if not defined(BOOST_UT_FORWARD) and \
    (defined(__cpp_lib_string_view) or defined(__APPLE__))
[[nodiscard]] constexpr auto operator==(std::string_view lhs,
                                        std::string_view rhs) {
  return detail::eq_{lhs, rhs};
}

[[nodiscard]] constexpr auto operator!=(std::string_view lhs,
                                        std::string_view rhs) {
  return detail::neq_{lhs, rhs};
}
#endif

template <class T, type_traits::requires_t<type_traits::is_container_v<T>> = 0>
[[nodiscard]] constexpr auto operator==(T&& lhs, T&& rhs) {
  return detail::eq_{static_cast<T&&>(lhs), static_cast<T&&>(rhs)};
}

template <class T, type_traits::requires_t<type_traits::is_container_v<T>> = 0>
[[nodiscard]] constexpr auto operator!=(T&& lhs, T&& rhs) {
  return detail::neq_{static_cast<T&&>(lhs), static_cast<T&&>(rhs)};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator==(const TLhs& lhs, const TRhs& rhs) {
  return detail::eq_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator!=(const TLhs& lhs, const TRhs& rhs) {
  return detail::neq_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator>(const TLhs& lhs, const TRhs& rhs) {
  return detail::gt_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator>=(const TLhs& lhs, const TRhs& rhs) {
  return detail::ge_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator<(const TLhs& lhs, const TRhs& rhs) {
  return detail::lt_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator<=(const TLhs& lhs, const TRhs& rhs) {
  return detail::le_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator and(const TLhs& lhs, const TRhs& rhs) {
  return detail::and_{lhs, rhs};
}

template <class TLhs, class TRhs,
          type_traits::requires_t<type_traits::is_op_v<TLhs> or
                                  type_traits::is_op_v<TRhs>> = 0>
[[nodiscard]] constexpr auto operator or(const TLhs& lhs, const TRhs& rhs) {
  return detail::or_{lhs, rhs};
}

template <class T, type_traits::requires_t<type_traits::is_op_v<T>> = 0>
[[nodiscard]] constexpr auto operator not(const T& t) {
  return detail::not_{t};
}

template <class T>
[[nodiscard]] constexpr auto operator|(const detail::skip&, const T& t) {
  return detail::test_skip{t};
}

template <class F, class T,
          type_traits::requires_t<type_traits::is_container_v<T>> = 0>
[[nodiscard]] constexpr auto operator|(const F& f, const T& t) {
  return [f, t](const auto name) {
    for (const auto& arg : t) {
      detail::on<F>(events::test<F, typename T::value_type>{
          .type = "test", .name = name, .location = {}, .arg = arg, .run = f});
    }
  };
}

template <
    class F, template <class...> class T, class... Ts,
    type_traits::requires_t<not type_traits::is_container_v<T<Ts...>>> = 0>
[[nodiscard]] constexpr auto operator|(const F& f, const T<Ts...>& t) {
  return [f, t](const auto name) {
    apply(
        [f, name](const auto&... args) {
          (detail::on<F>(events::test<F, Ts>{.type = "test",
                                             .name = name,
                                             .location = {},
                                             .arg = args,
                                             .run = f}),
           ...);
        },
        t);
  };
}
}  //  namespace operators

template <class TExpr, type_traits::requires_t<
                           type_traits::is_op_v<TExpr> or
                           type_traits::is_convertible_v<TExpr, bool>> = 0>
constexpr auto expect(const TExpr& expr,
                      const reflection::source_location& sl =
                          reflection::source_location::current()) {
  return detail::expect_<TExpr>{detail::on<TExpr>(
      events::assertion<TExpr>{.expr = expr, .location = sl})};
}

#if defined(__cpp_nontype_template_parameter_class)
template <auto Constant>
#else
template <bool Constant>
#endif
constexpr auto constant = Constant;

#if defined(__cpp_exceptions)
template <class TException, class TExpr>
[[nodiscard]] constexpr auto throws(const TExpr& expr) {
  return detail::throws_<TExpr, TException>{expr};
}

template <class TExpr>
[[nodiscard]] constexpr auto throws(const TExpr& expr) {
  return detail::throws_<TExpr>{expr};
}

template <class TExpr>
[[nodiscard]] constexpr auto nothrow(const TExpr& expr) {
  return detail::nothrow_{expr};
}
#endif

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>) and not defined(BOOST_UT_FORWARD)
template <class TExpr>
[[nodiscard]] constexpr auto aborts(const TExpr& expr) {
  return detail::aborts_{expr};
}
#endif

using _b = detail::value<bool>;
using _c = detail::value<char>;
using _s = detail::value<short>;
using _i = detail::value<int>;
using _l = detail::value<long>;
using _ll = detail::value<long long>;
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
  constexpr /*explicit(false)*/ suite(TSuite suite) {
    static_assert(1 == sizeof(suite));
    detail::on<decltype(+suite)>(
        events::suite<decltype(+suite)>{.run = +suite});
  }
};

[[maybe_unused]] constexpr auto true_b = detail::integral_constant<true>{};
[[maybe_unused]] constexpr auto false_b = detail::integral_constant<false>{};

[[maybe_unused]] inline auto log = detail::log{};
[[maybe_unused]] inline auto that = detail::that_{};
[[maybe_unused]] constexpr auto test = [](const auto name) {
  return detail::test{"test", name};
};
[[maybe_unused]] constexpr auto should = test;
[[maybe_unused]] constexpr auto skip = detail::skip{};
[[maybe_unused]] constexpr auto given = [](const auto name) {
  return detail::test{"given", name};
};
[[maybe_unused]] constexpr auto when = [](const auto name) {
  return detail::test{"when", name};
};
[[maybe_unused]] constexpr auto then = [](const auto name) {
  return detail::test{"then", name};
};
template <class T = void>
[[maybe_unused]] constexpr auto type = detail::type_<T>();

template <class TLhs, class TRhs>
[[nodiscard]] constexpr auto eq(const TLhs& lhs, const TRhs& rhs) {
  return detail::eq_{lhs, rhs};
}
template <class TLhs, class TRhs>
[[nodiscard]] constexpr auto neq(const TLhs& lhs, const TRhs& rhs) {
  return detail::neq_{lhs, rhs};
}
template <class TLhs, class TRhs>
[[nodiscard]] constexpr auto gt(const TLhs& lhs, const TRhs& rhs) {
  return detail::gt_{lhs, rhs};
}
template <class TLhs, class TRhs>
[[nodiscard]] constexpr auto ge(const TLhs& lhs, const TRhs& rhs) {
  return detail::ge_{lhs, rhs};
}
template <class TLhs, class TRhs>
[[nodiscard]] constexpr auto lt(const TLhs& lhs, const TRhs& rhs) {
  return detail::lt_{lhs, rhs};
}
template <class TLhs, class TRhs>
[[nodiscard]] constexpr auto le(const TLhs& lhs, const TRhs& rhs) {
  return detail::le_{lhs, rhs};
}

template <class T>
[[nodiscard]] constexpr auto mut(const T& t) noexcept -> T& {
  return const_cast<T&>(t);
}

using literals::operator""_test;
using literals::operator""_i;
using literals::operator""_s;
using literals::operator""_c;
using literals::operator""_l;
using literals::operator""_ll;
using literals::operator""_u;
using literals::operator""_uc;
using literals::operator""_us;
using literals::operator""_ul;
using literals::operator""_f;
using literals::operator""_d;
using literals::operator""_ld;
using operators::operator==;
using operators::operator!=;
using operators::operator>;
using operators::operator>=;
using operators::operator<;
using operators::operator<=;
using operators::operator and;
using operators::operator or;
using operators::operator not;
using operators::operator|;
}  // namespace v1_1_6
}  // namespace boost::ut
#endif
