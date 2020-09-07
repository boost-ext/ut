//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#if defined(__cpp_modules)
export module ut;
export import std;
namespace std::experimental {
class source_location {
public:
  [[nodiscard]] static constexpr auto current(
#if (__has_builtin(__builtin_FILE) and __has_builtin(__builtin_LINE))
      const char *file = __builtin_FILE(), int line = __builtin_LINE()
#else
      const char *file = "unknown", int line = {}
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
  const char *file_{"unknown"};
  int line_{};
};
} // namespace std::experimental
#else
#pragma once
#include <experimental/source_location>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
#endif

#if __has_include(<concepts>)
#include <concepts>
#else
namespace std {
template <class... Ts> concept same_as = std::is_same_v<Ts...>;
template <class F, class... TArgs>
concept invocable = requires(F f, TArgs... args) {
  f(args...);
};
template <class TFrom, class To>
concept convertible_to = std::is_convertible_v<TFrom, To>;
} // namespace std
#endif

#if defined(__cpp_modules)
export namespace ut::inline v1 {
#else
namespace ut::inline v1 {
#endif

namespace concepts {
/**
 * Streamable concept
 */
template <class T, class... TArgs>
concept streamable = requires(T t, TArgs... args) {
  ((t << args), ...);
};

/**
 * Printable concept
 */
template <class T> concept printable = requires(std::ostream &os, T t) {
  os << t;
};

/**
 * Test concept
 */
template <class T, auto expr = +[] {}> concept test = requires(T test) {
  { test.name }
  ->printable;
  { test = expr }
  ->std::same_as<void>;
};

/**
 * Suite concept
 */
template <class TSuite> concept suite = std::invocable<TSuite>;

/**
 * Expression concept
 */
template <class TExpr>
concept expression = std::convertible_to<TExpr, bool> and printable<TExpr>;

/**
 * Operator concept
 */
template <class TOp> concept op = requires(TOp op) {
  requires requires {
    op.lhs;
    op.rhs;
    requires std::convertible_to<TOp, bool>;
  }
  or requires {
    typename TOp::value_type;
    op.get();
  };
  requires printable<TOp>;
};
} // namespace concepts

namespace detail {
/**
 * Helper to get underlying value
 * @param op operator
 * @return underlying value
 */
constexpr auto get(auto op) {
  if constexpr (requires { op.get(); }) {
    return op.get();
  } else {
    return op;
  }
}

/**
 * Helper to print failed expression and its location
 *
 * @param expr expression
 * @param location location
 */
constexpr auto error(const auto &expr, const auto &location) -> void {
  std::cerr << location.file_name() << ':' << location.line()
            << ":FAILED: " << expr << '\n';
}

template <auto Value> struct value {
  using value_type = decltype(Value);

  /**
   * Value getter
   * @return Value
   */
  constexpr auto get() const { return Value; }

  /**
   * Nicely prints the operation
   */
  friend auto &operator<<(auto &os, const value &) { return (os << Value); }
};

template <class TValue> struct value_location {
  constexpr explicit(false)
      value_location(TValue value,
                     const std::experimental::source_location &location =
                         std::experimental::source_location::current())
      : value_{value}, location_{location} {}

  constexpr auto get() const { return value_; }
  constexpr auto location() const { return location_; }

private:
  TValue value_{};
  std::experimental::source_location location_{};
};

/**
 * Comparison Operator
 */
template <class TLhs, class TRhs> struct eq {
  TLhs lhs{}; // Left-hand side operator
  TRhs rhs{}; // Right-hand side operator

  /**
   * Performs comparison operation
   * @return true if expression is successful
   */
  [[nodiscard]] constexpr explicit(false) operator bool() const {
    return get(lhs) == get(rhs);
  }

  /**
   * Nicely prints the operation
   */
  friend auto &operator<<(auto &os, const eq &op) {
    return (os << get(op.lhs) << " == " << get(op.rhs));
  }
};
template <class TLhs, class TRhs> eq(TLhs, TRhs) -> eq<TLhs, TRhs>;
} // namespace detail

namespace literals {
/**
 * Creates constant object for which operators can be overloaded
 * @example 42_i
 * @return compile-time value object
 */
template <char... Cs>[[nodiscard]] constexpr auto operator""_i() {
  constexpr auto pow = []<class TBase, class TExp>(TBase base, TExp exp) {
    std::common_type_t<TBase, TExp> result{1};
    for (TExp i{}; i < exp; ++i) {
      result *= base;
    }
    return result;
  };
  return [pow]<auto... Ns>(std::index_sequence<Ns...>) {
    return detail::value<(int(pow(10, sizeof...(Ns) - Ns - 1) * (Cs - '0')) +
                          ...)>{};
  }
  (std::make_index_sequence<sizeof...(Cs)>{});
}
} // namespace literals

namespace operators {
/**
 * Comparison overload if at least one of {lhs, rhs} is an operator
 * @example (42_i == 42)
 * @param lhs Left-hand side operator
 * @param rhs Right-hand side operator
 * @return Comparison object
 */
template <class TLhs, class TRhs>
    requires concepts::op<TLhs> or
    concepts::op<TRhs>[[nodiscard]] constexpr concepts::op auto
    operator==(TLhs lhs, TRhs rhs) {
  return detail::eq{lhs, rhs};
}

namespace terse {

/**
 * Comparison overload for terse syntax
 * @example (42_i == 42)
 * @param lhs Left-hand side operator
 * @param rhs Right-hand side operator
 * @return Comparison object which check the expression on destruction
 */
template <concepts::op T>
constexpr concepts::op auto
operator==(detail::value_location<typename T::value_type> lhs, T rhs) {
  using eq_t = detail::eq<decltype(lhs), decltype(rhs)>;
  struct eq : eq_t {
    ~eq() noexcept {
      if (not *this) {
        detail::error(*this, eq_t::lhs.location());
      }
    }
  };
  return eq{{lhs, rhs}};
}

template <concepts::op T>
constexpr concepts::op auto
operator==(T lhs, detail::value_location<typename T::value_type> rhs) {
  using eq_t = detail::eq<decltype(lhs), decltype(rhs)>;
  struct eq : eq_t {
    ~eq() noexcept {
      if (not *this) {
        detail::error(*this, eq_t::rhs.location());
      }
    }
  };
  return eq{{lhs, rhs}};
}
} // namespace terse
} // namespace operators

/**
 * Evaluates an expression
 * @example expect(42_i == 42);
 * @param expr expression to be evaluated
 * @param location source code location
 * @return streamable
 */
constexpr concepts::streamable auto &
expect(concepts::expression auto expr,
       const std::experimental::source_location &location =
           std::experimental::source_location::current()) {
  if (not expr) {
    error(expr, location);
  }
  return std::cerr;
}

/**
 * Represents suite object
 */
struct suite final {
  /**
   * Assigns and executes test suite
   */
  [[nodiscard]] constexpr explicit(false) suite(concepts::suite auto suite) {
    suite();
  }
};

/**
 * Represents test object
 */
struct test final {
  std::string_view name{}; /// test case name

