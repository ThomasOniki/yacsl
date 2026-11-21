#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/demangle.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include <dlfcn.h>
#include <unwind.h>

namespace yacsl::debug {

StackTrace StackTrace::current() noexcept {
  auto callback = [](_Unwind_Context* ctx, void* pvoid) -> _Unwind_Reason_Code {
    StackTrace& self = *static_cast<StackTrace*>(pvoid);
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    self.addresses_.push(reinterpret_cast<void*>(_Unwind_GetRegionStart(ctx)));
    return _URC_NO_REASON;
  };

  StackTrace ret;
  _Unwind_Backtrace(callback, &ret);
  return ret;
}

containers::Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace const& self) noexcept {
  if (self.entries_.len() == self.addresses_.len()) {
    return self.entries_;
  }
  self.entries_.resize(self.addresses_.len());
  for (usize i = 0; i < self.addresses_.len(); ++i) {
    Dl_info info;
    if (dladdr(self.addresses_[i], &info) != 0) {
      self.entries_[i] = {
          .caller =
              info.dli_sname != nullptr ? debug::demangle(info.dli_sname) : "",
          .object       = ranges::StringView(info.dli_fname),
          .base_address = info.dli_fbase,
          .address      = self.addresses_[i],
      };
    }
  }
  return self.entries_;
}

} // namespace yacsl::debug

#endif
