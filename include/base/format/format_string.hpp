#pragma once

#include "base/containers/stack_array.hpp"
#include "base/containers/string.hpp"
#include "base/containers/tuple.hpp"
#include "base/format/format_string_parser.hpp"
#include "base/format/formatter.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/meta/integer_sequence.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base::internal {

template <typename Indices, typename... Args> class FormatStringImpl;
template <usize... Indices, typename... Args>
class FormatStringImpl<IndexSequence<Indices...>, Args...> {
public:
  consteval FormatStringImpl(char const* fmt) {
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
  static void format_to(String& buf, FormatStringImpl const& fmt,
                        FmtArgs&&... args) {
    (..., [&] {
      buf += fmt.segments_[Indices];
      fmt.formatters_.template get<Indices>().format(
          buf, base::forward<FmtArgs>(args));
    }());
    buf += fmt.segments_[sizeof...(Args)];
  }

public:
  using TypeIdentity = FormatStringImpl;

private:
  mutable Tuple<Formatter<Args>...> formatters_;

  StackArray<StringView, sizeof...(Args) + 1> segments_;
};

template <> class FormatStringImpl<IndexSequence<>> {
public:
  consteval FormatStringImpl(char const* msg) : msg_{msg} {}

  static void format_to(String& buf, FormatStringImpl const& fmt) {
    buf += fmt.msg_;
  }

public:
  using TypeIdentity = FormatStringImpl;

private:
  StringView msg_;
};

} // namespace base::internal

namespace base {

template <typename... Args>
using FormatString =
    internal::FormatStringImpl<IndexSequenceToN<sizeof...(Args)>,
                               RemoveCVRef<Args>...>::TypeIdentity;

template <typename... Args> // NOLINTNEXTLINE(misc-no-recursion)
void format_to(String& buf, FormatString<Args...> const& fmt, Args&&... args) {
  FormatString<Args...>::format_to(buf, fmt, base::forward<Args>(args)...);
}

} // namespace base
