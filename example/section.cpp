//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <sstream>
#include <string_view>
#include <vector>

int main() {
  using namespace boost::ut;

  "[vector]"_test = [] {
    std::vector<int> v(5);

    expect((5_ul == std::size(v)) >> fatal);

    should("resize bigger") = [v] {  // or "resize bigger"_test
      mut(v).resize(10);
      expect(10_ul == std::size(v));
    };

    expect((5_ul == std::size(v)) >> fatal);

    should("resize smaller") = [=]() mutable {  // or "resize smaller"_test
      v.resize(0);
      expect(0_ul == std::size(v));
    };
  };

  using namespace std::literals;

  {
    std::stringstream str{};

    "[str1]"_test = [str{std::move(str)}] {
      mut(str) << '1';
      expect(str.str() == "1"sv);
    };
  }

  // FIXME: [clang-analyzer-cplusplus.Move,-warnings-as-errors]
  // Note: Object 'str' of type 'std::__1::basic_stringstream' is left in a
  // valid but unspecified state after move
  {
    std::stringstream str{};

    "[str2]"_test = [str{std::move(str)}] {
      mut(str) << '2';
      expect(str.str() == "2"sv);
    };
  }
}
