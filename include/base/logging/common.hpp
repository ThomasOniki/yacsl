#pragma once

#include "base/containers/stack_array.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

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
    "[TRACE 3] ",          "[TRACE 2] ",
    "[TRACE 1] ",          "[[36mDEBUG[0m] ",
    "[[32mINFO[0m] ",    "[[34mNOTICE[0m] ",
    "[[33mWARNING[0m] ", "[[31mERROR[0m] ",
    "[[41mPANIC[0m] ",   "",
};

#ifndef NDEBUG
constexpr LogLevel DEFAULT_LOG_LEVEL = LogLevel::Trace3;
#else
constexpr LogLevel DEFAULT_LOG_LEVEL = LogLevel::Info;
#endif
} // namespace base
