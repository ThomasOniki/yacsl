#ifdef __EMSCRIPTEN__

#include "base/containers/vector.hpp"
#include "base/debug/stack_trace.hpp"

namespace base::debug {

StackTrace StackTrace::current() noexcept { return StackTrace(); }
containers::Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace& self) noexcept {
  self.entries_ = {};
  return self.entries_;
}

} // namespace base::debug

#endif
