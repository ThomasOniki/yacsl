#include "base/containers/stack_array.hpp"
#include "base/format/format_string.hpp"
#include "base/debug/log.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/os/file.hpp"
#include "base/ranges/initializer_list.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"
#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int& status() {
  static int status;
  return status;
}

static void check(char const* expected) {
  static constexpr size_t buf_size     = 4096;
  size_t const            expected_len = strlen(expected);

  assert(expected_len - 2 < buf_size);
  base::StackArray<char, buf_size, size_t> buf = {};
  memcpy(buf.data(), expected, expected_len);
  buf[expected_len] = '\n';

  base::StackArray<char, buf_size> logs = {};
  // NOLINTNEXTLINE(android-cloexec-fopen)
  FILE* file = fopen("test_logs.txt", "rb");
  assert(file);
  (void)fread(logs.data(), sizeof(logs), 1, file);
  // NOLINTNEXTLINE(android-cloexec-fopen)
  (void)fclose(file);

  // NOLINTNEXTLINE(android-cloexec-fopen)
  FILE* truncated = fopen("test_logs.txt", "w");
  assert(truncated);
  // NOLINTNEXTLINE(android-cloexec-fopen)
  (void)fclose(truncated);

  if (memcmp(static_cast<void*>(logs.data()),
             static_cast<void const*>(buf.data()),
             buf_size) != 0) { // NOLINTNEXTLINE(modernize-use-std-print)
    printf("Logging test failed. Expected\n%s,got\n%s.\n", buf.data(),
           logs.data());
    status() = 1;
  }
}

namespace test_ns {
// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class, misc-use-internal-linkage)
enum Enum1 : base::u16 {
  Zero        = 0,
  FiveHundred = 500,
};
// NOLINTNEXTLINE(misc-use-internal-linkage)
enum class EnumClass : base::u16 {
  Zero        = 0,
  FiveHundred = 500,
};
} // namespace test_ns

template <typename... Args>
static void log_to_file(base::Logger::Level                lvl,
                        base::FormatString<Args...> const& fmt,
                        Args&&... args) {
  base::File::Flags const flags = base::File::Flags::Write |
                                  base::File::Flags::Create |
                                  base::File::Flags::Trunc;
  base::Logger logger(base::File::open("test_logs.txt", flags).expect_ok(),
                      base::File::open("test_logs.txt", flags).expect_ok(),
                      base::Logger::Level::Trace3);
  logger.log(lvl, fmt, base::forward<Args>(args)...);
}

using enum base::Logger::Level;

