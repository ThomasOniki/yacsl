#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/debug/_extern_functions.hpp"
#include "yacsl/fmt/atoi.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::fmt {

class FormatStringParser {
public:
  enum class Align : u8 {
    Left,
    Middle,
    Right,
  };
  enum class Sign : u8 {
    Normal,
    Plus,
    Space,
  };
  explicit constexpr FormatStringParser(YACSL_LIFETIMEBOUND char const* fmt);
  constexpr bool skip(this FormatStringParser& self, char chr) noexcept;
  [[nodiscard]] constexpr char  check_fill(this FormatStringParser& self);
  [[nodiscard]] constexpr Align check_align(this FormatStringParser& self,
                                            Align default_align);
  /// returns '\0', ' ' or '+'.
  [[nodiscard]] constexpr Sign check_sign(this FormatStringParser& self);
  [[nodiscard]] constexpr u32  check_width(this FormatStringParser& self);
  [[nodiscard]] constexpr u32  check_precision(this FormatStringParser& self);

  [[nodiscard]] constexpr char check_option(this FormatStringParser& self,
                                            char               default_option,
                                            ranges::StringView possibilities);

  [[nodiscard]] constexpr char const* end_parse(this FormatStringParser& self);

private:
  alignas(16) char const* fmt_ = nullptr;
};

constexpr FormatStringParser::FormatStringParser(char const* fmt) : fmt_(fmt) {
  if (*fmt_ != '{') {
    debug::detail::extern_log_error(
        "FormatStringParser called on a string that doesn't start with '{'.");
  }

  ++fmt_;

  if (*fmt_ == '\0') {
    debug::detail::extern_log_error("'{' without '}' in format string.");
  }
}

constexpr bool FormatStringParser::skip(this FormatStringParser& self,
                                        char                     chr) noexcept {
  if (*self.fmt_ == chr) {
    ++self.fmt_;
    return true;
  }
  return false;
}

constexpr char FormatStringParser::check_fill(this FormatStringParser& self) {
  if (self.fmt_[0] == '}') {
    if (self.fmt_[1] == '}') {
      self.fmt_ += 2;
      return '}';
    }
    return ' ';
  }
  if (*self.fmt_ == '0' || self.fmt_[1] == '>' || self.fmt_[1] == '<' ||
      self.fmt_[1] == '^') {
    return *self.fmt_++;
  }
  return ' ';
}

constexpr FormatStringParser::Align
FormatStringParser::check_align(this FormatStringParser& self,
                                Align                    default_align) {
  if (self.skip('>')) {
    return Align::Right;
  }
  if (self.skip('<')) {
    return Align::Left;
  }
  if (self.skip('^')) {
    return Align::Middle;
  }
  return default_align;
}

constexpr FormatStringParser::Sign
FormatStringParser::check_sign(this FormatStringParser& self) {
  if (self.skip('+')) {
    return Sign::Plus;
  }
  if (self.skip(' ')) {
    return Sign::Space;
  }
  self.skip('-');
  return Sign::Normal;
}

constexpr u32 FormatStringParser::check_width(this FormatStringParser& self) {
  char const* begin = self.fmt_;
  self.fmt_         = fmt::num_end(self.fmt_);
  return atoi<u32>(begin, self.fmt_);
}

constexpr u32
FormatStringParser::check_precision(this FormatStringParser& self) {
  if (*self.fmt_ == '.') {
    ++self.fmt_;
    return self.check_width();
  }
  return 0;
}

constexpr char
FormatStringParser::check_option(this FormatStringParser& self,
                                 char                     default_option,
                                 ranges::StringView       possibilities) {
  for (char const possibility : possibilities) {
    if (self.skip(possibility)) {
      return possibility;
    }
  }
  return default_option;
}

constexpr char const*
FormatStringParser::end_parse(this FormatStringParser& self) {
  if (*self.fmt_ == '\0') {
    debug::detail::extern_log_error(
        "Expected '}' before '\\0' in format string.");
  }
  if (*self.fmt_ != '}') {
    debug::detail::extern_log_error("Invalid format string.");
  }

  ++self.fmt_;
  return self.fmt_;
}

} // namespace yacsl::fmt
