#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include "base/containers/vector.hpp"
#include "base/os/stack_trace.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/demangle.hpp"
#include "base/utilities/type_aliases.hpp"
#include <dlfcn.h>
#include <execinfo.h>

namespace base {

StackTrace StackTrace::current() noexcept {
  StackTrace ret;
  ret.addresses_.resize(ret.addresses_.capacity());
  auto size =
      static_cast<usize>(backtrace(ret.addresses_.data(), static_cast<int>(ret.addresses_.size())));
  while (size == ret.addresses_.size()) {
    ret.addresses_.resize(ret.addresses_.capacity() * 2);
    size = static_cast<usize>(
        backtrace(ret.addresses_.data(), static_cast<int>(ret.addresses_.size())));
  }
  ret.addresses_.resize(size);
  return ret;
}

const Vector<StackTraceEntry, 16>& StackTrace::compute_entries() noexcept {
  if (entries_.size() == addresses_.size()) {
    return entries_;
  }
  entries_.resize(addresses_.size());
  for (usize i = 0; i < addresses_.size(); ++i) {
    Dl_info info;
    dladdr(addresses_[i], &info);
    entries_[i] = {
        .caller       = demangle(info.dli_sname),
        .object       = StringView(info.dli_fname),
        .base_address = info.dli_fbase,
        .address      = addresses_[i],
    };
  }
  return entries_;
}

} // namespace base

#endif
