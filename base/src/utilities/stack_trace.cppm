module;

#include "base/attributes.hpp"
#include <dlfcn.h>

#ifndef __EMSCRIPTEN__
#include <execinfo.h>
#endif

export module base.utilities.stack_trace;

import base.containers.vector;
import base.containers.string;
import base.memory.lifetimes;
import base.utilities.type_aliases;
import base.utilities.demangle;
import base.ranges.string_view;
import base.logging.logger;
import base.logging.common;

export namespace base {

struct StackTraceEntry {
  alignas(64) String caller;
  StringView object;
  void*      base_address = nullptr;
  void*      address      = nullptr;
};

struct StackTrace {

  // Only call this when you actually need it. Computing the entries is non trivial and can be done
  // at any time as long as this->ptrs_ was gotten at the right time.
  const Vector<StackTraceEntry, 16>& compute_entries() noexcept {
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

  void print(LogLevel lvl) noexcept {
    compute_entries();
    for (usize i = 0; i < entries_.size(); ++i) {
      base::log(lvl, "#{} {} in {}", i, entries_[i].address, entries_[i].caller);
    }
  }

#ifdef __EMSCRIPTEN__
  static StackTrace current() noexcept {
    StackTrace ret;
    return ret;
  }
#else
  BASE_NOINLINE static StackTrace current() noexcept {
    StackTrace ret;
    ret.addresses_.resize(ret.addresses_.capacity());
    usize size = ret.addresses_.size();
    while (size == ret.addresses_.size()) {
      ret.addresses_.resize(ret.addresses_.capacity() * 2);
      size = backtrace(ret.addresses_.data(), static_cast<int>(ret.addresses_.size()));
    }
    ret.addresses_.resize(size);
    return ret;
  }
#endif

  alignas(128) Vector<StackTraceEntry, 16> entries_;
  Vector<void*, 21> addresses_;
};

} // namespace base
