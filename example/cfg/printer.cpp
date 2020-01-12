//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <iostream>
#include <utility>

namespace ut = boost::ut;

namespace cfg {
struct printer : ut::printer {
  template <class T>
  auto& operator<<(T&& t) {
    std::cerr << std::forward<T>(t);
    return *this;
  }
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<ut::reporter<cfg::printer>>{};

int main() {
  using namespace ut;
  "printer"_test = [] {};
}
