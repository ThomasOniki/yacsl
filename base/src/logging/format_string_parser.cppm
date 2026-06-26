module;

#include "base/attributes.hpp"

export module base.logging.format_string_parser;

import base.utilities.type_aliases;
import base.ranges.string_view;
import base.strings.atoi;

export namespace base {

class FormatStringParser {
public:
  explicit constexpr FormatStringParser(BASE_LIFETIMEBOUND const char* fmt);
  [[nodiscard]] constexpr char check_fill();
  [[nodiscard]] constexpr char check_align(char default_align);
  /// returns '\0', ' ' or '+'.
  [[nodiscard]] constexpr char check_sign();
  [[nodiscard]] constexpr u32  check_width();
  [[nodiscard]] constexpr u32  check_precision();

  [[nodiscard]] constexpr char check_option(char default_option, StringView possibilities);

  [[nodiscard]] constexpr const char* end_parse();

  static void log_error_no_deps(StringView msg) noexcept;

private:
  alignas(16) const char* fmt_ = nullptr;
};

constexpr FormatStringParser::FormatStringParser(const char* fmt) : fmt_(fmt) {
  if (*fmt_ != '{') {
    log_error_no_deps("FormatStringParser called on a string that doesn't start with '{'.");
  }

  ++fmt_;

  if (*fmt_ == '\0') {
    log_error_no_deps("'{' without '}' in format string.");
  }
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

constexpr char FormatStringParser::check_align(char default_align) {
  return check_option(default_align, "<>^");
}

constexpr char FormatStringParser::check_sign() {
  const char sign = check_option('-', "+- ");
  return sign == '-' ? '\0' : sign;
}

constexpr u32 FormatStringParser::check_width() {
  const char* begin = fmt_;
  fmt_              = num_end(fmt_);
  return atoi<u32>({begin, fmt_});
}

constexpr u32 FormatStringParser::check_precision() {
  if (*fmt_ == '.') {
    ++fmt_;
    return check_width();
  }
  return 0;
}

constexpr char FormatStringParser::check_option(char default_option, StringView possibilities) {
  for (const char possibility : possibilities) {
    if (possibility == *fmt_) {
      ++fmt_;
      return possibility;
    }
  }
  return default_option;
}

constexpr const char* FormatStringParser::end_parse() {
  if (*fmt_ == '\0') {
    log_error_no_deps("Expected '}' before '\0' in format string.");
  }
  if (*fmt_ != '}') {
    log_error_no_deps("Invalid format string.");
  }

  ++fmt_;
  return fmt_;
}

} // namespace base
