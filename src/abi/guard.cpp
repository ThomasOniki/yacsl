#ifdef YACSL_NO_LIBCPP

#include "yacsl/compiler/export.hpp"
#include "yacsl/compiler/pragmas.hpp"
#include "yacsl/type_aliases.hpp"
#include <sched.h>

namespace yacsl {

#if defined(__arm__) || defined(__wasm32__) || defined(__ARM64_ARCH_8_32__)
using Guard = u32;
#else
using Guard = u64;
#endif

// NOLINTBEGIN(bugprone-reserved-identifier)

YACSL_GCC_PRAGMA(GCC diagnostic push)
YACSL_GCC_PRAGMA(GCC diagnostic ignored "-Wmissing-declarations")

extern "C" int __cxa_guard_acquire(Guard* guard) noexcept YACSL_API {
  Guard expected = 0;
  while (!__atomic_compare_exchange_n(guard, &expected, 2, false,
                                      __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
    if (expected == 1) {
      return 0;
    }
    sched_yield();
  }
  return 1;
}
extern "C" void __cxa_guard_release(Guard* guard) noexcept YACSL_API {
  __atomic_store_n(guard, 1, __ATOMIC_RELEASE);
}
extern "C" void __cxa_guard_abort(Guard* guard) noexcept YACSL_API {
  __atomic_store_n(guard, 0, __ATOMIC_RELEASE);
}

YACSL_GCC_PRAGMA(GCC diagnostic pop)

// NOLINTEND(bugprone-reserved-identifier)

} // namespace yacsl

#endif
