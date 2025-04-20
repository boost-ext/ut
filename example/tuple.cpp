#include <atomic>
#include <boost/ut.hpp>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

namespace {
std::optional<std::tuple<int, std::string>> findUserById(int id) {
  if (id == 42) {
    return std::make_tuple(42, "Alice");
  } else {
    return std::nullopt;
  }
}

std::tuple<double, char, std::string> get_student(int id) {
  switch (id) {
    case 0:
      return {3.8, 'A', "Lisa Simpson"};
    case 1:
      return {2.9, 'C', "Milhouse Van Houten"};
    case 2:
      return {1.7, 'D', "Ralph Wiggum"};
    case 3:
      return {0.6, 'F', "Bart Simpson"};
  }

  throw std::invalid_argument("id");
}

struct S {
  int n;
  std::string s;
  double d;

  friend bool operator<(const S& lhs, const S& rhs) noexcept {
    // compares lhs.n to rhs.n,
    // then lhs.s to rhs.s,
    // then lhs.d to rhs.d
    // in that order, first non-equal result is returned
    // or false if all elements are equal
    return std::tie(lhs.n, lhs.s, lhs.d) < std::tie(rhs.n, rhs.s, rhs.d);
  }
};

std::tuple<int, int> divide(int a, int b) {
  return std::make_tuple(a / b, a % b);  // quotient and remainder
}

}  // namespace

int main() {
  using namespace boost::ut;

  const auto student0 = get_student(0);
  std::cout << "ID: 0, "
            << "GPA: " << std::get<0>(student0) << ", "
            << "grade: " << std::get<1>(student0) << ", "
            << "name: " << std::get<2>(student0) << '\n';
  expect(std::tuple_size<decltype(student0)>::value == 3);
  expect(std::get<2>(student0) == "Lisa Simpson");

  const auto student1 = get_student(1);
  std::cout << "ID: 1, "
            << "GPA: " << std::get<double>(student1) << ", "
            << "grade: " << std::get<char>(student1) << ", "
            << "name: " << std::get<std::string>(student1) << '\n';
  expect(std::get<2>(student1) == "Milhouse Van Houten");

  double gpa2{};
  char grade2{};
  std::string name2;
  std::tie(gpa2, grade2, name2) = get_student(2);
  std::cout << "ID: 2, "
            << "GPA: " << gpa2 << ", "
            << "grade: " << grade2 << ", "
            << "name: " << name2 << '\n';
  expect(get_student(2) ==
         std::tuple<double, char, std::string>{1.7, 'D', "Ralph Wiggum"});

  // C++17 structured binding:
  const auto [gpa3, grade3, name3] = get_student(3);
  std::cout << "ID: 3, "
            << "GPA: " << gpa3 << ", "
            << "grade: " << grade3 << ", "
            << "name: " << name3 << '\n';
  expect(get_student(3) ==
         std::tuple<double, char, std::string>{0.6, 'F', "Bart Simpson"});

  // Lexicographical comparison demo:
  std::set<S> set_of_s;

  S value{42, "Test", 3.14};
  std::set<S>::iterator iter;
  bool is_inserted{};

  // Unpack a pair:
  std::tie(iter, is_inserted) = set_of_s.insert(value);
  expect(is_inserted);

  // std::tie and structured bindings:
  auto position = [](int w) { return std::tuple(1 * w, 2 * w); };

  auto [x, y] = position(1);
  expect(x == 1 && y == 2);
  std::tie(x, y) = position(2);  // reuse x, y with tie
  expect(x == 2 && y == 4);

  // Implicit conversions are permitted:
  std::tuple<char, short> coordinates(6, 9);
  std::tie(x, y) = coordinates;
  expect(x == 6 && y == 9);

  // Skip an element:
  std::string z;
  std::tie(x, std::ignore, z) = std::tuple(1, 2.0, "Test");
  expect(x == 1 && z == "Test");

  // Multiple Return Values
  auto [quotient, remainder] = divide(10, 3);
  expect(quotient == 3);
  expect(remainder == 1);

  expect(std::tuple(2, 1) == divide(5, 2));

  std::atomic<bool> test_value{false};
  expect(!test_value);

  {
    auto result = findUserById(42);
    //FIXME: expect(result);
    if (result) {
      // To unpack, use *result or result.value().
      auto [userId, username] = *result;  // unpack the tuple
      std::cout << "Found user: " << userId << ", " << username << "\n";
    } else {
      std::cout << "User not found.\n";
    }
    expect(result != std::nullopt);
  }
}