int main() {
  log_to_file(Trace3, "Hello, {}", "world!");
  check("[TRACE 3] Hello, world!");

  log_to_file(Trace2, "decimal: {}, hex: {x}, binary: {b}, octal: {o}.", 5,
              0x3a, 0b101, 075);
  check("[TRACE 2] decimal: 5, hex: 3a, binary: 101, octal: 75.");

  log_to_file(
      Trace1,
      "Alternate forms: {#x}, {#b}, {#o}. Uppercase: {X}, {B}, {#X}, {#B}",
      0x3a, 0b101, 075, 0X3A, 0B101, 0X3A, 0B101);
  check("[TRACE 1] Alternate forms: 0x3a, 0b101, 075. Uppercase: 3A, 101, "
        "0X3A, 0B101");

  log_to_file(Debug, "aligned: {010}, centered: {*^7}, left, binary: {<#8b}.",
              96, 737, 0b1101);
  check("[DEBUG] aligned: 0000000096, centered: **737**, left, binary: 0b1101  "
        ".");

  log_to_file(
      Info, "explicit + sign: {0+10}. space for plus sign: {*^ #9x}, {*^ #9x}",
      386, 0xaf, -0xaf);
  check("[INFO] explicit + sign: 000000+386. space for plus sign: ** 0xaf**, "
        "**-0xaf**");

  log_to_file(Warning, "int list: {}, hex int list: {08x}",
              base::InitializerList<int>{0, -1, 20, -35},
              base::InitializerList<int>{0xF, -0x1a, 0x1, 0x0});
  check("[WARNING] int list: [0, -1, 20, -35], hex int list: [0000000f, "
        "00000-1a, 00000001, "
        "00000000]");

  log_to_file(Error, "centered wstring: |{^10}|", L"hello");
  check("[ERROR] centered wstring: |  hello   |");

  log_to_file(Panic, "debug char: {?}, debug string: {?}", '\n',
              "\x17\0abcde\n\t");
  check(R"([PANIC] debug char: '\n', debug string: "\x17\0abcde\n\t")");

  {
    int var = 0;
    log_to_file(Info, "pointer: {}", &var);
    base::StackArray<char, 256> buf = {};
    // the %p format is implementation defined
    [[maybe_unused]] int const printed =
        snprintf(buf.data(), 256, "[INFO] pointer: %#" PRIxPTR,
                 reinterpret_cast<uintptr_t>(&var));
    assert(printed > 0);
    check(buf.data());
  }

  log_to_file(Info, "unicode char: {}, debug wstring with unicode: {*>3?}",
              U'😁', L"nerd emoji: 🤓");
  check("[INFO] unicode char: 😁, debug wstring with unicode: \"nerd emoji: "
        "🤓\"");

  log_to_file(Info, "char16_t string range: {*^10}",
              base::InitializerList<base::BasicStringView<char16_t>>{
                  u"foo", u"bar", u"🤓", u"⭐",
                  u"The emoji aren't padded correctly because they use 4 UTF-8 "
                  u"chars."});
  check("[INFO] char16_t string range: [***foo****, ***bar****, ***🤓***, "
        "***⭐****, "
        "The emoji aren't padded correctly because they use 4 UTF-8 chars.]");

  log_to_file(Info, "debug char16_t string: {?}",
              u"nerd emoji (again): 🤓, newline: \n");
  check(
      "[INFO] debug char16_t string: \"nerd emoji (again): 🤓, newline: \\n\"");

  log_to_file(Info, "enum format: {}", test_ns::Enum1::Zero);
  check("[INFO] enum format: test_ns::Zero");
  log_to_file(Info, "values >= 256 are formatted as casted ints: {}",
              test_ns::Enum1::FiveHundred);
  check(
      "[INFO] values >= 256 are formatted as casted ints: (test_ns::Enum1)500");
  log_to_file(Info, "Same for unamed values: {}", test_ns::Enum1{1});
  check("[INFO] Same for unamed values: (test_ns::Enum1)1");

  log_to_file(Info, "With enum classes: {}, {}, {}", test_ns::EnumClass::Zero,
              test_ns::EnumClass::FiveHundred, test_ns::EnumClass{1});
  check("[INFO] With enum classes: test_ns::EnumClass::Zero, "
        "(test_ns::EnumClass)500, "
        "(test_ns::EnumClass)1");

  log_to_file(Info, "With padding: {*<30} {*^30} {*>30}", test_ns::Enum1::Zero,
              test_ns::Enum1::Zero, test_ns::Enum1::Zero);
  check("[INFO] With padding: test_ns::Zero***************** "
        "********test_ns::Zero********* "
        "*****************test_ns::Zero");

  log_to_file(Info, "With padding: {*<30} {*^30} {*>30}",
              test_ns::EnumClass::FiveHundred, test_ns::EnumClass::FiveHundred,
              test_ns::EnumClass::FiveHundred);
  check("[INFO] With padding: (test_ns::EnumClass)500******* "
        "***(test_ns::EnumClass)500**** "
        "*******(test_ns::EnumClass)500");

  log_to_file(Info, "Goodbye, world!");
  check("[INFO] Goodbye, world!");

  (void)remove("test_logs.txt");

  return status();
}
