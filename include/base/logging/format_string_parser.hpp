#pragma once

#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/strings/str_to_int.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base

{

class FormatStringParser {
public:
	explicit constexpr FormatStringParser(const char* fmt);
	[[nodiscard]] constexpr char check_fill();
	[[nodiscard]] constexpr char check_align(char default_align);
	/// returns '\0', ' ' or '+'.
	[[nodiscard]] constexpr char check_sign();
	[[nodiscard]] constexpr u32	 check_width();
	[[nodiscard]] constexpr u32	 check_precision();

	[[nodiscard]] constexpr char check_option(char default_option, const StringView& possibilities);

	[[nodiscard]] constexpr const char* end_parse();

private:
	alignas(16) const char* fmt_ = nullptr;
};

constexpr FormatStringParser::FormatStringParser(const char* fmt) : fmt_(fmt) {
	if (*fmt_ != '{') {
		log_error("FormatStringParser called on a string that doesn't start with '{'.");
	}

	++fmt_;

	if (*fmt_ == '\0') {
		log_error("'{' without '}' in format string.");
	}
}

constexpr char FormatStringParser::check_fill() {
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
	fmt_							= num_end(fmt_);
	return str_to_int<u32>({begin, fmt_});
}

constexpr u32 FormatStringParser::check_precision() {
	if (*fmt_ == '.') {
		++fmt_;
		return check_width();
	}
	return 0;
}

constexpr char FormatStringParser::check_option(char							default_option,
																								const StringView& possibilities) {
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
		log_error("Expected '}' before '\0' in format string.");
	}
	if (*fmt_ != '}') {
		log_error("Invalid format string.");
	}

	++fmt_;
	return fmt_;
}

} // namespace base
