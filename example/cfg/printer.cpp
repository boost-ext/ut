//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

namespace ut = boost::ut;

namespace cfg {
struct printer : ut::printer {
  constexpr printer() {
    none = "";
    red = "";
    green = "";
  }
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<ut::reporter<cfg::printer>>{};

int main() {
  using namespace ut;
  "no colors"_test = [] {};
}
