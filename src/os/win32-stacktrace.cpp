#ifdef _WIN32

#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/os/stack_trace.hpp"
#include "base/utilities/type_aliases.hpp"
#include "win32-utils.hpp"
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <windows.h>
#include <winnt.h>

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

Vector<StackTraceEntry, 16> const& StackTrace::compute_entries() noexcept {
  if (entries_.size() == addresses_.size()) {
    return entries_;
  }
  entries_.resize(addresses_.size());
  for (usize i = 0; i < addresses_.size(); ++i) {
    HMODULE module = nullptr;
    BASE_TRY_SYSCALL(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                        static_cast<LPCSTR>(addresses_[i]), &module));

    String module_name;
    module_name.resize(128);
    DWORD len =
        GetModuleFileNameA(module, module_name.data(), static_cast<DWORD>(module_name.size()));
    while (len == module_name.size()) {
      module_name.resize(module_name.size() * 2);
      len = GetModuleFileNameA(module, module_name.data(), static_cast<DWORD>(module_name.size()));
    }
    module_name.resize(len);
    panic_if(len == 0, "GetModuleFileNameA failed(): {}", win32_err_str());

    MODULEINFO info = {};
    GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

    FreeLibrary(module);

    entries_[i] = {
        .caller       = String(),
        .object       = base::move(module_name),
        .base_address = info.lpBaseOfDll,
        .address      = addresses_[i],
    };
  }
  return entries_;
}

} // namespace base

#endif
