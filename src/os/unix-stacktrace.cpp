#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include "base/containers/vector.hpp"
#include "base/os/stack_trace.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include "base/utils/demangle.hpp"
#include <dlfcn.h>
#include <execinfo.h>

namespace base {

StackTrace StackTrace::current() noexcept {
  StackTrace ret;
  ret.addresses_.resize(ret.addresses_.capacity());
  auto size = static_cast<usize>(backtrace(
      ret.addresses_.data(), static_cast<int>(ret.addresses_.size())));
  while (size == ret.addresses_.size()) {
    ret.addresses_.resize(ret.addresses_.capacity() * 2);
    size = static_cast<usize>(backtrace(
        ret.addresses_.data(), static_cast<int>(ret.addresses_.size())));
  }
  ret.addresses_.resize(size);
  return ret;
}

Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace& self) noexcept {
  if (self.entries_.size() == self.addresses_.size()) {
    return self.entries_;
  }
  self.entries_.resize(self.addresses_.size());
  for (usize i = 0; i < self.addresses_.size(); ++i) {
    Dl_info info;
    dladdr(self.addresses_[i], &info);
    self.entries_[i] = {
        .caller       = demangle(info.dli_sname),
        .object       = StringView(info.dli_fname),
        .base_address = info.dli_fbase,
        .address      = self.addresses_[i],
    };
  }
  return self.entries_;
}

} // namespace base

#endif
