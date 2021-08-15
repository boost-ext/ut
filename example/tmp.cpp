//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

template <class...>
struct list {};

int main() {
  using namespace boost::ut;

  static constexpr auto i = 42;

  "type"_test= [] {
    constexpr auto return_int = [] { return i; };

    expect(type<>(i) == type<int>);
    expect(type<int> == type<>(i));
    expect(type<int> == return_int());
    expect(type<float> != return_int());
  };

  "constant"_test = [] {
    expect(constant<42_i == i> and type<void> == type<void> and
           type<list<void, int>> == type<list<void, int>>);
  };

#if defined(__cpp_concepts)
  "compiles"_test = [] {

    struct foo { int value{}; };
    struct bar { };

    expect(    [](auto t) { return requires { t.value; }; }(foo{}));
    expect(not [](auto t) { return requires { t.value; }; }(bar{}));
  };
#endif
}
