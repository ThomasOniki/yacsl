#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/format_string_parser.hpp"
#include "base/metaprogramming/declval.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/strings/itoa.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/range_like.hpp"
#include "base/type_traits/sign.hpp"
#include "base/utilities/extended_types.hpp"
#include "base/utilities/type_aliases.hpp"
#include "base/utilities/type_string.hpp"

namespace base {

template <typename T> class Formatter;

template <Integral T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    sign_  = parser.check_sign();
    width_ = parser.check_width();
    type_  = parser.check_option(Character<T> ? 'c' : 'd', "bodxc?");
    return parser.end_parse();
  }

  void format(CharBuffer& buf, T value) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
  char sign_  = '\0';
  char type_  = 'd';
};

extern template void Formatter<signed char>::format(CharBuffer& buf, signed char value) const;
extern template void Formatter<wchar_t>::format(CharBuffer& buf, wchar_t value) const;
extern template void Formatter<char8_t>::format(CharBuffer& buf, char8_t value) const;
extern template void Formatter<char16_t>::format(CharBuffer& buf, char16_t value) const;
extern template void Formatter<char32_t>::format(CharBuffer& buf, char32_t value) const;

extern template void Formatter<unsigned char>::format(CharBuffer& buf, unsigned char value) const;
extern template void Formatter<unsigned short>::format(CharBuffer& buf, unsigned short value) const;
extern template void Formatter<unsigned int>::format(CharBuffer& buf, unsigned int value) const;
extern template void Formatter<unsigned long>::format(CharBuffer& buf, unsigned long value) const;
extern template void Formatter<unsigned long long>::format(CharBuffer&        buf,
                                                           unsigned long long value) const;

extern template void Formatter<char>::format(CharBuffer& buf, char value) const;
extern template void Formatter<short>::format(CharBuffer& buf, short value) const;
extern template void Formatter<int>::format(CharBuffer& buf, int value) const;
extern template void Formatter<long>::format(CharBuffer& buf, long value) const;
extern template void Formatter<long long>::format(CharBuffer& buf, long long value) const;

#ifdef BASE_INT128_FOUND
extern template void Formatter<i128>::format(CharBuffer& buf, i128 value) const;
extern template void Formatter<u128>::format(CharBuffer& buf, u128 value) const;
#endif

template <FloatingPoint T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    sign_  = parser.check_sign();
    width_ = parser.check_width();
    type_  = parser.check_option('g', "gfea");
    return parser.end_parse();
  }

  void format(CharBuffer& buf, T value) const;

private:
  u32  precision_ = 0;
  u32  width_     = 0;
  char fill_      = ' ';
  char align_     = '>';
  char sign_      = '\0';
  char type_      = 'g';
};

template <> class Formatter<bool> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    type_  = parser.check_option('s', "sdxob");
    return parser.end_parse();
  }

  void format(CharBuffer& buf, bool value) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
  char type_  = 's';
};

template <> class Formatter<void*> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(CharBuffer& buf, const void* ptr) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
};

template <typename T> class Formatter<T*> : Formatter<void*> {
public:
  constexpr const char* parse(const char* fmt) { return Formatter<void*>::parse(fmt); }
  void format(CharBuffer& buf, const T* ptr) const { Formatter<void*>::format(buf, ptr); }
};

template <Character Char> class Formatter<BasicStringView<Char, usize>> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    type_  = parser.check_option('s', "s?");
    return parser.end_parse();
  }

  using Reference = const BasicStringView<Char, usize>&;

  void format(CharBuffer& buf, Reference str) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
  char type_  = 's';
};

extern template void Formatter<BasicStringView<char>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<unsigned char>>::format(CharBuffer&,
                                                                       Reference) const;
extern template void Formatter<BasicStringView<signed char>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<wchar_t>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<char8_t>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<char16_t>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<char32_t>>::format(CharBuffer&, Reference) const;

template <Character Char, typename SizeT>
class Formatter<BasicStringView<Char, SizeT>> : Formatter<BasicStringView<Char, usize>> {
public:
  constexpr const char* parse(const char* fmt) {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  void format(CharBuffer& buf, const BasicStringView<Char, SizeT>& str) const {
    return Formatter<BasicStringView<Char>>::format(
        buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())));
  }
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
template <Character Char, usize N> class Formatter<Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void format(CharBuffer& buf, const Char (&str)[N]) const {
    return Formatter<BasicStringView<Char>>::format(
        buf, BasicStringView(static_cast<const Char*>(str), N - 1));
  }
};
template <Character Char, usize N>
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
class Formatter<const Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void format(CharBuffer& buf, const Char (&str)[N]) const {
    return Formatter<BasicStringView<Char>>::format(buf, BasicStringView(str, N - 1));
  }
};

template <RangeLike T>
class Formatter<T> : Formatter<RemoveCVRef<decltype(*declval<T>().begin())>> {
public:
  constexpr const char* parse(const char* fmt) { return Base::parse(fmt); }

  void format(CharBuffer& buf, const T& range) {
    buf.append('[');
    auto iter = range.begin();
    auto end  = range.end();
    if (iter != end) {
      Base::format(buf, *iter);
      ++iter;
      for (; iter != end; ++iter) {
        buf.append(", ");
        Base::format(buf, *iter);
      }
    }
    buf.append(']');
  }

private:
  using Element = RemoveCVRef<decltype(*declval<T>().begin())>;
  using Base    = Formatter<Element>;
};

template <Enum T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    return parser.end_parse();
  }

  void format(CharBuffer& buf, T value) {
    auto scalar = static_cast<UnderlyingType>(value);
    auto u_val  = static_cast<MakeUnsigned<UnderlyingType>>(value);
    if (u_val < 256) {
      buf.append(TABLE[u_val]);
    } else {
      const usize len = itoa_digit_count(scalar);
      char*       str = buf.grow(len + TYPE_STRING<T>.size() + 2);
      str[0]          = '(';
      ++str;
      memcpy(str, TYPE_STRING<T>.data(), TYPE_STRING<T>.size());
      str[TYPE_STRING<T>.size()] = ')';
      itoa_from_end(buf.end(), scalar);
    }
  }

private:
  using UnderlyingType = UnderlyingEnumType<T>;
  template <u32... Values>
  static constexpr StackArray<StringView, sizeof...(Values)>
  table(IntegerSequence<u32, Values...> /*sequence*/) {
    StackArray<StringView, sizeof...(Values)> ret;
    (..., [&]() { ret[Values] = table_entry<T{static_cast<UnderlyingType>(Values)}>(); }());
    return ret;
  }

  template <T Value> static constexpr StringView table_entry() {
    constexpr StringView pretty_function = __PRETTY_FUNCTION__;
    constexpr usize      offset =
#ifdef BASE_COMPILER_GCC
        StringView("with ").size() +
#endif
        StringView("static StringView base::Formatter<>::table_entry() [T = , Value = ").size() +
        2 * TYPE_STRING<T>.size();
    return {pretty_function.begin() + offset, pretty_function.end() - 1};
  }

  static constexpr StackArray<StringView, 256> TABLE = table(IntegerSequenceToN<u32, 256>{});
};

} // namespace base
