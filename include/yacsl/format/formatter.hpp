#pragma once

#include "yacsl/containers/nttp_string.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/format/format_string_parser.hpp"
#include "yacsl/format/itoa.hpp"
#include "yacsl/format/padding.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/declval.hpp"
#include "yacsl/meta/integer_sequence.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/allocator.hpp"
#include "yacsl/type_traits/categories.hpp"
#include "yacsl/type_traits/qualifiers.hpp"
#include "yacsl/type_traits/range_like.hpp"
#include "yacsl/type_traits/sign.hpp"
#include "yacsl/type_traits/type_string.hpp"

namespace yacsl::format {

template <typename T> class Formatter {
public:
  constexpr char const* parse(this Formatter& /*self*/,
                              char const* fmt) noexcept {
    return fmt + 2;
  }
  void format(this Formatter const& /*self*/, containers::String& buf,
              T const& value) noexcept {
    buf += type_traits::TYPE_STRING<T>;
    buf += " at ";
    itoa(buf, reinterpret_cast<uptr>(&value), "0123456789abcdef");
  }
};

template <type_traits::Integral T> class Formatter<T> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Right);
    self.sign_  = parser.check_sign();
    self.is_alternate_form_ = parser.skip('#');
    self.width_             = parser.check_width();
    if (parser.skip('b')) {
      self.type_ = Type::Binary;
    } else if (parser.skip('B')) {
      self.type_ = Type::BinaryUpperCase;
    } else if (parser.skip('o')) {
      self.type_ = Type::Octal;
    } else if (parser.skip('d')) {
      self.type_ = Type::Decimal;
    } else if (parser.skip('x')) {
      self.type_ = Type::Hex;
    } else if (parser.skip('X')) {
      self.type_ = Type::HexUpperCase;
    } else if (parser.skip('c')) {
      self.type_ = Type::Char;
    } else if (parser.skip('?')) {
      self.type_ = Type::DebugChar;
    }
    return parser.end_parse();
  }

  void format(this Formatter const& self, containers::String& buf,
              T value) noexcept {
    PaddingOptions const opts = {
        .width = self.width_,
        .fill  = self.fill_,
        .align = self.align_,
    };

    switch (self.type_) {
    case Type::Binary:
      if (self.is_alternate_form_) {
        self.format_with_sign<"01", "0b">(buf, value, opts);
      } else {
        self.format_with_sign<"01", "">(buf, value, opts);
      }
      break;
    case Type::BinaryUpperCase:
      if (self.is_alternate_form_) {
        self.format_with_sign<"01", "0B">(buf, value, opts);
      } else {
        self.format_with_sign<"01", "">(buf, value, opts);
      }
      break;
    case Type::Octal:
      if (self.is_alternate_form_) {
        self.format_with_sign<"01234567", "0">(buf, value, opts);
      } else {
        self.format_with_sign<"01234567", "">(buf, value, opts);
      }
      break;
    case Type::Decimal:
      [[likely]] self.format_with_sign<"0123456789", "">(buf, value, opts);
      break;
    case Type::Hex:
      if (self.is_alternate_form_) {
        self.format_with_sign<"0123456789abcdef", "0x">(buf, value, opts);
      } else {
        self.format_with_sign<"0123456789abcdef", "">(buf, value, opts);
      }
      break;
    case Type::HexUpperCase:
      if (self.is_alternate_form_) {
        self.format_with_sign<"0123456789ABCDEF", "0X">(buf, value, opts);
      } else {
        self.format_with_sign<"0123456789ABCDEF", "">(buf, value, opts);
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
  template <containers::NTTPString Charset, containers::NTTPString Prefix>
  void format_with_sign(this Formatter const& self, containers::String& buf,
                        T value, PaddingOptions opts) noexcept {
    switch (self.sign_) {
    case FormatStringParser::Sign::Normal:
      [[likely]] format_with_prefix_after_sign<Charset, Prefix, "">(buf, value,
                                                                    opts);
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
  template <containers::NTTPString Charset, containers::NTTPString Prefix,
            containers::NTTPString Plus = "">
  static void format_with_prefix_after_sign(containers::String& buf, T value,
                                            PaddingOptions opts) noexcept {
    if constexpr (type_traits::UnsignedIntegral<T>) {
      pad_integral<Charset, containers::cat_nttpstrings<Plus, Prefix>>(
          buf, value, opts);
    } else {
      if (value >= 0) {
        pad_integral<Charset, containers::cat_nttpstrings<Plus, Prefix>>(
            buf, static_cast<type_traits::MakeUnsigned<T>>(value), opts);
      } else {
        pad_integral<Charset, containers::cat_nttpstrings<"-", Prefix>>(
            buf, static_cast<type_traits::MakeUnsigned<T>>(-value), opts);
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

  u32  width_    = 0;
  char fill_     = ' ';
  Type type_ : 3 = type_traits::Character<T> ? Type::Char : Type::Decimal;
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Right;
  FormatStringParser::Sign  sign_ : 2  = FormatStringParser::Sign::Normal;
  bool                      is_alternate_form_ : 1 = false;
};

template <> class Formatter<bool> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Right);
    self.width_ = parser.check_width();
    if (parser.skip('d')) {
      self.type_ = Type::Numeric;
    }
    parser.skip('s');
    return parser.end_parse();
  }

  void format(this Formatter const& self, containers::String& buf,
              bool value) noexcept {
    using yacsl::ranges::operator""_sv;
    PaddingOptions const opts{
        .width = self.width_,
        .fill  = self.fill_,
        .align = self.align_,
    };
    if (self.type_ == Type::Alpha) {
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

template <type_traits::FloatingPoint T> class Formatter<T> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Right);
    self.sign_  = parser.check_sign();
    self.width_ = parser.check_width();
    if (parser.skip('f')) {
      self.type_ = Type::General;
    } else if (parser.skip('e')) {
      self.type_ = Type::Scientific;
    } else if (parser.skip('a')) {
      self.type_ = Type::Hex;
    }
    parser.skip('g');
    return parser.end_parse();
  }

  void format(this Formatter const& self, containers::String& buf,
              T value) noexcept;

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
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Right);
    self.width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(this Formatter const& self, containers::String& buf,
              T const* ptr) noexcept {
    PaddingOptions const opts{
        .width = self.width_,
        .fill  = self.fill_,
        .align = self.align_,
    };
    pad_integral<"0123456789abcdef", "0x">(buf, reinterpret_cast<uptr>(ptr),
                                           opts);
  }

private:
  u32                       width_     = 0;
  char                      fill_      = ' ';
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Right;
};

template <type_traits::Character Char, type_traits::Integral SizeT>
class Formatter<ranges::BasicStringView<Char, SizeT>> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Left);
    self.width_ = parser.check_width();
    if (parser.skip('?')) {
      self.type_ = Type::Debug;
    }
    parser.skip('s');
    return parser.end_parse();
  }

  void format(this Formatter const& self, containers::String& buf,
              ranges::BasicStringView<Char, SizeT> str) noexcept {
    PaddingOptions const opts{
        .width = self.width_,
        .fill  = self.fill_,
        .align = self.align_,
    };
    if (self.type_ == Type::Normal) [[likely]] {
      pad_string(buf,
                 ranges::BasicStringView<Char>(str.data(),
                                               static_cast<usize>(str.size())),
                 opts);
    } else {
      pad_string<true>(buf,
                       ranges::BasicStringView<Char>(
                           str.data(), static_cast<usize>(str.size())),
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

template <type_traits::Character Char, type_traits::Integral SizeT>
class Formatter<ranges::Slice<Char, SizeT>> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }
  void format(this Formatter const& self, containers::String& buf,
              ranges::Slice<Char, SizeT> str) noexcept {
    return self.impl_.format(buf, {str.data(), str.size()});
  }

private:
  Formatter<ranges::BasicStringView<Char, SizeT>> impl_;
};

template <type_traits::Character Char> class Formatter<Char*> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Left);
    self.width_ = parser.check_width();
    if (parser.skip('?')) {
      self.type_ = Type::Debug;
    } else if (parser.skip('x')) {
      self.type_ = Type::Pointer;
    }
    return parser.end_parse();
  }

  void format(this Formatter const& self, containers::String& buf,
              Char const* str) noexcept {
    PaddingOptions const opts{
        .width = self.width_,
        .fill  = self.fill_,
        .align = self.align_,
    };
    if (self.type_ == Type::CString) [[likely]] {
      pad_string(buf, ranges::BasicStringView<Char>(str), opts);
    } else if (self.type_ == Type::Debug) {
      pad_string<true>(buf, ranges::BasicStringView<Char>(str), opts);
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

template <type_traits::Character Char, usize N,
          type_traits::Allocator A, type_traits::Integral SizeT>
class Formatter<containers::BasicString<Char, N, A, SizeT>> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }
  void format(
      this Formatter const& self, containers::String& buf,
      containers::BasicString<Char, N, A, SizeT> const& str) noexcept {
    return self.impl_.format(buf, str.view());
  }

private:
  Formatter<ranges::BasicStringView<Char>> impl_;
};

