#pragma once

#include "base/containers/nttp_string.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string.hpp"
#include "base/formatting/format_string_parser.hpp"
#include "base/formatting/itoa.hpp"
#include "base/formatting/padding.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/metaprogramming/declval.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/range_like.hpp"
#include "base/type_traits/sign.hpp"
#include "base/type_traits/type_string.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T> class Formatter {
public:
  constexpr const char* parse(const char* fmt) noexcept { return fmt + 2; }
  void                  format(String& buf, const T& value) const noexcept {
    buf += TYPE_STRING<T>;
    buf += " at ";
    itoa(buf, reinterpret_cast<uptr>(&value), "0123456789abcdef");
  }
};

template <Integral T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_              = parser.check_fill();
    align_             = parser.check_align(FormatStringParser::Align::Right);
    sign_              = parser.check_sign();
    is_alternate_form_ = parser.skip('#');
    width_             = parser.check_width();
    if (parser.skip('b')) {
      type_ = Type::Binary;
    } else if (parser.skip('B')) {
      type_ = Type::BinaryUpperCase;
    } else if (parser.skip('o')) {
      type_ = Type::Octal;
    } else if (parser.skip('d')) {
      type_ = Type::Decimal;
    } else if (parser.skip('x')) {
      type_ = Type::Hex;
    } else if (parser.skip('X')) {
      type_ = Type::HexUpperCase;
    } else if (parser.skip('c')) {
      type_ = Type::Char;
    } else if (parser.skip('?')) {
      type_ = Type::DebugChar;
    }
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept {
    const PaddingOptions opts = {
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };

    switch (type_) {
    case Type::Binary:
      if (is_alternate_form_) {
        format_with_sign<"01", "0b">(buf, value, opts);
      } else {
        format_with_sign<"01", "">(buf, value, opts);
      }
      break;
    case Type::BinaryUpperCase:
      if (is_alternate_form_) {
        format_with_sign<"01", "0B">(buf, value, opts);
      } else {
        format_with_sign<"01", "">(buf, value, opts);
      }
      break;
    case Type::Octal:
      if (is_alternate_form_) {
        format_with_sign<"01234567", "0">(buf, value, opts);
      } else {
        format_with_sign<"01234567", "">(buf, value, opts);
      }
      break;
    case Type::Decimal:
      [[likely]] format_with_sign<"0123456789", "">(buf, value, opts);
      break;
    case Type::Hex:
      if (is_alternate_form_) {
        format_with_sign<"0123456789abcdef", "0x">(buf, value, opts);
      } else {
        format_with_sign<"0123456789abcdef", "">(buf, value, opts);
      }
      break;
    case Type::HexUpperCase:
      if (is_alternate_form_) {
        format_with_sign<"0123456789ABCDEF", "0X">(buf, value, opts);
      } else {
        format_with_sign<"0123456789ABCDEF", "">(buf, value, opts);
      }
      break;
    case Type::Char:
      if constexpr (sizeof(T) == 1) {
        pad_char(buf, static_cast<char8_t>(value), opts);
      } else if constexpr (sizeof(T) == 2) {
        pad_char(buf, static_cast<char16_t>(value), opts);
      } else {
        pad_char(buf, static_cast<char32_t>(value), opts);
      }
      break;
    case Type::DebugChar:
      if constexpr (sizeof(T) == 1) {
        pad_debug_char(buf, static_cast<char8_t>(value), opts);
      } else if constexpr (sizeof(T) == 2) {
        pad_debug_char(buf, static_cast<char16_t>(value), opts);
      } else {
        pad_debug_char(buf, static_cast<char32_t>(value), opts);
      }
      break;
    default:
      [[unlikely]] break;
    }
  }

private:
  template <NTTPString Charset, NTTPString Prefix>
  void format_with_sign(String& buf, T value, PaddingOptions opts) const noexcept {
    switch (sign_) {
    case FormatStringParser::Sign::Normal:
      [[likely]] format_with_prefix_after_sign<Charset, Prefix, "">(buf, value, opts);
      break;
    case FormatStringParser::Sign::Space:
      format_with_prefix_after_sign<Charset, Prefix, " ">(buf, value, opts);
      break;
    case FormatStringParser::Sign::Plus:
      format_with_prefix_after_sign<Charset, Prefix, "+">(buf, value, opts);
      break;
    default:
      [[unlikely]] break;
    }
  }

public:
  template <NTTPString Charset, NTTPString Prefix, NTTPString Plus = "">
  static void format_with_prefix_after_sign(String& buf, T value, PaddingOptions opts) noexcept {
    if constexpr (UnsignedIntegral<T>) {
      pad_integral<Charset, cat_nttpstrings<Plus, Prefix>>(buf, value, opts);
    } else {
      if (value >= 0) {
        pad_integral<Charset, cat_nttpstrings<Plus, Prefix>>(
            buf, static_cast<MakeUnsigned<T>>(value), opts);
      } else {
        pad_integral<Charset, cat_nttpstrings<"-", Prefix>>(
            buf, static_cast<MakeUnsigned<T>>(-value), opts);
      }
    }
  }

private:
  enum class Type : u8 {
    Binary,
    BinaryUpperCase,
    Octal,
    Decimal,
    Hex,
    HexUpperCase,
    Char,
    DebugChar,
  };

  u32                       width_                 = 0;
  char                      fill_                  = ' ';
  Type                      type_ : 3              = Character<T> ? Type::Char : Type::Decimal;
  FormatStringParser::Align align_ : 2             = FormatStringParser::Align::Right;
  FormatStringParser::Sign  sign_ : 2              = FormatStringParser::Sign::Normal;
  bool                      is_alternate_form_ : 1 = false;
};

