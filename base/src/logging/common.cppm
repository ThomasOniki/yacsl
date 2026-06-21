export module base.logging.common;

import base.metaprogramming.integer_sequence;
import base.ranges.string_view;
import base.utilities.type_aliases;
import base.containers.stack_array;

export namespace base {

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
