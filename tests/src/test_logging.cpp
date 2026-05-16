#include "base/containers/initializer_list.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/file_logger.hpp"
#include "base/utilities/type_aliases.hpp"
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
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  FILE* file = fopen("test_logs.txt", "rbe");
  assert(file);
  (void)fread(logs.data(), sizeof(logs), 1, file);
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  (void)fclose(file);

  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  [[maybe_unused]] FILE* truncated = fopen("test_logs.txt", "w+e");
  assert(truncated);
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  (void)fclose(truncated);

  if (memcmp(static_cast<void*>(logs.data()), static_cast<const void*>(buf.data()), buf_size) !=
      0) {
    printf("Logging test failed. Expected\n%s,got\n%s.\n", buf.data(), logs.data());
    status() = 1;
  }
}

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
enum Enum1 : base::u16 {
  Zero        = 0,
  FiveHundred = 500,
};
enum class EnumClass : base::u16 {
  Zero        = 0,
  FiveHundred = 500,
};
} // namespace

int main() {
  base::FileLogger logger("test_logs.txt", base::LogLevel::Trace3);

  logger.trace3("Hello, {}", "world!");
  check("[TRACE 3] Hello, world!");

  logger.trace2("decimal: {}, hex: {x}, binary: {b}, octal: {o}.", 5, 0x3a, 0b101, 075);
  check("[TRACE 2] decimal: 5, hex: 3a, binary: 101, octal: 75.");

  logger.trace1("aligned: {010}, centered: {*^7}, left, hex: {<4x}.", 96, 737, 0xf3a);
  check("[TRACE 1] aligned: 0000000096, centered: **737**, left, hex: f3a .");

  logger.debug("+ sign: {0+10}.", 386);
  check("[DEBUG] + sign: 000000+386.");

  logger.info("int list: {}, hex int list: {08x}", base::InitializerList<int>{0, -1, 20, -35},
              base::InitializerList<int>{0xF, -0x1a, 0x1, 0x0});
  check("[INFO] int list: [0, -1, 20, -35], hex int list: [0000000f, 00000-1a, 00000001, "
        "00000000]");

  logger.warn("centered wstring: |{^10}|", L"hello");
  check("[WARNING] centered wstring: |  hello   |");

  logger.error("debug char: {?}, debug string: {?}", '\n', "\x17\0abcde\n\t");
  check(R"([ERROR] debug char: '\n', debug string: "\x17\0abcde\n\t")");

  logger.panic("unicode char: {}, debug wstring with unicode: {*>3?}", U'😁', L"nerd emoji: 🤓");
  check("[PANIC] unicode char: 😁, debug wstring with unicode: \"nerd emoji: 🤓\"");

  logger.info("char16_t string range: {*^10}",
              base::InitializerList<base::BasicStringView<char16_t>>{
                  u"foo", u"bar", u"🤓", u"⭐",
                  u"The emoji aren't padded correctly because they use 4 UTF-8 chars."});
  check("[INFO] char16_t string range: [***foo****, ***bar****, ***🤓***, ***⭐****, "
        "The emoji aren't padded correctly because they use 4 UTF-8 chars.]");

  logger.info("debug char16_t string: {?}", u"nerd emoji (again): 🤓, newline: \n");
  check("[INFO] debug char16_t string: \"nerd emoji (again): 🤓, newline: \\n\"");

  logger.info("enum formatting: {}", Enum1::Zero);
  check("[INFO] enum formatting: (anonymous namespace)::Zero");
  logger.info("values >= 256 are formatted as casted ints: {}", Enum1::FiveHundred);
  check("[INFO] values >= 256 are formatted as casted ints: ((anonymous namespace)::Enum1)500");
  logger.info("Same for unamed values: {}", Enum1{1});
  check("[INFO] Same for unamed values: ((anonymous namespace)::Enum1)1");

  logger.info("With enum classes: {}, {}, {}", EnumClass::Zero, EnumClass::FiveHundred,
              EnumClass{1});
  check("[INFO] With enum classes: (anonymous namespace)::EnumClass::Zero, ((anonymous "
        "namespace)::EnumClass)500, ((anonymous namespace)::EnumClass)1");

  logger.info("Goodbye, world!");
  check("[INFO] Goodbye, world!");

  (void)remove("test_logs.txt");

  return status();
}
