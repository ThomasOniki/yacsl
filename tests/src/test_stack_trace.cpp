#include "test_utils.hpp"
#include "yacsl/attributes.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/ranges/string_view.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_ENTRY(entry_idx, expected_caller)                                \
  EXPECT_EQ(stack_trace.entries()[entry_idx].caller.view(),                    \
            yacsl::ranges::StringView(expected_caller))

YACSL_NOINLINE static int f1(bool /*unused*/, char /*unused*/) {
  int                            ret = 0;
  yacsl::debug::StackTrace const stack_trace =
      yacsl::debug::StackTrace::current();
  stack_trace.compute_entries();
  CHECK_ENTRY(0, "yacsl::debug::StackTrace::current()");
  CHECK_ENTRY(1, "f1(bool, char)");
  CHECK_ENTRY(2, "ns1::ns2::f2(void*)");
  CHECK_ENTRY(
      3, "int ns1::ns2::(anonymous namespace)::S<void, 0>::f3<void, 15>()");
  CHECK_ENTRY(
      4,
      "int (anonymous namespace)::f4<ns1::ns2::(anonymous namespace)::S<void, "
      "0>{(void*)0, (char)97}>()");
  return ret;
}

namespace ns1::ns2 {
YACSL_NOINLINE static int f2(void* /*unused*/) {
  volatile const int result = ::f1(true, 'a');
  return result;
}

namespace {
template <typename T = void, int I = 0> struct S {
  template <typename U = void, int J = 15> YACSL_NOINLINE static int f3() {
    volatile const int result = f2(nullptr);
    return result;
  }

  alignas(16) void* p = nullptr;
  char chr            = 'a';
};
} // namespace
} // namespace ns1::ns2

namespace {
// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
template <ns1::ns2::S<> S = ns1::ns2::S<>{}> YACSL_NOINLINE int f4() noexcept {
  volatile const int result = ns1::ns2::S<>::f3();
  return result;
}
} // namespace

int main() {
  int ret = 0;
  EXPECT(f4() == 0);
  yacsl::debug::flush_logger();
  return ret;
}
