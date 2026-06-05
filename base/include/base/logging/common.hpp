#pragma once

#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

namespace internal {
template <typename Indices, typename... Args> class FormatStringImpl;
} // namespace internal

template <typename... Args>
using FormatString =
    internal::FormatStringImpl<IndexSequenceToN<sizeof...(Args)>, Args...>::TypeIdentity;

template <typename... Args> void log_trace3(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_trace2(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_trace1(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_debug(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_info(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_notice(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_warning(const FormatString<Args...>& fmt, Args&&... args);
template <typename... Args> void log_error(const FormatString<Args...>& fmt, Args&&... args);

void log_trace3(StringView msg);
void log_trace2(StringView msg);
void log_trace1(StringView msg);
void log_debug(StringView msg);
void log_info(StringView msg);
void log_notice(StringView msg);
void log_warning(StringView msg);
void log_error(StringView msg);

enum class LogLevel : u8 {
  Trace3  = 0,
  Trace2  = 1,
  Trace1  = 2,
  Debug   = 3,
  Info    = 4,
  Notice  = 5,
  Warning = 6,
  Error   = 7,
  Panic   = 8,
  None    = 9,
};

constexpr StackArray<StringView, 10> LOG_LEVEL_STRINGS = {
    "[TRACE 3] ", "[TRACE 2] ", "[TRACE 1] ", "[DEBUG] ", "[INFO] ",
    "[NOTICE] ",  "[WARNING] ", "[ERROR] ",   "[PANIC] ", "",
};

constexpr StackArray<StringView, 10> COLORED_LOG_LEVEL_STRINGS = {
    "[TRACE 3] ",
    "[TRACE 2] ",
    "[TRACE 1] ",
    "[\x1B[36mDEBUG\x1B[0m] ",
    "[\x1B[32mINFO\x1B[0m] ",
    "[\x1B[34mNOTICE\x1B[0m] ",
    "[\x1B[33mWARNING\x1B[0m] ",
    "[\x1B[31mERROR\x1B[0m] ",
    "[\x1B[41mPANIC\x1B[0m] ",
    "",
};

#ifndef NDEBUG
constexpr LogLevel DEFAULT_LOG_LEVEL = LogLevel::Trace3;
#else
constexpr LogLevel DEFAULT_LOG_LEVEL = LogLevel::Info;
#endif
} // namespace base
