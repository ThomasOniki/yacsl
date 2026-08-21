#pragma once

#include "base/attributes.hpp"

#include "base/formatting/atoi.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

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
  explicit constexpr FormatStringParser(BASE_LIFETIMEBOUND char const* fmt);
  constexpr bool                skip(char chr) noexcept;
  [[nodiscard]] constexpr char  check_fill();
  [[nodiscard]] constexpr Align check_align(Align default_align);
  /// returns '\0', ' ' or '+'.
  [[nodiscard]] constexpr Sign check_sign();
  [[nodiscard]] constexpr u32  check_width();
  [[nodiscard]] constexpr u32  check_precision();

  [[nodiscard]] constexpr char check_option(char default_option, StringView possibilities);

  [[nodiscard]] constexpr char const* end_parse();

  static void log_error_no_deps(StringView msg) noexcept;

private:
  alignas(16) char const* fmt_ = nullptr;
};

constexpr FormatStringParser::FormatStringParser(char const* fmt) : fmt_(fmt) {
  if (*fmt_ != '{') {
    log_error_no_deps("FormatStringParser called on a string that doesn't start with '{'.");
  }

  ++fmt_;

  if (*fmt_ == '\0') {
    log_error_no_deps("'{' without '}' in format string.");
  }
}

constexpr bool FormatStringParser::skip(char chr) noexcept {
  if (*fmt_ == chr) {
    ++fmt_;
    return true;
  }
  return false;
}

constexpr char FormatStringParser::check_fill() {
  if (fmt_[0] == '}') {
    if (fmt_[1] == '}') {
      fmt_ += 2;
      return '}';
    }
    return ' ';
  }
  if (*fmt_ == '0' || fmt_[1] == '>' || fmt_[1] == '<' || fmt_[1] == '^') {
    return *fmt_++;
  }
  return ' ';
}

constexpr FormatStringParser::Align FormatStringParser::check_align(Align default_align) {
  if (skip('>')) {
    return Align::Right;
  }
  if (skip('<')) {
    return Align::Left;
  }
  if (skip('^')) {
    return Align::Middle;
  }
  return default_align;
}

constexpr FormatStringParser::Sign FormatStringParser::check_sign() {
  if (skip('+')) {
    return Sign::Plus;
  }
  if (skip(' ')) {
    return Sign::Space;
  }
  skip('-');
  return Sign::Normal;
}

constexpr u32 FormatStringParser::check_width() {
  char const* begin = fmt_;
  fmt_              = num_end(fmt_);
  return atoi<u32>(begin, fmt_);
}

constexpr u32 FormatStringParser::check_precision() {
  if (*fmt_ == '.') {
    ++fmt_;
    return check_width();
  }
  return 0;
}

constexpr char FormatStringParser::check_option(char default_option, StringView possibilities) {
  for (char const possibility : possibilities) {
    if (skip(possibility)) {
      return possibility;
    }
  }
  return default_option;
}

constexpr char const* FormatStringParser::end_parse() {
  if (*fmt_ == '\0') {
    log_error_no_deps("Expected '}' before '\\0' in format string.");
  }
  if (*fmt_ != '}') {
    log_error_no_deps("Invalid format string.");
  }

  ++fmt_;
  return fmt_;
}

} // namespace base