  /**
   * Assigns and executes test function
   * @param test function
   */
  auto operator=(std::invocable auto test)
      -> void requires(not std::convertible_to<decltype(test), void (*)()>) {
    std::clog << "\nRunning... " << name << '\n';
    test();
  }

  /**
   * Assigns and executes test function
   * with faster compilation times duo to type-erased invocable
   * @param test function
   */
  auto operator=(void (*test)()) -> void {
    std::clog << "\nRunning... " << name << '\n';
    test();
  }
};

/**
 * Creates named test object
 * @example "hello world"_test
 * @return test object
 */
[[nodiscard]] constexpr concepts::test auto operator""_test(const char *name,
                                                            std::size_t size) {
  return test{.name = {name, size}};
}

/**
 * Convenient alias for creating test sections
 * @example should("return true") = []{};
 */
constexpr inline auto should = [](const auto name) -> concepts::test auto {
  return test{name};
};

/**
 * Convenient aliases for creating BDD tests
 * @example given("I have an object") = []{};
 * @example when("I call it") = []{};
 * @example then("I should get") = []{};
 */
namespace bdd {
constexpr inline auto given = [](const auto name) -> concepts::test auto {
  return test{name};
};
constexpr inline auto when = [](const auto name) -> concepts::test auto {
  return test{name};
};
constexpr inline auto then = [](const auto name) -> concepts::test auto {
  return test{name};
};
} // namespace bdd

/**
 * Convenient aliases for creating Spec tests
 * @example describe("test") = [] { it("should") = []{}; };
 */
namespace spec {
constexpr inline auto describe = [](const auto name) -> concepts::test auto {
  return test{name};
};
constexpr inline auto it = [](const auto name) -> concepts::test auto {
  return test{name};
};
} // namespace spec
} // namespace ut::v1
