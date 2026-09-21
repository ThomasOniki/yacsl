#pragma once

#include "base/containers/stack_array.hpp"
#include "base/containers/string.hpp"
#include "base/containers/tuple.hpp"
#include "base/debug/source_location.hpp"
#include "base/format/format_string_parser.hpp"
#include "base/format/formatter.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/meta/integer_sequence.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/qualifiers.hpp"

namespace base::format::internal {

template <typename Indices, typename... Args> class FormatStringImpl;
template <usize... Indices, typename... Args>
class FormatStringImpl<meta::IndexSequence<Indices...>, Args...> {
public:
  consteval FormatStringImpl(
      char const*           fmt,
      debug::SourceLocation location = debug::SourceLocation::current())
      : location_(location) {
    (..., [&]() {
      char const* segment_begin = fmt;
      for (; *fmt != '{'; ++fmt) {
        if (*fmt == '\0') {
          FormatStringParser::log_error_no_deps(
              "Too few format specifiers for the argument count.");
        }
      }
      segments_[Indices] = {segment_begin, fmt};
      fmt                = formatters_.template get<Indices>().parse(fmt);
    }());

    char const* segment_begin = fmt;
    for (; *fmt != '\0'; ++fmt) {
      if (*fmt == '{') {
        FormatStringParser::log_error_no_deps(
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

} // namespace base::format::internal

namespace base::format {

template <typename... Args>
using FormatString =
    internal::FormatStringImpl<meta::IndexSequenceToN<sizeof...(Args)>,
                               type_traits::RemoveCVRef<Args>...>::TypeIdentity;

template <typename... Args> // NOLINTNEXTLINE(misc-no-recursion)
void format_to(containers::String& buf, FormatString<Args...> const& fmt,
               Args&&... args) {
  FormatString<Args...>::format_to(buf, fmt, memory::forward<Args>(args)...);
}

} // namespace base::format
