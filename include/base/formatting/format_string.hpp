#pragma once

#include "base/containers/stack_array.hpp"
#include "base/containers/string.hpp"
#include "base/containers/tuple.hpp"
#include "base/formatting/format_string_parser.hpp"
#include "base/formatting/formatter.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base::internal {

template <typename Indices, typename... Args> class FormatStringImpl;

template <typename FString> class FormatToImpl;

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

  friend class internal::FormatToImpl<FormatStringImpl>;

public:
  using TypeIdentity = FormatStringImpl;

private:
  mutable Tuple<Formatter<RemoveCVRef<Args>>...> formatters_;

  StackArray<StringView, sizeof...(Args) + 1> segments_;
};

template <> class FormatStringImpl<IndexSequence<>> {
public:
  // NOLINTNEXTLINE
  template <usize N>
  consteval FormatStringImpl(char const (&msg BASE_LIFETIMEBOUND)[N]) : msg_{msg} {}

  friend class FormatToImpl<FormatStringImpl>;

public:
  using TypeIdentity = FormatStringImpl;

private:
  StringView msg_;
};

template <usize... Indices, typename... Args>
class FormatToImpl<FormatStringImpl<IndexSequence<Indices...>, Args...>> {
public:
  // NOLINTNEXTLINE(misc-no-recursion)
  static void format_to_impl(String&                                                     buf,
                             FormatStringImpl<IndexSequence<Indices...>, Args...> const& fmt,
                             Args&&... args) {
    (..., [&]() {
      buf += fmt.segments_[Indices];
      fmt.formatters_.template get<Indices>().format(buf, base::forward<Args>(args));
    }());
    buf += fmt.segments_[sizeof...(Args)];
  }
};

template <> class FormatToImpl<FormatStringImpl<IndexSequence<>>> {
public:
  // NOLINTNEXTLINE(misc-no-recursion)
  static void format_to_impl(String& buf, FormatStringImpl<IndexSequence<>> const& msg) {
    buf += msg.msg_;
  }
};

} // namespace base::internal

namespace base {

template <typename... Args>
using FormatString =
    internal::FormatStringImpl<IndexSequenceToN<sizeof...(Args)>, Args...>::TypeIdentity;

template <typename... Args> // NOLINTNEXTLINE(misc-no-recursion)
void format_to(String& buf, FormatString<Args...> const& fmt, Args&&... args) {
  internal::FormatToImpl<FormatString<Args...>>::format_to_impl(buf, fmt,
                                                                base::forward<Args>(args)...);
}

} // namespace base
