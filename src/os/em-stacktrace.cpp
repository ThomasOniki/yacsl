#ifdef __EMSCRIPTEN__

#include "base/containers/vector.hpp"
#include "base/os/stack_trace.hpp"

namespace base::os {

StackTrace StackTrace::current() noexcept { return StackTrace(); }
containers::Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace& self) noexcept {
  self.entries_ = {};
  return self.entries_;
}

} // namespace base::os

#endif
