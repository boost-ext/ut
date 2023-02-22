/*
 * File: static_string_cpp.hpp
 * File Created: 2023-02-08 07:30:54
 * Author: Костин П.А. (kostin_pavel@mail.ru)
 * Last Modified:
 * Modified By:
 */
#pragma once
#include <array>
#include <limits>
#include <string>

namespace snw1 {

#define ITOSS(x) int_to_static_string<(x), char>()
#define ITOSW(x) int_to_static_string<(x), wchar_t>()
#define UTOSS(x) uint_to_static_string<(x), char>()
#define UTOSW(x) uint_to_static_string<(x), wchar_t>()
#define SSTOI(x) static_string_to_int((x))
#define SSTOU(x) static_string_to_uint((x))

template <typename Char, size_t Size>
struct basic_static_string;

namespace __static_string_detail {

template <size_t... Indexes>
struct index_sequence {};

template <size_t Size, size_t... Indexes>
struct make_index_sequence
    : make_index_sequence<Size - 1, Size - 1, Indexes...> {};

template <size_t... Indexes>
struct make_index_sequence<0, Indexes...> : index_sequence<Indexes...> {};

template <size_t Size, size_t... Indexes>
struct make_reverse_index_sequence
    : make_reverse_index_sequence<Size - 1, Indexes..., Size - 1> {};

template <size_t... Indexes>
struct make_reverse_index_sequence<0, Indexes...> : index_sequence<Indexes...> {
};

template <size_t Begin, size_t End, size_t... Indexes>
struct make_index_subsequence
    : make_index_subsequence<Begin, End - 1, End - 1, Indexes...> {};

template <size_t Pos, size_t... Indexes>
struct make_index_subsequence<Pos, Pos, Indexes...>
    : index_sequence<Indexes...> {};

template <typename Char, Char... Chars>
struct char_sequence {};

template <unsigned long long Value, typename Char, Char... Chars>
struct make_uint_char_sequence
    : make_uint_char_sequence<Value / 10, Char,
                              static_cast<Char>('0') + Value % 10, Chars...> {};

template <typename Char, Char... Chars>
struct make_uint_char_sequence<0, Char, Chars...>
    : char_sequence<Char, Chars...> {};

template <typename Char>
struct make_uint_char_sequence<0, Char>
    : char_sequence<Char, static_cast<Char>('0')> {};

template <bool Negative, long long Value, typename Char, Char... Chars>
struct make_int_char_sequence {};

template <long long Value, typename Char, Char... Chars>
struct make_int_char_sequence<true, Value, Char, Chars...>
    : make_int_char_sequence<true, Value / 10, Char,
                             static_cast<Char>('0') - (Value % 10), Chars...> {
};

template <long long Value, typename Char, Char... Chars>
struct make_int_char_sequence<false, Value, Char, Chars...>
    : make_int_char_sequence<false, Value / 10, Char,
                             static_cast<Char>('0') + Value % 10, Chars...> {};

template <typename Char, Char... Chars>
struct make_int_char_sequence<true, 0, Char, Chars...>
    : char_sequence<Char, static_cast<Char>('-'), Chars...> {};

template <typename Char, Char... Chars>
struct make_int_char_sequence<false, 0, Char, Chars...>
    : char_sequence<Char, Chars...> {};

template <typename Char>
struct make_int_char_sequence<false, 0, Char>
    : char_sequence<Char, static_cast<Char>('0')> {};

template <typename Char, size_t Size>
constexpr basic_static_string<Char, Size> make(
    const basic_static_string<Char, Size>& str) {
  return str;
}

template <typename Char, size_t Size>
constexpr basic_static_string<Char, Size> make(const Char (&str)[Size]) {
  return make(str, make_index_sequence<Size - 1>{});
}

template <typename Char, Char... Chars>
constexpr basic_static_string<Char, sizeof...(Chars) + 1> make(
    char_sequence<Char, Chars...>) {
  return {Chars..., static_cast<Char>('\0')};
}

template <typename Char, size_t Size, size_t... Indexes>
constexpr basic_static_string<Char, sizeof...(Indexes) + 1> make(
    const Char (&str)[Size], index_sequence<Indexes...>) {
  return {str[Indexes]..., static_cast<Char>('\0')};
}

template <typename Char, size_t Size, size_t... Indexes>
constexpr basic_static_string<Char, sizeof...(Indexes) + 1> make(
    const basic_static_string<Char, Size>& str, index_sequence<Indexes...>) {
  return {str.data[Indexes]..., static_cast<Char>('\0')};
}

template <typename Char, size_t Size1, size_t Size2>
constexpr int compare(const basic_static_string<Char, Size1>& str1,
                      size_t index1,
                      const basic_static_string<Char, Size2>& str2,
                      size_t index2, size_t cur_length, size_t max_length) {
  return cur_length > max_length || (index1 >= Size1 && index2 >= Size2) ? 0
         : index1 >= Size1                                               ? -1
         : index2 >= Size2                                               ? 1
         : str1.data[index1] > str2.data[index2]                         ? 1
         : str1.data[index1] < str2.data[index2]
             ? -1
             : compare(str1, index1 + 1, str2, index2 + 1, cur_length + 1,
                       max_length);
}

template <typename Char, size_t Size1, size_t Size2>
constexpr int compare(const basic_static_string<Char, Size1>& str1,
                      const basic_static_string<Char, Size2>& str2) {
  return compare(str1, 0, str2, 0, 1, std::numeric_limits<size_t>::max());
}

template <typename Char, size_t Size1, size_t Size2>
constexpr int compare(const basic_static_string<Char, Size1>& str1,
                      const Char (&str2)[Size2]) {
  return compare(str1, __static_string_detail::make(str2));
}

template <typename Char, size_t Size1, size_t Size2>
constexpr int compare(const Char (&str1)[Size1],
                      const basic_static_string<Char, Size2>& str2) {
  return compare(__static_string_detail::make(str1), str2);
}

template <typename Char, size_t Size1, size_t Size2>
constexpr int static_string_compare(const Char (&str1)[Size1],
                                    const Char (&str2)[Size2]) {
  return compare(__static_string_detail::make(str1),
                 __static_string_detail::make(str2));
}

template <typename Char, size_t Size1, size_t... Indexes1, size_t Size2,
          size_t... Indexes2>
constexpr basic_static_string<Char, Size1 + Size2 - 1> concat2(
    const basic_static_string<Char, Size1>& str1, index_sequence<Indexes1...>,
    const basic_static_string<Char, Size2>& str2, index_sequence<Indexes2...>) {
  return {str1.data[Indexes1]..., str2.data[Indexes2]...,
          static_cast<Char>('\0')};
}

template <typename Char, size_t Size1, size_t Size2>
constexpr basic_static_string<Char, Size1 + Size2 - 1> concat2(
    const basic_static_string<Char, Size1>& str1,
    const basic_static_string<Char, Size2>& str2) {
  return concat2(str1, make_index_sequence<Size1 - 1>{}, str2,
                 make_index_sequence<Size2 - 1>{});
}

template <typename Char>
constexpr basic_static_string<Char, 1> concat() {
  return {static_cast<Char>('\0')};
}

template <typename Char, typename Arg, typename... Args>
constexpr auto concat(Arg&& arg, Args&&... args) {
  return concat2(make(std::forward<Arg>(arg)),
                 concat<Char>(std::forward<Args>(args)...));
}

template <typename Char, size_t Size>
constexpr unsigned long long hash(const basic_static_string<Char, Size>& str,
                                  size_t index) {
  return index >= Size - 1
             ? 5381ULL
             : hash(str, index + 1) * 33ULL +
                   static_cast<unsigned long long>(str.data[index] + 1);
}

template <typename Char, size_t Size>
constexpr size_t count(const basic_static_string<Char, Size>& str, Char ch,
                       size_t index) {
  return index >= Size - 1
             ? 0
             : (str.data[index] == ch ? 1 : 0) + count(str, ch, index + 1);
}

template <typename Char, size_t Size>
constexpr long long to_int(const basic_static_string<Char, Size>& str,
                           size_t index, size_t first) {
  return index < first || index >= Size - 1 ? 0
         : first == 0 ? (str.data[index] - static_cast<Char>('0')) +
                            10LL * to_int(str, index - 1, first)
                      : -(str.data[index] - static_cast<Char>('0')) +
                            10LL * to_int(str, index - 1, first);
}

template <typename Char, size_t Size>
constexpr long long to_int(const basic_static_string<Char, Size>& str) {
  return Size < 2 ? 0LL
         : str.data[0] == static_cast<Char>('-')
             ? __static_string_detail::to_int(str, Size - 2, 1)
             : __static_string_detail::to_int(str, Size - 2, 0);
}

template <typename Char, size_t Size>
constexpr unsigned long long to_uint(const basic_static_string<Char, Size>& str,
                                     size_t index) {
  return Size < 2 || index >= Size - 1
             ? 0
             : (str.data[index] - static_cast<Char>('0')) +
                   10ULL * to_uint(str, index - 1);
}

template <typename Char, size_t Size>
std::basic_string<Char> to_string(const basic_static_string<Char, Size>& str) {
  return std::basic_string<Char>(str.data.data());
}

template <typename Char>
constexpr Char lower(Char ch) {
  return ch < static_cast<Char>('A') || ch > static_cast<Char>('Z')
             ? ch
             : ch - static_cast<Char>('A') + static_cast<Char>('a');
}

template <typename Char>
constexpr Char upper(Char ch) {
  return ch < static_cast<Char>('a') || ch > static_cast<Char>('z')
             ? ch
             : ch - static_cast<Char>('a') + static_cast<Char>('A');
}

template <typename Char, size_t Size, size_t... Indexes>
constexpr basic_static_string<Char, Size> lower(
    const basic_static_string<Char, Size>& str, index_sequence<Indexes...>) {
  return {lower(str.data[Indexes])...};
}

template <typename Char, size_t Size, size_t... Indexes>
constexpr basic_static_string<Char, Size> upper(
    const basic_static_string<Char, Size>& str, index_sequence<Indexes...>) {
  return {upper(str.data[Indexes])...};
}

}  // namespace __static_string_detail

template <typename Char, size_t Size>
struct basic_static_string {
  static constexpr size_t npos = std::numeric_limits<size_t>::max();
  constexpr size_t length() const { return Size - 1; }
  constexpr size_t size() const { return Size - 1; }
  constexpr size_t begin() const { return 0; }
  constexpr size_t end() const { return Size - 1; }
  constexpr size_t rbegin() const { return Size - 2; }
  constexpr size_t rend() const { return std::numeric_limits<size_t>::max(); }
  constexpr bool empty() const { return Size < 2; }
  template <typename... Args>
  static constexpr auto make(Args&&... args) {
    return __static_string_detail::concat<Char>(std::forward<Args>(args)...);
  }
  constexpr auto reverse() const {
    return __static_string_detail::make(
        *this, __static_string_detail::make_reverse_index_sequence<Size - 1>{});
  }
  template <size_t Begin, size_t End>
  constexpr auto substring() const {
    static_assert(Begin <= End, "Begin is greater than End (Begin > End)");
    static_assert(End <= Size - 1,
                  "End is greater than string length (End > Size - 1)");
    return __static_string_detail::make(
        *this, __static_string_detail::make_index_subsequence<Begin, End>{});
  }
  template <size_t End>
  constexpr auto prefix() const {
    return substring<0, End>();
  }
  template <size_t Begin>
  constexpr auto suffix() const {
    return substring<Begin, Size - 1>();
  }
  constexpr size_t find(Char ch, size_t from = 0, size_t nth = 0) const {
    return Size < 2 || from >= Size - 1 ? npos
           : data[from] != ch           ? find(ch, from + 1, nth)
           : nth > 0                    ? find(ch, from + 1, nth - 1)
                                        : from;
  }
  template <size_t SubSize>
  constexpr size_t find(const basic_static_string<Char, SubSize>& substr,
                        size_t from = 0, size_t nth = 0) const {
    return (Size < SubSize) || (from > (Size - SubSize)) ? npos
           : __static_string_detail::compare(*this, from, substr, 0, 1,
                                             SubSize - 1) != 0
               ? find(substr, from + 1, nth)
           : nth > 0 ? find(substr, from + 1, nth - 1)
                     : from;
  }
  template <size_t SubSize>
  constexpr size_t find(const Char (&substr)[SubSize], size_t from = 0,
                        size_t nth = 0) const {
    return find(__static_string_detail::make(substr), from, nth);
  }
  constexpr size_t rfind(Char ch, size_t from = Size - 2,
                         size_t nth = 0) const {
    return Size < 2 || from > Size - 2 ? npos
           : data[from] != ch          ? rfind(ch, from - 1, nth)
           : nth > 0                   ? rfind(ch, from - 1, nth - 1)
                                       : from;
  }
  template <size_t SubSize>
  constexpr size_t rfind(const basic_static_string<Char, SubSize>& substr,
                         size_t from = Size - SubSize, size_t nth = 0) const {
    return (Size < SubSize) || (from > (Size - SubSize)) ? npos
           : __static_string_detail::compare(*this, from, substr, 0, 1,
                                             SubSize - 1) != 0
               ? rfind(substr, from - 1, nth)
           : nth > 0 ? rfind(substr, from - 1, nth - 1)
                     : from;
  }
  template <size_t SubSize>
  constexpr size_t rfind(const Char (&substr)[SubSize],
                         size_t from = Size - SubSize, size_t nth = 0) const {
    return rfind(__static_string_detail::make(substr), from, nth);
  }
  constexpr bool contains(Char ch) const { return find(ch) != npos; }
  template <size_t SubSize>
  constexpr bool contains(
      const basic_static_string<Char, SubSize>& substr) const {
    return find(substr) != npos;
  }
  template <size_t SubSize>
  constexpr bool contains(const Char (&substr)[SubSize]) const {
    return find(substr) != npos;
  }
  template <size_t SubSize>
  constexpr bool starts_with(
      const basic_static_string<Char, SubSize>& prefix) const {
    return SubSize > Size ? false
                          : __static_string_detail::compare(
                                *this, 0, prefix, 0, 1, SubSize - 1) == 0;
  }
  template <size_t SubSize>
  constexpr bool starts_with(const Char (&prefix)[SubSize]) const {
    return starts_with(__static_string_detail::make(prefix));
  }
  template <size_t SubSize>
  constexpr bool ends_with(
      const basic_static_string<Char, SubSize>& suffix) const {
    return SubSize > Size
               ? false
               : __static_string_detail::compare(*this, Size - SubSize, suffix,
                                                 0, 1, SubSize - 1) == 0;
  }
  template <size_t SubSize>
  constexpr bool ends_with(const Char (&suffix)[SubSize]) const {
    return ends_with(__static_string_detail::make(suffix));
  }
  constexpr size_t count(Char ch) const {
    return __static_string_detail::count(*this, ch, 0);
  }
  template <size_t Index>
  constexpr auto split() const {
    return std::make_pair(prefix<Index>(), suffix<Index + 1>());
  }
  constexpr unsigned long long hash() const {
    return __static_string_detail::hash(*this, 0);
  }
  constexpr Char operator[](size_t index) const { return data[index]; }
  constexpr long long to_int() const {
    return __static_string_detail::to_int(*this);
  }
  constexpr unsigned long long to_uint() const {
    return __static_string_detail::to_uint(*this, 0);
  }
  std::string str() const { return __static_string_detail::to_string(*this); }
  constexpr auto lower() const {
    return __static_string_detail::lower(
        *this, __static_string_detail::make_index_sequence<Size>{});
  }
  constexpr auto upper() const {
    return __static_string_detail::upper(
        *this, __static_string_detail::make_index_sequence<Size>{});
  }
  std::array<const Char, Size> data;
};

template <size_t Size>
using static_string_t = basic_static_string<char, Size>;
template <size_t Size>
using static_wstring_t = basic_static_string<wchar_t, Size>;

using static_string = basic_static_string<char, 0>;
using static_wstring = basic_static_string<wchar_t, 0>;

/* template <typename Char, Char... Chars>
constexpr basic_static_string<Char, sizeof...(Chars) + 1> operator"" _ss() {
  return {Chars..., static_cast<Char>('\0')};
}; */

template <typename Char, size_t Size>
std::basic_ostream<Char>& operator<<(
    std::basic_ostream<Char>& bos, const basic_static_string<Char, Size>& str) {
  bos << str.data.data();
  return bos;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator==(const basic_static_string<Char, Size1>& str1,
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) == 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator==(const basic_static_string<Char, Size1>& str1,
                          const Char (&str2)[Size2]) {
  return __static_string_detail::compare(str1, str2) == 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator==(const Char (&str1)[Size1],
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) == 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator!=(const basic_static_string<Char, Size1>& str1,
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) != 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator!=(const basic_static_string<Char, Size1>& str1,
                          const Char (&str2)[Size2]) {
  return __static_string_detail::compare(str1, str2) != 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator!=(const Char (&str1)[Size1],
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) != 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator<(const basic_static_string<Char, Size1>& str1,
                         const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) < 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator<(const basic_static_string<Char, Size1>& str1,
                         const Char (&str2)[Size2]) {
  return __static_string_detail::compare(str1, str2) < 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator<(const Char (&str1)[Size1],
                         const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) < 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator<=(const basic_static_string<Char, Size1>& str1,
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) <= 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator<=(const basic_static_string<Char, Size1>& str1,
                          const Char (&str2)[Size2]) {
  return __static_string_detail::compare(str1, str2) <= 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator<=(const Char (&str1)[Size1],
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) <= 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator>(const basic_static_string<Char, Size1>& str1,
                         const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) > 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator>(const basic_static_string<Char, Size1>& str1,
                         const Char (&str2)[Size2]) {
  return __static_string_detail::compare(str1, str2) > 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator>(const Char (&str1)[Size1],
                         const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) > 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator>=(const basic_static_string<Char, Size1>& str1,
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) >= 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator>=(const basic_static_string<Char, Size1>& str1,
                          const Char (&str2)[Size2]) {
  return __static_string_detail::compare(str1, str2) >= 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr bool operator>=(const Char (&str1)[Size1],
                          const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::compare(str1, str2) >= 0;
}

template <typename Char, size_t Size1, size_t Size2>
constexpr auto operator+(const basic_static_string<Char, Size1>& str1,
                         const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::concat2(str1, str2);
}

template <typename Char, size_t Size1, size_t Size2>
constexpr auto operator+(const basic_static_string<Char, Size1>& str1,
                         const Char (&str2)[Size2]) {
  return __static_string_detail::concat2(str1,
                                         __static_string_detail::make(str2));
}

template <typename Char, size_t Size1, size_t Size2>
constexpr auto operator+(const Char (&str1)[Size1],
                         const basic_static_string<Char, Size2>& str2) {
  return __static_string_detail::concat2(__static_string_detail::make(str1),
                                         str2);
}

template <long long Value, typename Char>
constexpr auto int_to_static_string() {
  return __static_string_detail::make(
      __static_string_detail::make_int_char_sequence<(Value < 0), Value,
                                                     Char>{});
}

template <unsigned long long Value, typename Char>
constexpr auto uint_to_static_string() {
  return __static_string_detail::make(
      __static_string_detail::make_uint_char_sequence<Value, Char>{});
}

template <typename Char, size_t Size>
constexpr long long static_string_to_int(
    const basic_static_string<Char, Size>& str) {
  return __static_string_detail::to_int(str);
}

template <typename Char, size_t Size>
constexpr long long static_string_to_int(const Char (&str)[Size]) {
  return static_string_to_int(__static_string_detail::make(str));
}

template <typename Char, size_t Size>
constexpr unsigned long long static_string_to_uint(
    const basic_static_string<Char, Size>& str) {
  return __static_string_detail::to_uint(str, Size - 2);
}

template <typename Char, size_t Size>
constexpr unsigned long long static_string_to_uint(const Char (&str)[Size]) {
  return static_string_to_uint(__static_string_detail::make(str));
}

}  // namespace snw1