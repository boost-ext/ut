//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

#if defined(_MSC_VER) and not defined(__clang__)
  // MSVC fails if the `""_i` literal operator is used inside the 'verify' lambda:
  // "sl.cpp(16,37): error C3688: invalid literal suffix '_i'; literal operator or literal operator template 'operator ""_i' not found"
  "sl"_test = [] {
    static constexpr auto _42_i = 42_i;
    auto verify = [](auto sl, auto i) {
      expect(i == _42_i, sl) << "error with given source location";
    };

    verify(boost::ut::reflection::source_location::current(), 42_i);
  };
#else
  "sl"_test = [] {
    auto verify = [](auto sl, auto i) {
      expect(i == 42_i, sl) << "error with given source location";
    };

    verify(boost::ut::reflection::source_location::current(), 42_i);
  };
#endif
}
