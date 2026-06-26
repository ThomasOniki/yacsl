export module base.logging.format_string;

import base.logging.common;
import base.logging.formatter;
import base.logging.format_string_parser;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.metaprogramming.integer_sequence;
import base.type_traits.qualifiers;
import base.containers.string;
import base.containers.stack_array;
import base.containers.tuple;
import base.ranges.string_view;

namespace base::internal {

template <typename Indices, typename... Args> class FormatStringImpl;

template <typename FString> class FormatToImpl;

template <usize... Indices, typename... Args>
class FormatStringImpl<IndexSequence<Indices...>, Args...> {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval FormatStringImpl(const char* fmt) {
    (..., [&]() {
      const char* segment_begin = fmt;
      for (; *fmt != '{'; ++fmt) {
        if (*fmt == '\0') {
          FormatStringParser::log_error_no_deps(
              "Too few format specifiers for the argument count.");
        }
      }
      segments_[Indices] = {segment_begin, fmt};
      fmt                = formatters_.template get<Indices>().parse(fmt);
    }());

    const char* segment_begin = fmt;
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
  template <usize N> consteval FormatStringImpl(const char (&msg)[N]) : msg_{msg} {}

  friend class FormatToImpl<FormatStringImpl>;

public:
  using TypeIdentity = FormatStringImpl;

private:
  StringView msg_;
};

template <usize... Indices, typename... Args>
class FormatToImpl<FormatStringImpl<IndexSequence<Indices...>, Args...>> {
public:
  static void format_to_impl(String&                                                 buf,
                             const FormatStringImpl<IndexSequence<Indices...>, Args...>& fmt,
                             Args&&... args) {
    (..., [&]() {
      buf.append(fmt.segments_[Indices]);
      fmt.formatters_.template get<Indices>().format(buf, base::forward<Args>(args));
    }());
    buf.append(fmt.segments_[sizeof...(Args)]);
  }
};

template <> class FormatToImpl<FormatStringImpl<IndexSequence<>>> {
public:
  static void format_to_impl(String& buf, const FormatStringImpl<IndexSequence<>>& msg) {
    buf.append(msg.msg_);
  }
};

} // namespace base::internal

export namespace base {

template <typename... Args>
using FormatString =
    internal::FormatStringImpl<IndexSequenceToN<sizeof...(Args)>, Args...>::TypeIdentity;

template <typename... Args>
void format_to(String& buf, const FormatString<Args...>& fmt, Args&&... args) {
  internal::FormatToImpl<FormatString<Args...>>::format_to_impl(buf, fmt,
                                                                base::forward<Args>(args)...);
}

} // namespace base
