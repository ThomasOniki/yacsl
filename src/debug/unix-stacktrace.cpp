#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/demangle.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include <dlfcn.h>
#include <execinfo.h>

namespace yacsl::debug {

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

containers::Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace& self) noexcept {
  if (self.entries_.size() == self.addresses_.size()) {
    return self.entries_;
  }
  self.entries_.resize(self.addresses_.size());
  for (usize i = 0; i < self.addresses_.size(); ++i) {
    Dl_info info;
    dladdr(self.addresses_[i], &info);
    self.entries_[i] = {
        .caller       = debug::demangle(info.dli_sname),
        .object       = ranges::StringView(info.dli_fname),
        .base_address = info.dli_fbase,
        .address      = self.addresses_[i],
    };
  }
  return self.entries_;
}

} // namespace yacsl::debug

#endif
