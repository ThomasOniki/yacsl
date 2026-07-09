import base.logging.logger;
import base.logging.panic;
import base.logging.common;
import base.utilities.stack_trace;
import base.ranges.string_view;

#include "base/attributes.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_ENTRY(entry_idx, expected_caller)                                                    \
  EXPECT_EQ(stack_trace.entries_[entry_idx].caller.view(), base::StringView(expected_caller))

BASE_NOINLINE static int f1(bool /*unused*/, char /*unused*/) {
  int              ret         = 0;
  base::StackTrace stack_trace = base::StackTrace::current();
  stack_trace.compute_entries();
  CHECK_ENTRY(0, "base::StackTrace@base.utilities.stack_trace::current()");
  CHECK_ENTRY(1, "f1(bool, char)");
  CHECK_ENTRY(2, "ns1::ns2::f2(void*)");
  CHECK_ENTRY(3, "int ns1::ns2::(anonymous namespace)::S<void, 0>::f3<void, 15>()");
  CHECK_ENTRY(4, "int (anonymous namespace)::f4<ns1::ns2::(anonymous namespace)::S<void, "
                 "0>{(void*)0, (char)97}>()");
  base::log_info("returning {}", ret);
  return ret;
}

namespace ns1::ns2 {
BASE_NOINLINE static int f2(void* /*unused*/) {
  volatile int result = ::f1(true, 'a');
  return result;
}

namespace {
template <typename T = void, int I = 0> struct S {
  template <typename U = void, int J = 15> BASE_NOINLINE static int f3() {
    volatile int result = f2(nullptr);
    return result;
  }

  alignas(16) void* p = nullptr;
  char chr            = 'a';
};
} // namespace
} // namespace ns1::ns2

namespace {
template <ns1::ns2::S<> S = ns1::ns2::S<>{}> BASE_NOINLINE int f4() noexcept {
  volatile int result = ns1::ns2::S<>::f3();
  return result;
}
} // namespace

int main() {
  int ret = 0;
  EXPECT(f4() == 0);
  return ret;
}
