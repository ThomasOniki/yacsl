#include "base/containers/function.hpp"
#include "base/containers/string.hpp"
#include "base/format/format_string.hpp"
#include "base/format/format_string_parser.hpp"
#include "base/format/formatter.hpp"
#include "base/debug/log.hpp"
#include "base/ranges/slice.hpp"
#include "test_utils.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace {
struct Counts {
  alignas(16) uint32_t constructor_call_count = 0;
  uint32_t copy_constructor_call_count        = 0;
  uint32_t move_constructor_call_count        = 0;
  uint32_t destructor_call_count              = 0;

  [[nodiscard]] constexpr bool
  operator==(Counts const&) const noexcept = default;
};
struct TestStruct {
  inline static Counts counts;

  static void reset_counts() { counts = Counts{}; }

  TestStruct() { ++counts.constructor_call_count; }

  TestStruct(TestStruct const& /*other*/) {
    ++counts.copy_constructor_call_count;
  }
  [[maybe_unused]] TestStruct(TestStruct&& /*other*/) noexcept {
    ++counts.move_constructor_call_count;
  }

  ~TestStruct() noexcept { ++counts.destructor_call_count; }

  TestStruct& operator=(TestStruct const&) = delete;
  TestStruct& operator=(TestStruct&&)      = delete;
};
} // namespace

template <> class base::Formatter<Counts> {
public:
  static constexpr char const* parse(char const* fmt) {
    base::FormatStringParser parser(fmt);
    return parser.end_parse();
  }
  [[maybe_unused]] static void format(base::String& buf, Counts const& counts) {
    base::format_to(
        buf,
        "constructors: {}, copy_constructors: {}, move_contructors: {}, "
        "destructors: {}",
        counts.constructor_call_count, counts.copy_constructor_call_count,
        counts.move_constructor_call_count, counts.destructor_call_count);
  }
};

namespace {
class TestAllocator {
public:
  inline static uint32_t allocate_call_count = 0;
  inline static uint32_t free_call_count     = 0;

  using SizeT = size_t;

  bool operator==(this TestAllocator const& /*self*/,
                  TestAllocator const& /*other*/) {
    return true;
  }

  template <typename T> base::Slice<T, SizeT> allocate(SizeT count) {
    ++allocate_call_count;
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    return {static_cast<T*>(malloc(sizeof(T) * count)), count};
  }
  template <typename T> void free(base::Slice<T, SizeT> mem) {
    ++free_call_count;
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    ::free(mem.data());
  }
};
} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_COUNTS(...)                                                      \
  do {                                                                         \
    Counts const expected = Counts{__VA_ARGS__};                               \
    EXPECT_EQ(TestStruct::counts, expected);                                   \
    TestStruct::reset_counts();                                                \
  } while (false)

static int test_function_running() {
  int                    ret = 0;
  bool                   ran = false;
  base::Function<void()> func([&ran]() { ran = true; });

  EXPECT(!ran);
  func();
  EXPECT(ran);
  return ret;
}

static int test_moved_lambda() {
  int ret = 0;
  {
    CHECK_COUNTS(0);

    // test must be non-to const be moved.
    // NOLINTNEXTLINE(misc-const-correctness)
    TestStruct test;

    CHECK_COUNTS(.constructor_call_count = 1);

    base::Function<int(int)> func([test](int value) { return value * 2; });

    CHECK_COUNTS(.copy_constructor_call_count = 1,
                 .move_constructor_call_count = 1, .destructor_call_count = 1);

    int const result = func(10);
    if (result != 20) {
      (void)fprintf(stderr, "function returned %d, not 20", result);
    }
  }
  CHECK_COUNTS(.destructor_call_count = 2);
  return ret;
}

static int test_copied_lambda() {
  int ret = 0;
  {
    TestStruct const test;
    CHECK_COUNTS(.constructor_call_count = 1);

    base::Function<void(), 0, TestAllocator> const func(
        [&test]() { (void)test; });

    CHECK_COUNTS(0);

    if (TestAllocator::allocate_call_count != 1) {
      (void)fprintf(stderr, "Expected 1 allocation, got %u",
                    TestAllocator::allocate_call_count);
    }
    if (TestAllocator::free_call_count != 0) {
      (void)fprintf(stderr, "Expected 0 frees, got %u",
                    TestAllocator::free_call_count);
    }
  }
  CHECK_COUNTS(.destructor_call_count = 1);
  return ret;
}

static int test_ref_capture_lamdba() {
  int ret = 0;
  {
    TestStruct const test;
    CHECK_COUNTS(.constructor_call_count = 1);

    auto lambda = [test]() {};
    CHECK_COUNTS(.copy_constructor_call_count = 1);

    base::Function<void()> const func(lambda);
    CHECK_COUNTS(.copy_constructor_call_count = 1);
  }
  CHECK_COUNTS(.destructor_call_count = 3);

  return ret;
}

int main() {
  int ret = 0;
  EXPECT(test_function_running() == 0);
  EXPECT(test_moved_lambda() == 0);
  EXPECT(test_copied_lambda() == 0);
  EXPECT(test_ref_capture_lamdba() == 0);
  base::Logger::get_default_instance().flush();
  return ret;
}
