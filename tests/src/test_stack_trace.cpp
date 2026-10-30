#include "yacsl/compiler/attributes.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/testing/expect.hpp"
#include "yacsl/testing/run.hpp"
#include <stdlib.h>

YACSL_NOINLINE static int f1(bool /*unused*/, char /*unused*/) {
  yacsl::debug::StackTrace const stack_trace =
      yacsl::debug::StackTrace::current();
  stack_trace.compute_entries();
  yacsl::testing::expect_eq(stack_trace.entries()[0].caller,
                            "yacsl::debug::StackTrace::current()");
  yacsl::testing::expect_eq(stack_trace.entries()[1].caller, "f1(bool, char)");
  yacsl::testing::expect_eq(stack_trace.entries()[2].caller,
                            "ns1::ns2::f2(void*)");
  yacsl::testing::expect_eq(stack_trace.entries()[3].caller,
                            "int ns1::ns2::(anonymous namespace)::S<void, "
                            "0>::f3<void, 15>()");
  yacsl::testing::expect_eq(
      stack_trace.entries()[4].caller,
      "int (anonymous namespace)::f4<ns1::ns2::(anonymous namespace)::S<void, "
      "0>{(void*)0, (char)97}>()");
  return 0;
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
template <ns1::ns2::S<> S = ns1::ns2::S<>{}>
// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
YACSL_NOINLINE void f4() noexcept {
  [[maybe_unused]] volatile const int result = ns1::ns2::S<>::f3();
}
} // namespace

int main() {
  yacsl::testing::run("stack trace", f4<>);
  yacsl::debug::flush_logger();
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
