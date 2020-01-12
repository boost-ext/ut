//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <stdexcept>

int main() {
  using namespace boost::ut;

  "exceptions"_test = [] {
    expect(throws<std::runtime_error>([] {
      throw std::runtime_error{"exception!"};
    })) << "throws runtime_error";
    expect(throws([] { throw 0; })) << "throws any exception";
    expect(nothrow([] {})) << "doesn't throw";
  };
}
