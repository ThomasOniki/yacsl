#include "yacsl/containers/function.hpp"
#include "yacsl/containers/option.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/fmt/format_string_parser.hpp"
#include "yacsl/fmt/formatter.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/testing/expect.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"
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

template <> class yacsl::fmt::Formatter<Counts> {
public:
  static constexpr char const* parse(char const* fmt) {
    yacsl::fmt::FormatStringParser parser(fmt);
    return parser.end_parse();
  }
  [[maybe_unused]] static void format(yacsl::containers::String& buf,
                                      Counts const&              counts) {
    yacsl::fmt::format_to(
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

  yacsl::ranges::Slice<yacsl::u8, SizeT>
  allocate(this TestAllocator& /*self*/, SizeT size,
           yacsl::memory::Alignment align) {
    ++allocate_call_count;
#ifdef _WIN32
    return {
        static_cast<yacsl::u8*>(
            _aligned_malloc(static_cast<size_t>(align), size)),
        size,
    };
#else
    return {
        static_cast<yacsl::u8*>(
            aligned_alloc(static_cast<size_t>(align), size)),
        size,
    };
#endif
  }
  void deallocate(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> mem) {
    ++free_call_count;
#ifdef _WIN32
    _aligned_free(mem.data());
#else
    free(mem.data());
#endif
  }

  yacsl::containers::Option<yacsl::ranges::Slice<yacsl::u8, SizeT>>
  resize_in_place(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> /*mem*/,
                  SizeT /*new_size*/) {
    return yacsl::utils::NIL;
  }
};
} // namespace

static void check_counts(Counts expected) {
  yacsl::testing::expect_eq(TestStruct::counts, expected);
  TestStruct::reset_counts();
}

static void test_function_runs(yacsl::testing::Test& /*test*/) {
  bool                                ran = false;
  yacsl::containers::Function<void()> func([&ran] { ran = true; });

  yacsl::testing::expect(!ran);
  func();
  yacsl::testing::expect(ran);
}

static void test_moved_lambda(yacsl::testing::Test& /*test*/) {
  {
    check_counts({});

    // test must be non-to const be moved.
    // NOLINTNEXTLINE(misc-const-correctness)
    TestStruct test;

    check_counts({.constructor_call_count = 1});

    yacsl::containers::Function<int(int)> func(
        [test](int value) { return value * 2; });

    check_counts({
        .copy_constructor_call_count = 1,
        .move_constructor_call_count = 1,
        .destructor_call_count       = 1,
    });

    int const result = func(10);
    if (result != 20) {
      (void)fprintf(stderr, "function returned %d, not 20\n", result);
    }
  }
  check_counts({.destructor_call_count = 2});
}

static void test_copied_lambda(yacsl::testing::Test& /*test*/) {
  {
    TestStruct const test;
    check_counts({.constructor_call_count = 1});

    yacsl::containers::Function<void(), 0, TestAllocator> const func(
        [&test] { (void)test; });

    check_counts({});

    if (TestAllocator::allocate_call_count != 1) {
      (void)fprintf(stderr, "Expected 1 allocation, got %u.\n",
                    TestAllocator::allocate_call_count);
    }
    if (TestAllocator::free_call_count != 0) {
      (void)fprintf(stderr, "Expected 0 frees, got %u.\n",
                    TestAllocator::free_call_count);
    }
  }
  check_counts({.destructor_call_count = 1});
}

static void test_ref_capture_lamdba(yacsl::testing::Test& /*test*/) {
  {
    TestStruct const test;
    check_counts({.constructor_call_count = 1});

    auto lambda = [test] {};
    check_counts({.copy_constructor_call_count = 1});

    yacsl::containers::Function<void()> const func(lambda);
    check_counts({.copy_constructor_call_count = 1});
  }
  check_counts({.destructor_call_count = 3});
}

int main() {
  int                           ret   = 0;
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("function runs", test_function_runs),
      yacsl::testing::Test("moved lambda", test_moved_lambda),
      yacsl::testing::Test("copied lambda", test_copied_lambda),
      yacsl::testing::Test("ref capture lambda", test_ref_capture_lamdba),
  };
  for (auto& test : tests) {
    test.run();
  }
  yacsl::debug::flush_logger();
  return ret;
}
