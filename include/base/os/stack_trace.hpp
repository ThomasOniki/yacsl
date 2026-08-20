#pragma once

#ifdef _WIN32
#include "base/threading/mutex.hpp"
#include <assert.h>
#include <libloaderapi.h>
#include <windows.h>
#else

#include <dlfcn.h>
#ifndef __EMSCRIPTEN__
#include <execinfo.h>
#endif

#include "base/utilities/demangle.hpp"

#endif

#include "base/attributes.hpp"
#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/logging/logger.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

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
#ifdef _WIN32
    for (usize i = 0; i < addresses_.size(); ++i) {
      entries_[i] = {
          .caller       = String(""),
          .object       = ""_sv,
          .base_address = nullptr,
          .address      = addresses_[i],
      };
    }
#else
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
#endif
    return entries_;
  }

  void print(Logger::Level lvl) noexcept {
    compute_entries();
    for (usize i = 0; i < entries_.size(); ++i) {
      base::log(lvl, "#{} {} in {} ({}+{#x})", i, entries_[i].address, entries_[i].caller,
                entries_[i].object,
                static_cast<uchar*>(entries_[i].address) -
                    static_cast<uchar*>(entries_[i].base_address));
    }
  }

#ifdef __EMSCRIPTEN__
  BASE_NOINLINE static StackTrace current() noexcept { return StackTrace(); }
#elif defined(_WIN32)
  BASE_NOINLINE static StackTrace current() noexcept {
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
#else
  BASE_NOINLINE static StackTrace current() noexcept {
    StackTrace ret;
    ret.addresses_.resize(ret.addresses_.capacity());
    auto size = static_cast<usize>(
        backtrace(ret.addresses_.data(), static_cast<int>(ret.addresses_.size())));
    while (size == ret.addresses_.size()) {
      ret.addresses_.resize(ret.addresses_.capacity() * 2);
      size = static_cast<usize>(
          backtrace(ret.addresses_.data(), static_cast<int>(ret.addresses_.size())));
    }
    ret.addresses_.resize(size);
    return ret;
  }
#endif

  alignas(128) Vector<StackTraceEntry, 16> entries_;
  Vector<void*, 21> addresses_;

#ifdef _WIN32
private:
  inline static Mutex  dbghelp_mutex_;
  inline static HANDLE dbghelp_process_ = nullptr;
#endif
};

} // namespace base