template <type_traits::Character Char, usize N> class Formatter<Char[N]> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }
  void format(this Formatter const& self, containers::String& buf,
              Char const (&str)[N]) noexcept {
    return self.impl_.format(
        buf, ranges::BasicStringView(static_cast<Char const*>(str), N - 1));
  }

private:
  Formatter<ranges::BasicStringView<Char>> impl_;
};

template <type_traits::Character Char, usize N> class Formatter<Char const[N]> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }
  void format(this Formatter const& self, containers::String& buf,
              Char const (&str)[N]) noexcept {
    return self.Formatter<ranges::BasicStringView<Char>>::format(
        buf, ranges::BasicStringView(str, N - 1));
  }

private:
  Formatter<ranges::BasicStringView<Char>> impl_;
};

template <type_traits::RangeLike T> class Formatter<T> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }

  void format(this Formatter const& self, containers::String& buf,
              T const& range) noexcept {
    buf += '[';
    auto iter = range.begin();
    auto end  = range.end();
    if (iter != end) {
      self.impl_.format(buf, *iter);
      ++iter;
      for (; iter != end; ++iter) {
        buf += ", ";
        self.impl_.format(buf, *iter);
      }
    }
    buf += ']';
  }

private:
  Formatter<type_traits::RemoveCVRef<decltype(*meta::declval<T>().begin())>>
      impl_;
};

