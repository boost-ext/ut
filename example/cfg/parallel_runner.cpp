//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/ut.hpp>
#include <iostream>
#if __has_include(<execution>)
#include <execution>
#endif

namespace ut = boost::ut;

namespace cfg {
class parallel_runner : public ut::runner<> {
 public:
  using ut::runner<>::on;

  template <class... Ts>
  auto on(ut::events::test<Ts...> test) {
    std::clog << test.name << '\n';
    ut::runner<>::on(test);
  }

  [[nodiscard]] auto run() -> bool {
#if defined(__cpp_lib_parallel_algorithm)
    std::for_each(std::execution::par, std::cbegin(suites_), std::cend(suites_),
                  [&](const auto& suite) { suite(); });
#else
    std::for_each(std::cbegin(suites_), std::cend(suites_),
                  [&](const auto& suite) { suite(); });
#endif

    suites_.clear();

    return fails_ > 0;
  }
};
}  // namespace cfg

template <class... Ts>
inline auto ut::cfg<ut::override, Ts...> = cfg::parallel_runner{};

ut::suite parallel_1 = [] {
  using namespace ut;

  // sequential
  "test.1.1"_test = [] { expect(1_i == 1); };
  "test.1.2"_test = [] { expect(2_i == 2); };
  "test.1.3"_test = [] { expect(3_i == 3); };
};

ut::suite parallel_2 = [] {
  using namespace ut;

  // sequential
  "test.2.1"_test = [] { expect(1_i == 1); };
  "test.2.2"_test = [] { expect(2_i == 2); };
  "test.2.3"_test = [] { expect(3_i == 3); };
};

int main() { return ut::cfg<ut::override>.run(); }
