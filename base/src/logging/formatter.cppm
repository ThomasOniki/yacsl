module;

#include "base/attributes.hpp"
#include <string.h>

export module base.logging.formatter;

import base.type_traits.categories;
import base.type_traits.type_string;
import base.type_traits.range_like;
import base.type_traits.sign;
import base.type_traits.type_string;
import base.type_traits.qualifiers;
import base.logging.format_string_parser;
import base.containers.string;
import base.containers.stack_array;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.ranges.string_view;
import base.metaprogramming.integer_sequence;
import base.metaprogramming.declval;
import base.strings.itoa;

export namespace base {

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

  void format(String& buf, T value) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
  char sign_  = '\0';
  char type_  = 'd';
};

extern template void Formatter<signed char>::format(String& buf, signed char value) const;
extern template void Formatter<wchar_t>::format(String& buf, wchar_t value) const;
extern template void Formatter<char8_t>::format(String& buf, char8_t value) const;
extern template void Formatter<char16_t>::format(String& buf, char16_t value) const;
extern template void Formatter<char32_t>::format(String& buf, char32_t value) const;

extern template void Formatter<unsigned char>::format(String& buf, unsigned char value) const;
extern template void Formatter<unsigned short>::format(String& buf, unsigned short value) const;
extern template void Formatter<unsigned int>::format(String& buf, unsigned int value) const;
extern template void Formatter<unsigned long>::format(String& buf, unsigned long value) const;
extern template void Formatter<unsigned long long>::format(String&            buf,
                                                           unsigned long long value) const;

extern template void Formatter<char>::format(String& buf, char value) const;
extern template void Formatter<short>::format(String& buf, short value) const;
extern template void Formatter<int>::format(String& buf, int value) const;
extern template void Formatter<long>::format(String& buf, long value) const;
extern template void Formatter<long long>::format(String& buf, long long value) const;

#ifdef BASE_INT128_FOUND
extern template void Formatter<i128>::format(String& buf, i128 value) const;
extern template void Formatter<u128>::format(String& buf, u128 value) const;
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

  void format(String& buf, T value) const;

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

  void format(String& buf, bool value) const;

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

  void format(String& buf, const void* ptr) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
};

template <typename T> class Formatter<T*> : Formatter<void*> {
public:
  constexpr const char* parse(const char* fmt) { return Formatter<void*>::parse(fmt); }
  void format(String& buf, const T* ptr) const { Formatter<void*>::format(buf, ptr); }
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

  using View = BasicStringView<Char, usize>;

  void format(String& buf, View str) const;

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
  char type_  = 's';
};

extern template void Formatter<BasicStringView<char>>::format(String&, View) const;
extern template void Formatter<BasicStringView<unsigned char>>::format(String&, View) const;
extern template void Formatter<BasicStringView<signed char>>::format(String&, View) const;
extern template void Formatter<BasicStringView<wchar_t>>::format(String&, View) const;
extern template void Formatter<BasicStringView<char8_t>>::format(String&, View) const;
extern template void Formatter<BasicStringView<char16_t>>::format(String&, View) const;
extern template void Formatter<BasicStringView<char32_t>>::format(String&, View) const;

template <Character Char, Integral SizeT>
class Formatter<BasicStringView<Char, SizeT>> : Formatter<BasicStringView<Char, usize>> {
public:
  constexpr const char* parse(const char* fmt) {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  void format(String& buf, BasicStringView<Char, SizeT> str) const {
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
  void format(String& buf, const Char (&str)[N]) const {
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
  void format(String& buf, const Char (&str)[N]) const {
    return Formatter<BasicStringView<Char>>::format(buf, BasicStringView(str, N - 1));
  }
};

template <RangeLike T>
class Formatter<T> : Formatter<RemoveCVRef<decltype(*declval<T>().begin())>> {
public:
  constexpr const char* parse(const char* fmt) { return Base::parse(fmt); }

  void format(String& buf, const T& range) {
    buf += '[';
    auto iter = range.begin();
    auto end  = range.end();
    if (iter != end) {
      Base::format(buf, *iter);
      ++iter;
      for (; iter != end; ++iter) {
        buf += ", ";
        Base::format(buf, *iter);
      }
    }
    buf += ']';
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

  void format(String& buf, T value) {
    auto scalar = static_cast<UnderlyingType>(value);
    auto u_val  = static_cast<MakeUnsigned<UnderlyingType>>(value);
    if (u_val < 256) {
      buf += TABLE[u_val];
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
  make_table(IntegerSequence<u32, Values...> /*sequence*/) {
    StackArray<StringView, sizeof...(Values)> ret;
    (..., [&]() { ret[Values] = table_entry<T{static_cast<UnderlyingType>(Values)}>(); }());
    return ret;
  }

  template <T Value> static constexpr StringView table_entry_pretty() {
    return __PRETTY_FUNCTION__;
  }

  static constexpr usize TABLE_ENTRY_OFFSET_FROM_BEGIN = []() -> usize {
    constexpr StringView pretty  = table_entry_pretty<static_cast<T>(0)>();
    constexpr StringView to_find = "Value = ";
    for (const char* it = pretty.begin(); it < pretty.end() - to_find.size(); ++it) {
      for (usize i = 0;; ++i) {
        if (i == to_find.size()) {
          return static_cast<usize>(it - pretty.begin()) + i - "_pretty"_sv.size();
        }
        if (it[i] != to_find[i]) {
          break;
        }
      }
    }
    return 0;
  }();

  static constexpr usize TABLE_ENTRY_OFFSET_FROM_END = []() -> usize {
    constexpr StringView pretty    = table_entry_pretty<static_cast<T>(0)>();
    usize                templates = 0;
    for (usize i = TABLE_ENTRY_OFFSET_FROM_BEGIN; i < pretty.size(); ++i) {
      if (pretty[i] == '<') {
        ++templates;
      } else if (pretty[i] == '>') {
        --templates;
      }
      if (templates > 0) {
        continue;
      }
      if (pretty[i] == ',' || pretty[i] == ';' || pretty[i] == ']') {
        return pretty.size() - i;
      }
    }
    return pretty.size();
  }();

  template <T Value> static constexpr StringView table_entry() {
    constexpr StringView pretty = __PRETTY_FUNCTION__;
    return {pretty.begin() + TABLE_ENTRY_OFFSET_FROM_BEGIN,
            pretty.end() - TABLE_ENTRY_OFFSET_FROM_END};
  }

  static constexpr StackArray<StringView, 256> TABLE = make_table(IntegerSequenceToN<u32, 256>{});
};

} // namespace base
