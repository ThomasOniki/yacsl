import base.ranges.initializer_list;
import base.ranges.string_view;
import base.containers.stack_array;
import base.logging.logger;
import base.logging.common;
import base.logging.format_string;
import base.sys.common;
import base.sys.file;
import base.utilities.type_aliases;
import base.memory.lifetimes;
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int& status() {
  static int status;
  return status;
}

static void check(const char* expected) {
  static constexpr size_t buf_size     = 4096;
  const size_t            expected_len = strlen(expected);

  assert(expected_len - 2 < buf_size);
  base::StackArray<char, buf_size, size_t> buf = {};
  memcpy(buf.data(), expected, expected_len);
  buf[expected_len] = '\n';

  base::StackArray<char, buf_size> logs = {};
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, android-cloexec-fopen)
  FILE* file = fopen("test_logs.txt", "rb");
  assert(file);
  (void)fread(logs.data(), sizeof(logs), 1, file);
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, android-cloexec-fopen)
  (void)fclose(file);

  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, android-cloexec-fopen)
  FILE* truncated = fopen("test_logs.txt", "w");
  assert(truncated);
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, android-cloexec-fopen)
  (void)fclose(truncated);

  if (memcmp(static_cast<void*>(logs.data()), static_cast<const void*>(buf.data()), buf_size) !=
      0) {
    printf("Logging test failed. Expected\n%s,got\n%s.\n", buf.data(), logs.data());
    status() = 1;
  }
}

namespace test_ns {
// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
enum Enum1 : base::u16 {
  Zero        = 0,
  FiveHundred = 500,
};
enum class EnumClass : base::u16 {
  Zero        = 0,
  FiveHundred = 500,
};
} // namespace test_ns

template <typename... Args>
static void log_to_file(base::LogLevel lvl, const base::FormatString<Args...>& fmt,
                        Args&&... args) {
  const base::FileOpenFlags flags =
      base::FileOpenFlags::Write | base::FileOpenFlags::Create | base::FileOpenFlags::Trunc;
  base::Logger logger(base::File("test_logs.txt", flags), base::File("test_logs.txt", flags),
                      base::LogLevel::Trace3, base::Logger::Flags{});
  logger.log(lvl, fmt, base::forward<Args>(args)...);
}

using enum base::LogLevel;

int main() {
  log_to_file(Trace3, "Hello, {}", "world!");
  check("[TRACE 3] Hello, world!");

  log_to_file(Trace2, "decimal: {}, hex: {x}, binary: {b}, octal: {o}.", 5, 0x3a, 0b101, 075);
  check("[TRACE 2] decimal: 5, hex: 3a, binary: 101, octal: 75.");

  log_to_file(Trace1, "aligned: {010}, centered: {*^7}, left, hex: {<4x}.", 96, 737, 0xf3a);
  check("[TRACE 1] aligned: 0000000096, centered: **737**, left, hex: f3a .");

  log_to_file(Debug, "+ sign: {0+10}.", 386);
  check("[DEBUG] + sign: 000000+386.");

  log_to_file(Info, "int list: {}, hex int list: {08x}", base::InitializerList<int>{0, -1, 20, -35},
              base::InitializerList<int>{0xF, -0x1a, 0x1, 0x0});
  check("[INFO] int list: [0, -1, 20, -35], hex int list: [0000000f, 00000-1a, 00000001, "
        "00000000]");

  log_to_file(Warning, "centered wstring: |{^10}|", L"hello");
  check("[WARNING] centered wstring: |  hello   |");

  log_to_file(Error, "debug char: {?}, debug string: {?}", '\n', "\x17\0abcde\n\t");
  check(R"([ERROR] debug char: '\n', debug string: "\x17\0abcde\n\t")");

  log_to_file(Panic, "unicode char: {}, debug wstring with unicode: {*>3?}", U'😁',
              L"nerd emoji: 🤓");
  check("[PANIC] unicode char: 😁, debug wstring with unicode: \"nerd emoji: 🤓\"");

  log_to_file(Info, "char16_t string range: {*^10}",
              base::InitializerList<base::BasicStringView<char16_t>>{
                  u"foo", u"bar", u"🤓", u"⭐",
                  u"The emoji aren't padded correctly because they use 4 UTF-8 chars."});
  check("[INFO] char16_t string range: [***foo****, ***bar****, ***🤓***, ***⭐****, "
        "The emoji aren't padded correctly because they use 4 UTF-8 chars.]");

  log_to_file(Info, "debug char16_t string: {?}", u"nerd emoji (again): 🤓, newline: \n");
  check("[INFO] debug char16_t string: \"nerd emoji (again): 🤓, newline: \\n\"");

  log_to_file(Info, "enum formatting: {}", test_ns::Enum1::Zero);
  check("[INFO] enum formatting: test_ns::Zero");
  log_to_file(Info, "values >= 256 are formatted as casted ints: {}", test_ns::Enum1::FiveHundred);
  check("[INFO] values >= 256 are formatted as casted ints: (test_ns::Enum1)500");
  log_to_file(Info, "Same for unamed values: {}", test_ns::Enum1{1});
  check("[INFO] Same for unamed values: (test_ns::Enum1)1");

  log_to_file(Info, "With enum classes: {}, {}, {}", test_ns::EnumClass::Zero,
              test_ns::EnumClass::FiveHundred, test_ns::EnumClass{1});
  check("[INFO] With enum classes: test_ns::EnumClass::Zero, (test_ns::EnumClass)500, "
        "(test_ns::EnumClass)1");

  log_to_file(Info, "Goodbye, world!");
  check("[INFO] Goodbye, world!");

  (void)remove("test_logs.txt");

  return status();
}
