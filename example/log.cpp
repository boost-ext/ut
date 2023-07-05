//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

#if __has_include(<expected>)
#include <expected>
#include <string>
#endif

int main() {
  using namespace boost::ut;

  "log"_test = [] {
    boost::ut::log << "pre:" << 42;
    expect(42_i == 42) << "message on failure";
    boost::ut::log << "post";
  };

#if defined(__cpp_lib_expected)
  "lazy log"_test = [] {
    // It's not possible to write a test like:
    //   expect(e.has_value()) << e.error() << fatal;
    // This would evaluate e.error() when there is no error, instead lazy
    // evaluation is needed.
    std::expected<bool, std::string> e = true;
    expect(e.has_value()) << [&] { return e.error(); } << fatal;
    expect(e.value() == true);
  };
#endif
}