template <> class Formatter<bool> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align(FormatStringParser::Align::Right);
    width_ = parser.check_width();
    if (parser.skip('d')) {
      type_ = Type::Numeric;
    }
    parser.skip('s');
    return parser.end_parse();
  }

  void format(String& buf, bool value) const noexcept {
    const PaddingOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    if (type_ == Type::Alpha) {
      pad_string(buf, value ? "true"_sv : "false"_sv, opts);
    } else {
      pad_char(buf, static_cast<char>('0' + static_cast<int>(value)), opts);
    }
  }

private:
  enum class Type : u8 {
    Alpha,
    Numeric,
  };

  u32                       width_     = 0;
  char                      fill_      = ' ';
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Right;
  Type                      type_ : 1  = Type::Alpha;
};

template <FloatingPoint T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align(FormatStringParser::Align::Right);
    sign_  = parser.check_sign();
    width_ = parser.check_width();
    if (parser.skip('f')) {
      type_ = Type::General;
    } else if (parser.skip('e')) {
      type_ = Type::Scientific;
    } else if (parser.skip('a')) {
      type_ = Type::Hex;
    }
    parser.skip('g');
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept;

private:
  enum class Type : u8 {
    General,
    Fixed,
    Scientific,
    Hex,
  };

  u32                       precision_ = 2;
  u32                       width_     = 0;
  char                      fill_      = ' ';
  Type                      type_ : 2  = Type::General;
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Right;
  FormatStringParser::Sign  sign_ : 2  = FormatStringParser::Sign::Normal;
};

template <typename T> class Formatter<T*> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align(FormatStringParser::Align::Right);
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(String& buf, const T* ptr) const noexcept {
    const PaddingOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    pad_integral<"0123456789abcdef", "0x">(buf, reinterpret_cast<uptr>(ptr), opts);
  }

private:
  u32                       width_     = 0;
  char                      fill_      = ' ';
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Right;
};

template <Character Char, Integral SizeT> class Formatter<BasicStringView<Char, SizeT>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align(FormatStringParser::Align::Left);
    width_ = parser.check_width();
    if (parser.skip('?')) {
      type_ = Type::Debug;
    }
    parser.skip('s');
    return parser.end_parse();
  }

  void format(String& buf, BasicStringView<Char, SizeT> str) const noexcept {
    const PaddingOptions opts{.width = width_, .fill = fill_, .align = align_};
    if (type_ == Type::Normal) [[likely]] {
      pad_string(buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())), opts);
    } else {
      pad_string<true>(buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())),
                       opts);
    }
  }

private:
  enum class Type : u8 {
    Normal,
    Debug,
  };
  u32                       width_     = 0;
  char                      fill_      = ' ';
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Left;
  Type                      type_ : 1  = Type::Normal;
};

template <Character Char, Integral SizeT>
class Formatter<Slice<Char, SizeT>> : Formatter<BasicStringView<Char, SizeT>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char, SizeT>>::parse(fmt);
  }
  void format(String& buf, Slice<Char, SizeT> str) const noexcept {
    return Formatter<BasicStringView<Char, SizeT>>::format(buf, {str.data(), str.size()});
  }
};

template <Character Char> class Formatter<Char*> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align(FormatStringParser::Align::Left);
    width_ = parser.check_width();
    if (parser.skip('?')) {
      type_ = Type::Debug;
    } else if (parser.skip('x')) {
      type_ = Type::Pointer;
    }
    return parser.end_parse();
  }

  void format(String& buf, const Char* str) const noexcept {
    const PaddingOptions opts{.width = width_, .fill = fill_, .align = align_};
    if (type_ == Type::CString) [[likely]] {
      pad_string(buf, BasicStringView<Char>(str), opts);
    } else if (type_ == Type::Debug) {
      pad_string<true>(buf, BasicStringView<Char>(str), opts);
    } else {
      pad_integral<"0123456789abcdef">(buf, reinterpret_cast<uptr>(str), opts);
    }
  }

private:
  enum class Type : u8 {
    CString,
    Debug,
    Pointer,
  };

  u32                       width_     = 0;
  char                      fill_      = ' ';
  Type                      type_ : 2  = Type::CString;
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Left;
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

template <Character Char, usize N> class Formatter<Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  void format(String& buf, const Char (&str)[N]) const noexcept {
    return Formatter<BasicStringView<Char>>::format(
        buf, BasicStringView(static_cast<const Char*>(str), N - 1));
  }
};

template <Character Char, usize N>
class Formatter<const Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
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
    align_ = parser.check_align(FormatStringParser::Align::Left);
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept {
    const auto           scalar = static_cast<UnderlyingType>(value);
    const auto           u_val  = static_cast<MakeUnsigned<UnderlyingType>>(value);
    const PaddingOptions opts   = {
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    if (u_val < 256) {
      pad_string(buf, TABLE[u_val], opts);
      return;
    }
    pad_integral<"0123456789", NTTPString<CAST_STRING.size()>(CAST_STRING)>(buf, scalar, opts);
  }

private:
  static constexpr StackArray<char, TYPE_STRING<T>.size() + 2> CAST_STRING_ARR = []() {
    StackArray<char, TYPE_STRING<T>.size() + 2> arr{};
    arr[0]              = '(';
    arr[arr.size() - 1] = ')';
    copy(TYPE_STRING<T>.slice(), &arr[1]);
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
  u32                       width_     = 0;
  char                      fill_      = ' ';
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Left;
};

} // namespace base
