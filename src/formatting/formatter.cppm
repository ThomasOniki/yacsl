module;

#include "base/attributes.hpp"

export module base.formatting.formatter;

import base.type_traits.allocator;
import base.type_traits.categories;
import base.type_traits.range_like;
import base.type_traits.qualifiers;
import base.type_traits.type_string;
import base.type_traits.sign;
import base.formatting.format_string_parser;
import base.formatting.padding;
import base.formatting.itoa;
import base.containers.string;
import base.containers.stack_array;
import base.containers.nttp_string;
import base.utilities.type_aliases;
import base.ranges.string_view;
import base.metaprogramming.integer_sequence;
import base.metaprogramming.declval;
import base.memory.lifetimes;

namespace base {

export template <typename T> class Formatter;

template <Integral T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    sign_  = parser.check_sign();
    width_ = parser.check_width();
    type_  = parser.check_option(Character<T> ? 'c' : 'd', "bodxc?");
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept {
    const PaddingOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };

    switch (type_) {
    default:
      [[likely]] call_format_integral<10>(buf, value, opts);
      break;
    case 'x':
      call_format_integral<16>(buf, value, opts);
      break;
    case 'b':
      call_format_integral<2>(buf, value, opts);
      break;
    case 'o':
      call_format_integral<8>(buf, value, opts);
      break;
    case 'c':
      if constexpr (Character<T>) {
        pad_char(buf, value, opts);
      } else if constexpr (sizeof(T) == 1) {
        pad_char(buf, static_cast<char8_t>(value), opts);
      } else if constexpr (sizeof(T) == 2) {
        pad_char(buf, static_cast<char16_t>(value), opts);
      } else {
        pad_char(buf, static_cast<char32_t>(value), opts);
      }
      break;
    case '?':
      if constexpr (Character<T>) {
        pad_debug_char(buf, value, opts);
      } else if constexpr (sizeof(T) == 1) {
        pad_debug_char(buf, static_cast<char8_t>(value), opts);
      } else if constexpr (sizeof(T) == 2) {
        pad_debug_char(buf, static_cast<char16_t>(value), opts);
      } else {
        pad_debug_char(buf, static_cast<char32_t>(value), opts);
      }
      break;
    }
  }

private:
  template <u32 Base>
  void call_format_integral(String& buf, T value, PaddingOptions opts) const noexcept {
    switch (sign_) {
    case ' ':
      if constexpr (UnsignedIntegral<T>) {
        pad_integral<Base, " ">(buf, value, opts);
      } else {
        if (value < 0) {
          pad_integral<Base, "-">(buf, static_cast<MakeUnsigned<T>>(-value), opts);
        } else {
          pad_integral<Base, " ">(buf, static_cast<MakeUnsigned<T>>(value), opts);
        }
      }
      break;
    case '+':
      if constexpr (UnsignedIntegral<T>) {
        pad_integral<Base, "+">(buf, value, opts);
      } else {
        if (value < 0) {
          pad_integral<Base, "-">(buf, static_cast<MakeUnsigned<T>>(-value), opts);
        } else {
          pad_integral<Base, "+">(buf, static_cast<MakeUnsigned<T>>(value), opts);
        }
      }
      break;
    default:
      pad_integral<Base>(buf, value, opts);
      break;
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
  char sign_  = '\0';
  char type_  = 'd';
};

template <> class Formatter<bool> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    width_ = parser.check_width();
    type_  = parser.check_option('a', "ad");
    return parser.end_parse();
  }

  void format(String& buf, bool value) const noexcept {
    PaddingOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    if (type_ == 'a') {
      pad_string<false>(buf, value ? "true"_sv : "false"_sv, opts);
    } else {
      pad_char(buf, static_cast<char>('0' + static_cast<int>(value)), opts);
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
  char type_  = 'a';
};

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

  void format(String& buf, T value) const noexcept;

private:
  u32  precision_ = 0;
  u32  width_     = 0;
  char fill_      = ' ';
  char align_     = '>';
  char sign_      = '\0';
  char type_      = 'g';
};