template <type_traits::Enum T> class Formatter<T> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    FormatStringParser parser(fmt);
    self.fill_  = parser.check_fill();
    self.align_ = parser.check_align(FormatStringParser::Align::Left);
    self.width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(this Formatter& self, containers::String& buf, T value) noexcept {
    auto const scalar = static_cast<UnderlyingType>(value);
    auto const u_val =
        static_cast<type_traits::MakeUnsigned<UnderlyingType>>(value);
    PaddingOptions const opts = {
        .width = self.width_,
        .fill  = self.fill_,
        .align = self.align_,
    };
    if (u_val < 256) {
      pad_string(buf, TABLE[u_val], opts);
      return;
    }
    pad_integral<"0123456789", containers::NTTPString<CAST_STRING.size()>(
                                   CAST_STRING)>(buf, scalar, opts);
  }

private:
  static constexpr containers::StackArray<
      char, type_traits::TYPE_STRING<T>.size() + 2>
      CAST_STRING_ARR = []() {
        containers::StackArray<char, type_traits::TYPE_STRING<T>.size() + 2>
            arr{};
        arr[0]              = '(';
        arr[arr.size() - 1] = ')';
        memory::copy(type_traits::TYPE_STRING<T>.slice(), &arr[1]);
        return arr;
      }();

  static constexpr ranges::StringView CAST_STRING{
      CAST_STRING_ARR.data(),
      CAST_STRING_ARR.size(),
  };

  using UnderlyingType = type_traits::UnderlyingEnumType<T>;
  template <u32... Values>
  static constexpr containers::StackArray<ranges::StringView, sizeof...(Values)>
  make_table(meta::IntegerSequence<u32, Values...> /*sequence*/) {
    containers::StackArray<ranges::StringView, sizeof...(Values)> ret;
    (..., [&]() { ret[Values] = table_entry<T(Values)>(); }());
    return ret;
  }

  template <T Value> static constexpr ranges::StringView table_entry_pretty() {
    return __PRETTY_FUNCTION__;
  }

  static constexpr usize TABLE_ENTRY_OFFSET_FROM_BEGIN = []() -> usize {
    constexpr ranges::StringView pretty =
        table_entry_pretty<static_cast<T>(0)>();
    constexpr ranges::StringView to_find = "Value = ";
    for (char const* it = pretty.begin(); it < pretty.end() - to_find.size();
         ++it) {
      for (usize i = 0;; ++i) {
        if (i == to_find.size()) {
          using ranges::operator""_sv;
          return static_cast<usize>(it - pretty.begin()) + i -
                 "_pretty"_sv.size();
        }
        if (it[i] != to_find[i]) {
          break;
        }
      }
    }
    return 0;
  }();

  static constexpr usize TABLE_ENTRY_OFFSET_FROM_END = []() -> usize {
    constexpr ranges::StringView pretty =
        table_entry_pretty<static_cast<T>(0)>();
    usize templates = 0;
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

  template <T Value> static constexpr ranges::StringView table_entry() {
    constexpr ranges::StringView pretty = __PRETTY_FUNCTION__;
    return {pretty.begin() + TABLE_ENTRY_OFFSET_FROM_BEGIN,
            pretty.end() - TABLE_ENTRY_OFFSET_FROM_END};
  }

  static constexpr containers::StackArray<ranges::StringView, 256> TABLE =
      make_table(meta::IntegerSequenceToN<u32, 256>{});

private:
  u32                       width_     = 0;
  char                      fill_      = ' ';
  FormatStringParser::Align align_ : 2 = FormatStringParser::Align::Left;
};

template <> class Formatter<debug::SourceLocation> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }

  void format(this Formatter& self, containers::String& buf,
              debug::SourceLocation const& location) noexcept {
    self.impl_.format(buf, containers::String(location.file) + ":" +
                               itoa(location.line) + " in " +
                               location.function);
  }

private:
  Formatter<containers::String> impl_;
};

} // namespace yacsl::format
