<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-experimental/ut/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-experimental%2Fut.svg)</a>
<a href="https://travis-ci.org/boost-experimental/ut" target="_blank">![Build Status](https://img.shields.io/travis/boost-experimental/ut/master.svg?label=linux/osx)</a>
<a href="https://ci.appveyor.com/project/krzysztof-jusiak/ut" target="_blank">![Build Status](https://img.shields.io/appveyor/ci/krzysztof-jusiak/ut/master.svg?label=windows)</a>
<a href="https://codecov.io/gh/boost-experimental/ut" target="_blank">![Coveralls](https://codecov.io/gh/boost-experimental/ut/branch/master/graph/badge.svg)</a>
<a href="https://github.com/boost-experimental/ut/issues" target="_blank">![Github Issues](https://img.shields.io/github/issues/boost-experimental/ut.svg)</a>
<a href="https://godbolt.org/z/EtmE8-">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# [Boost].UT

> C++20 **single header, macro-free** μ(micro)/unit test framework

* **No dependencies** (C++20 / tested: GCC-9+, Clang-9.0+, MSVC-2019+*)
* **Single include** ([boost/ut.hpp](https://github.com/boost-experimental/ut/blob/master/include/boost/ut.hpp))
* **Macro-free** ([Based on modern C++ features](https://en.cppreference.com/w/cpp/compiler_support#cpp2a))
* **Easy to use** (Minimal interface - `""_test, expect`)
* **Fast to compile/execute** ([Benchmarks](benchmark))
* **Extensible** ([Runners](example/cfg/runner.cpp), [Reporters](example/cfg/reporter.cpp))

### Testing

> "If you liked it then you should have put a test on it", Beyonce rule

### Quick start

> Get the latest latest header [here!](https://github.com/boost-experimental/ut/blob/master/include/boost/ut.hpp)

```cpp
#include <boost/ut.hpp>
```

#### Hello World

```cpp
constexpr auto sum = [](auto... args) { return (0 + ... + args); };
```

```cpp
int main() {
  using namespace boost::ut;

  "hello world"_test = [] {
    expect(0_i == sum());
    expect(1_i == sum(1));
    expect(3_i == sum(1, 2));
  };
}
```

```sh
BOOST_UT_FILTER="hello world" ./hello_world
```

```
All tests passed (3 asserts in 1 tests)
```

#### Assertions

```cpp
"operators"_test = [] {
  expect(0_i == sum());
  expect(2_i != sum(1, 2));
  expect(sum(1) >= 0_i);
  expect(sum(1) <= 1_i);
};

"message"_test = [] {
  expect(3_i == sum(1, 2)) << "wrong sum";
};

"expressions"_test = [] {
  expect(0_i == sum() and 42_i == sum(40, 2));
  expect(0_i == sum() or 1_i == sum()) << "complex";
};

"floating points"_test = [] {
  expect(42.1_d == 42.101) << "epsilon=0.1";
  expect(42.10_d == 42.101) << "epsilon=0.01";
  expect(42.10000001 == 42.1_d) << "epsilon=0.1";
};

"fatal"_test = [] {
  std::vector v{1, 2, 3};
  !expect(std::size(v) == 3_ul) << "fatal assertion";
   expect(v[0] == 1_i);
   expect(v[1] == 2_i);
   expect(v[2] == 3_i);
};

"failure"_test = [] {
  expect(1_i == 2) << "should fail";
  expect(sum() == 1_i or 2_i == sum()) << "sum?";
};
```

```sh
./assertions
```

```
Running "static"...OK
Running "operators"...OK
Running "expressions"...OK
Running "message"...OK
Running "floating points"...OK
Running "fatal"...OK
Running "failure"...
  assertions.cpp:46:FAILED [1 == 2] should fail
  assertions.cpp:47:FAILED [0 == 1 or 2 == 0] sum?
FAILED

===============================================================================
tests:   7  | 1 failed
asserts: 13 | 12 passed | 1 failed
```

#### Sections

```cpp
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
```

#### Exceptions

```cpp
"exceptions"_test = [] {
  expect(throws<std::runtime_error>([]{throw std::runtime_error{""};}))
    << "throws runtime_error";
  expect(throws([]{throw 0;})) << "throws any exception";
  expect(nothrow([]{})) << "doesn't throw";
};
```

#### Parameterized

```cpp
"args"_test =
   [](const auto& arg) {
      expect(arg >= 1_i);
    }
  | std::vector{1, 2, 3};

"types"_test =
    []<class T> {
      expect(std::is_integral_v<T>) << "all types are integrals";
    }
  | std::tuple<bool, int>{};

"args and types"_test =
    []<class TArg>(const TArg& arg) {
      !expect(std::is_integral_v<TArg>);
       expect(42_i == arg or true_b == arg);
       expect(type<TArg> == type<int> or type<TArg> == type<bool>);
    }
  | std::tuple{true, 42};
```

#### Logging

```cpp
"logging"_test = [] {
  log << "pre";
  expect(42_i == 42) << "message on failure";
  log << "post";
};
```

#### Behavior Driven Development

```cpp
"scenario"_test = [] {
  given("I have...") = [] {
    when("I run...") = [] {
      then("I expect...") = [] { expect(1_u == 1u); };
      then("I expect...") = [] { expect(1u == 1_u); };
    };
  };
};
```

#### Test Suites

```cpp
namespace ut = boost::ut;

using ut::operator""_test;

auto _ = "test suite"_test = [] {
  using namespace ut;

  "should equal"_test = [] {
    expect(42_i == 42);
  };

  "should throw"_test = [] {
    expect(throws([]{throw 0;}));
  };
};

int main() { }
```

#### Skipping tests

```cpp
skip | "don't run"_test = [] {
  expect(42_i == 43) << "should not fire!";
};
```

#### Reporter

```cpp
namespace ut = boost::ut;

namespace cfg {
class reporter {
 public:
  auto on(ut::events::test_begin) -> void {}
  auto on(ut::events::test_run) -> void {}
  auto on(ut::events::test_skip) -> void {}
  auto on(ut::events::test_end) -> void {}
  auto on(ut::events::log) -> void {}
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion_pass<TLocation, TExpr>) -> void {}
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion_fail<TLocation, TExpr>) -> void {}
  auto on(ut::events::fatal_assertion) -> void {}
  auto on(ut::events::exception) -> void {}
  auto on(ut::events::summary) -> void{};
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<cfg::reporter>{};

int main() {
  using namespace ut;
  "example"_test = [] { expect(42 == 42_i); };
}
```

#### Runner

```cpp
namespace ut = boost::ut;

namespace cfg {
class runner {
 public:
  /**
   * @example "name"_test = [] {};
   * @param test.type ["test", "given", "when", "then"]
   * @param test.name "name"
   * @param test.arg parametrized argument
   * @param test.test function
   */
  template<class... Ts>
  auto on(ut::events::run<Ts...> test) {
    test.test(); // execute test
  }

  /**
   * @example skip | "don't run"_test = []{};
   * @param test.type ["test", "given", "when", "then"]
   * @param test.name "don't run"
   * @param test.arg parametrized argument
   * @param test.test function
   */
  template<class... Ts>
  auto on(ut::events::skip<Ts...> test) { }

  /**
   * @example file.cpp:42: expect(42_i == 42);
   * @param assertion.location { file.cpp, 42 }
   * @param assertion.expr 42_i == 42
   * @return expression evaluation
   */
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr> assertion) -> bool {
    return true; // all assertsions are valid
  }

  /**
   * @example !expect(2_i == 1)
   * @note triggered by `!expect`
   *       should std::exit
   */
  auto on(ut::events::fatal_assertion) { }

  /**
   * @example log << "message"
   * @param log.msg "message"
   */
  auto on(ut::events::log) { }
};
} // namespace cfg

template<> auto ut::cfg<ut::override> = cfg::runner{};

int main() {
  using namespace ut;

  "silent"_test = [] {
    expect(1_i == 2) << "ignored by the config";
  };
}
```

---

**Disclaimer** `[Boost].UT` is not an official Boost library.