template <typename T> class Formatter<T*> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(String& buf, const T* ptr) const noexcept {
    const PaddingOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    pad_integral<16>(buf, reinterpret_cast<uptr>(ptr), opts);
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
};

template <Character Char, Integral SizeT> class Formatter<BasicStringView<Char, SizeT>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    type_  = parser.check_option('s', "s?");
    return parser.end_parse();
  }

  void format(String& buf, BasicStringView<Char, SizeT> str) const noexcept {
    const PaddingOptions opts{.width = width_, .fill = fill_, .align = align_};
    if (type_ == 's') [[likely]] {
      pad_string<false>(buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())),
                        opts);
    } else {
      pad_string<true>(buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())),
                       opts);
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
  char type_  = 's';
};

template <Character Char> class Formatter<Char*> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    type_  = parser.check_option('s', "s?x");
    return parser.end_parse();
  }

  void format(String& buf, const Char* str) const noexcept {
    const PaddingOptions opts{.width = width_, .fill = fill_, .align = align_};
    if (type_ == 's') [[likely]] {
      pad_string<false>(buf, BasicStringView<Char>(str), opts);
    } else if (type_ == '?') {
      pad_string<true>(buf, BasicStringView<Char>(str), opts);
    } else {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      pad_integral<16>(buf, reinterpret_cast<uptr>(str), opts);
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
  char type_  = 's';
};

template <Character Char, usize N, AllocatorLike Allocator, Integral SizeT>
class Formatter<BasicString<Char, N, Allocator, SizeT>> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  void format(String& buf, const BasicString<Char, N, Allocator, SizeT>& str) const noexcept {
    return Formatter<BasicStringView<Char>>::format(buf, str.view());
  }
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
template <Character Char, usize N> class Formatter<Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void format(String& buf, const Char (&str)[N]) const noexcept {
    return Formatter<BasicStringView<Char>>::format(
        buf, BasicStringView(static_cast<const Char*>(str), N - 1));
  }
};

template <Character Char, usize N>
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
class Formatter<const Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void format(String& buf, const Char (&str)[N]) const noexcept {
    return Formatter<BasicStringView<Char>>::format(buf, BasicStringView(str, N - 1));
  }
};

template <RangeLike T>
class Formatter<T> : Formatter<RemoveCVRef<decltype(*declval<T>().begin())>> {
public:
  constexpr const char* parse(const char* fmt) noexcept { return Base::parse(fmt); }

  void format(String& buf, const T& range) const noexcept {
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
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept {
    auto           scalar = static_cast<UnderlyingType>(value);
    auto           u_val  = static_cast<MakeUnsigned<UnderlyingType>>(value);
    PaddingOptions opts   = {
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    if (u_val < 256) {
      pad_string<false>(buf, TABLE[u_val], opts);
      return;
    }
    pad_integral<10, NTTPString<CAST_STRING.size()>(CAST_STRING)>(buf, scalar, opts);
  }

private:
  static constexpr StackArray<char, TYPE_STRING<T>.size() + 2> CAST_STRING_ARR = []() {
    StackArray<char, TYPE_STRING<T>.size() + 2> arr{};
    arr[0]              = '(';
    arr[arr.size() - 1] = ')';
    copy_bytes(static_cast<void*>(&arr[1]), TYPE_STRING<T>.data(), TYPE_STRING<T>.size());
    return arr;
  }();

  static constexpr StringView CAST_STRING{CAST_STRING_ARR.data(), CAST_STRING_ARR.size()};

  using UnderlyingType = UnderlyingEnumType<T>;
  template <u32... Values>
  static constexpr StackArray<StringView, sizeof...(Values)>
  make_table(IntegerSequence<u32, Values...> /*sequence*/) {
    StackArray<StringView, sizeof...(Values)> ret;
    (..., [&]() { ret[Values] = table_entry<T(Values)>(); }());
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

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
};

} // namespace base
