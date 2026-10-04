#pragma once

#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/containers/tuple.hpp"
#include "yacsl/debug/_extern_functions.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/fmt/formatter.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/integer_sequence.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::fmt::internal {

template <typename Indices, typename... Args> class FormatStringImpl;
template <usize... Indices, typename... Args>
class FormatStringImpl<meta::IndexSequence<Indices...>, Args...> {
public:
  consteval FormatStringImpl(
      char const*           fmt,
      debug::SourceLocation location = debug::SourceLocation::current())
      : location_(location) {
    (..., [&] {
      char const* segment_begin = fmt;
      for (; *fmt != '{'; ++fmt) {
        if (*fmt == '\0') {
          debug::internal::extern_log_error(
              "Too few format specifiers for the argument count.");
        }
      }
      segments_[Indices] = {segment_begin, fmt};
      fmt                = formatters_.template get<Indices>().parse(fmt);
    }());

    char const* segment_begin = fmt;
    for (; *fmt != '\0'; ++fmt) {
      if (*fmt == '{') {
        debug::internal::extern_log_error(
            "Too many format specifiers for the arguments count.");
      }
    }
    segments_[sizeof...(Args)] = {segment_begin, fmt};
  }

  template <typename... FmtArgs>
  static void format_to(containers::String& buf, FormatStringImpl const& fmt,
                        FmtArgs&&... args) {
    (..., [&] {
      buf += fmt.segments_[Indices];
      fmt.formatters_.template get<Indices>().format(
          buf, memory::forward<FmtArgs>(args));
    }());
    buf += fmt.segments_[sizeof...(Args)];
  }

  [[nodiscard]] constexpr debug::SourceLocation const&
  source_location(this FormatStringImpl const& self) noexcept {
    return self.location_;
  }

public:
  using TypeIdentity = FormatStringImpl;

private:
  mutable containers::Tuple<Formatter<Args>...> formatters_;

  containers::StackArray<ranges::StringView, sizeof...(Args) + 1> segments_;

  debug::SourceLocation location_;
};

template <> class FormatStringImpl<meta::IndexSequence<>> {
public:
  consteval FormatStringImpl(
      char const*           msg,
      debug::SourceLocation location = debug::SourceLocation::current())
      : location_(location), msg_{msg} {}

  static void format_to(containers::String& buf, FormatStringImpl const& fmt) {
    buf += fmt.msg_;
  }

  [[nodiscard]] constexpr debug::SourceLocation const&
  source_location(this FormatStringImpl const& self) noexcept {
    return self.location_;
  }

public:
  using TypeIdentity = FormatStringImpl;

private:
  debug::SourceLocation location_;
  ranges::StringView    msg_;
};

} // namespace yacsl::fmt::internal

namespace yacsl::fmt {

template <typename... Args>
using FormatString =
    internal::FormatStringImpl<meta::IndexSequenceToN<sizeof...(Args)>,
                               meta::RemoveCVRef<Args>...>::TypeIdentity;

template <typename... Args>
void format_to(containers::String& buf, FormatString<Args...> const& fmt,
               Args&&... args) {
  FormatString<Args...>::format_to(buf, fmt, memory::forward<Args>(args)...);
}

template <typename... Args>
containers::String format(FormatString<Args...> const& fmt, Args&&... args) {
  containers::String buf;
  FormatString<Args...>::format_to(buf, fmt, memory::forward<Args>(args)...);
  return buf;
}

} // namespace yacsl::fmt
