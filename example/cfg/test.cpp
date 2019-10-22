//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <functional>
#include <vector>

namespace ut = boost::ut;

class test_cfg : ut::default_cfg {
  using ut::default_cfg::level_;

 public:
  template <class... Ts>
  auto on(ut::events::test_run<Ts...> test) {
    const auto run = [test, this] {
      if (not level_++) {
        test_begin(test.name);
      } else {
        out_ << "\n \"" << test.name << "\"...";
      }

      test.test();

      if (not--level_) {
        test_end();
      }
    };

    if (run_) {
      run();
    } else {
      tests_.push_back(run);
    }
  }

  [[nodiscard]] auto run([[maybe_unused]] int argc, [[maybe_unused]] const char** argv)
      -> int {
    run_ = true;
    for (auto& test : tests_) {
      test();
    }

    return ut::default_cfg::tests_.fail > 0;
  }

  using ut::default_cfg::on;

 private:
  std::vector<std::function<void()>> tests_{};
  bool run_{};
};

template <>
auto ut::cfg<ut::override> = test_cfg{};

using ut::operator""_test;

auto _ = "test suite"_test = [] {
  using namespace ut;
  "should be equal"_test = [] { expect(42_i == 42); };
};

int main(int argc, const char** argv) {
  using namespace ut;

  "example"_test = [] { expect(42 == 42_i); };

  return ut::cfg<ut::override>.run(argc, argv);
}
