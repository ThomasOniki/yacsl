#ifdef __EMSCRIPTEN__

#include "base/containers/vector.hpp"
#include "base/os/stack_trace.hpp"

namespace base {

StackTrace                         StackTrace::current() noexcept { return StackTrace(); }
const Vector<StackTraceEntry, 16>& StackTrace::compute_entries() noexcept {
  entries_ = {};
  return entries_;
}

} // namespace base

#endif
