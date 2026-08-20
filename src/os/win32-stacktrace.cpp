#ifdef _WIN32

#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/os/stack_trace.hpp"
#include "base/utilities/type_aliases.hpp"
#include <libloaderapi.h>
#include <windows.h>

namespace base {

StackTrace StackTrace::current() noexcept {
  StackTrace ret;
  ret.addresses_.resize(ret.addresses_.capacity());
  DWORD size = RtlCaptureStackBackTrace(0, static_cast<DWORD>(ret.addresses_.size()),
                                        ret.addresses_.data(), nullptr);
  while (size == ret.addresses_.size()) {
    ret.addresses_.resize(ret.addresses_.capacity() * 2);
    size = RtlCaptureStackBackTrace(size, static_cast<DWORD>(ret.addresses_.size()) - size,
                                    ret.addresses_.data(), nullptr);
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
    entries_[i] = {
        .caller       = String(""),
        .object       = ""_sv,
        .base_address = nullptr,
        .address      = addresses_[i],
    };
  }
  return entries_;
}

} // namespace base

#endif
