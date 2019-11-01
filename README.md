<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">![Boost Licence](http://img.shields.io/badge/license-boost-blue.svg)</a>
<a href="https://github.com/boost-experimental/ut/releases" target="_blank">![Version](https://badge.fury.io/gh/boost-experimental%2Fut.svg)</a>
<a href="https://travis-ci.org/boost-experimental/ut" target="_blank">![Build Status](https://img.shields.io/travis/boost-experimental/ut/master.svg?label=linux/osx)</a>
<a href="https://ci.appveyor.com/project/krzysztof-jusiak/ut" target="_blank">![Build Status](https://img.shields.io/appveyor/ci/krzysztof-jusiak/ut/master.svg?label=windows)</a>
<a href="https://codecov.io/gh/boost-experimental/ut" target="_blank">![Coveralls](https://codecov.io/gh/boost-experimental/ut/branch/master/graph/badge.svg)</a>
<a href="https://www.codacy.com/manual/krzysztof-jusiak/ut?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=boost-experimental/ut&amp;utm_campaign=Badge_Grade" target="_blank">![Codacy Badge](https://api.codacy.com/project/badge/Grade/c0bd979793124a0baf17506f93079aac)</a>
<a href="https://github.com/boost-experimental/ut/issues" target="_blank">![Github Issues](https://img.shields.io/github/issues/boost-experimental/ut.svg)</a>
<a href="https://godbolt.org/z/EtmE8-">![Try it online](https://img.shields.io/badge/try%20it-online-blue.svg)</a>

# [Boost].UT / μt

> C++20 **single header, macro-free** μ(micro)/Unit Testing Framework

* **No dependencies** (C++20 / tested: [GCC-9+, Clang-9.0+](https://travis-ci.org/boost-experimental/ut), [MSVC-2019+*](https://ci.appveyor.com/project/krzysztof-jusiak/ut))
* **Single include** ([boost/ut.hpp](https://github.com/boost-experimental/ut/blob/master/include/boost/ut.hpp))
* **Macro-free** ([Based on modern C++ features](https://en.cppreference.com/w/cpp/compiler_support#cpp2a))
* **Easy to use** (Minimal interface - `test, suite, expect`)
* **Fast to compile/execute** ([Benchmarks](benchmark))
* **Extensible** ([Runners](example/cfg/runner.cpp), [Reporters](example/cfg/reporter.cpp))

<a href="https://godbolt.org/z/izlLC-"><img src="doc/images/ut.png"></a>

### Testing

> "If you liked it then you should have put a test on it", Beyonce rule

### Quick start

> Get the latest latest header [here!](https://github.com/boost-experimental/ut/blob/master/include/boost/ut.hpp)

```cpp
#include <boost/ut.hpp>
```

**Hello World** (https://godbolt.org/z/ZLCQ2n)

```cpp
constexpr auto sum = [](auto... args) { return (0 + ... + args); };
```

```cpp
int main() {
  using namespace boost::ut;

  "hello world"_test = [] {
    expect(0_i == sum());
    expect(sum(1) == 1_i);
    expect(3_i == sum(1, 2));
  };
}
```

```
All tests passed (3 asserts in 1 tests)
```

**Assertions** (https://godbolt.org/z/A-BGn2)

```cpp
"message"_test = [] {
  expect(3_i == sum(1, 2)) << "wrong sum";
};

"operators"_test = [] {
  expect(0_i == sum());
  expect(2_i != sum(1, 2));
  expect(sum(1) >= 0_i);
  expect(sum(1) <= 1_i);
};

"expressions"_test = [] {
  expect(0_i == sum() and 42_i == sum(40, 2));
  expect(0_i == sum() or 1_i == sum()) << "compound";
};

"floating points"_test = [] {
  expect(42.1_d == 42.101) << "epsilon=0.1";
  expect(42.10_d == 42.101) << "epsilon=0.01";
  expect(42.10000001 == 42.1_d) << "epsilon=0.1";
};

"constant"_test = [] {
  constexpr auto compile_time_v = 42;
  auto run_time_v = 99;
  expect(constant<42_i == compile_time_v> and run_time_v == 99_i);
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

```
Running "message"...OK
Running "operators"...OK
Running "expressions"...OK
Running "floating points"...OK
Running "constant"...OK
Running "fatal"...OK
Running "failure"...
  assertions.cpp:48:FAILED [1 == 2] should fail
  assertions.cpp:49:FAILED [(0 == 1 or 2 == 0)] sum?
FAILED

===============================================================================

tests:   6  | 1 failed
asserts: 16 | 14 passed | 2 failed
```

**Sections** (https://godbolt.org/z/Q4iXBE)

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

```
All tests passed (4 asserts in 1 tests)
```

**Exceptions** (https://godbolt.org/z/3BdTtA)

```cpp
"exceptions"_test = [] {
  expect(throws<std::runtime_error>([]{throw std::runtime_error{""};}))
    << "throws runtime_error";
  expect(throws([]{throw 0;})) << "throws any exception";
  expect(nothrow([]{})) << "doesn't throw";
};
```

```
All tests passed (3 asserts in 1 tests)
```

**Parameterized** (https://godbolt.org/z/N_4CIF)

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

```
All tests passed (11 asserts in 7 tests)
```

**Logging** (https://godbolt.org/z/WLVmwd)

```cpp
"logging"_test = [] {
  log << "pre";
  expect(42_i == 43) << "message on failure";
  log << "post";
};
```

```
Running "logging"...
pre
  logging.cpp:8:FAILED [42 == 43] message on failure
post
FAILED

===============================================================================

tests:   1 | 1 failed
asserts: 1 | 0 passed | 1 failed
```

**Behavior Driven Development** (https://godbolt.org/z/A1Lkec)

```cpp
"scenario"_test = [] {
  given("I have...") = [] {
    when("I run...") = [] {
      then("I expect...") = [] { expect(1_i == 1); };
      then("I expect...") = [] { expect(1 == 1_i); };
    };
  };
};
```

```
All tests passed (2 asserts in 1 tests)
```

**Test Suites** (https://godbolt.org/z/1pPYlN)

```cpp
namespace ut = boost::ut;

ut::suite _ = [] {
  using namespace ut;

  "equality/exceptions"_test = [] {
    "should equal"_test = [] {
      expect(42_i == 42);
    };

    "should throw"_test = [] {
      expect(throws([]{throw 0;}));
    };
  };
};

int main() { }
```

```
All tests passed (2 asserts in 1 tests)
```

**Skipping tests** (https://godbolt.org/z/FeWVjB)

```cpp
skip | "don't run"_test = [] {
  expect(42_i == 43) << "should not fire!";
};
```

```
All tests passed (0 asserts in 0 tests)
1 tests skipped
```

**Runner** (https://godbolt.org/z/0uaFlH)

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
   * @param test() execute test
   */
  template<class... Ts>
  auto on(ut::events::test<Ts...>) { }

  /**
   * @example skip | "don't run"_test = []{};
   * @param skip.type ["test", "given", "when", "then"]
   * @param skip.name "don't run"
   * @param skip.arg parametrized argument
   */
  template<class... Ts>
  auto on(ut::events::skip<Ts...>) { }

  /**
   * @example file.cpp:42: expect(42_i == 42);
   * @param assertion.location { "file.cpp", 42 }
   * @param assertion.expr 42_i == 42
   * @return true if expr passes, false otherwise
   */
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion<TLocation, TExpr>) -> bool {
    return {};
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
```

**Reporter** (https://godbolt.org/z/COV5xu)

```cpp
namespace ut = boost::ut;

namespace cfg {
class reporter {
 public:
  /**
   * @example "name"_test = [] {};
   * @param test_begin.type ["test", "given", "when", "then"]
   * @param test_begin.name "name"
   */
  auto on(ut::events::test_begin) -> void {}

  /**
   * @example "name"_test = [] {};
   * @param test_run.type ["test", "given", "when", "then"]
   * @param test_run.name "name"
   */
  auto on(ut::events::test_run) -> void {}

  /**
   * @example "name"_test = [] {};
   * @param test_skip.type ["test", "given", "when", "then"]
   * @param test_skip.name "name"
   */
  auto on(ut::events::test_skip) -> void {}

  /**
   * @example "name"_test = [] {};
   * @param test_end.type ["test", "given", "when", "then"]
   * @param test_end.name "name"
   */
  auto on(ut::events::test_end) -> void {}

  /**
   * @example log << "message"
   * @param log.msg "message"
   */
  auto on(ut::events::log) -> void {}

  /**
   * @example file.cpp:42: expect(42_i == 42);
   * @param assertion_pass.location { "file.cpp", 42 }
   * @param assertion_pass.expr 42_i == 42
   */
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion_pass<TLocation, TExpr>) -> void {}

  /**
   * @example file.cpp:42: expect(42_i != 42);
   * @param assertion_fail.location { "file.cpp", 42 }
   * @param assertion_fail.expr 42_i != 42
   */
  template <class TLocation, class TExpr>
  auto on(ut::events::assertion_fail<TLocation, TExpr>) -> void {}

  /**
   * @example !expect(2_i == 1)
   * @note triggered by `!expect`
   *       should std::exit
   */
  auto on(ut::events::fatal_assertion) -> void {}

  /**
   * @example "exception"_test = [] { throw std::runtime_error{""}; };
   */
  auto on(ut::events::exception) -> void {}

  /**
   * @note triggered on destruction of runner
   */
  auto on(ut::events::summary) -> void{};
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<cfg::reporter>{};
```

More examples [here!](example)

---

**Disclaimer** `[Boost].UT` is not an official Boost library.
