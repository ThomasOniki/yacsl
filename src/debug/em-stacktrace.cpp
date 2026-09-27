#ifdef __EMSCRIPTEN__

#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/stack_trace.hpp"

namespace yacsl::debug {

StackTrace StackTrace::current() noexcept { return StackTrace(); }
containers::Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace const& self) noexcept {
  self.entries_ = {};
  return self.entries_;
}

} // namespace yacsl::debug

#endif
