#ifdef _WIN32

#include "../../os/win32/utils.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/type_aliases.hpp"
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <windows.h>
#include <winnt.h>

namespace yacsl::debug {

StackTrace StackTrace::current() noexcept {
  StackTrace ret;
  ret.addresses_.resize(ret.addresses_.capacity());
  DWORD size =
      RtlCaptureStackBackTrace(0, static_cast<DWORD>(ret.addresses_.len()),
                               ret.addresses_.data(), nullptr);
  while (size == ret.addresses_.len()) {
    ret.addresses_.resize(ret.addresses_.capacity() * 2);
    size = RtlCaptureStackBackTrace(
        size, static_cast<DWORD>(ret.addresses_.len()) - size,
        ret.addresses_.data(), nullptr);
  }
  ret.addresses_.resize(size);
  return ret;
}

containers::Vector<StackTraceEntry, 16> const&
StackTrace::compute_entries(this StackTrace const& self) noexcept {
  if (self.entries_.len() == self.addresses_.len()) {
    return self.entries_;
  }
  self.entries_.resize(self.addresses_.len());
  for (usize i = 0; i < self.addresses_.len(); ++i) {
    HMODULE module = nullptr;
    YACSL_TRY_SYSCALL(
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           static_cast<LPCSTR>(self.addresses_[i]), &module));

    containers::String module_name;
    module_name.resize(128);
    DWORD len = GetModuleFileNameA(module, module_name.data(),
                                   static_cast<DWORD>(module_name.len()));
    while (len == module_name.len()) {
      module_name.resize(module_name.len() * 2);
      len = GetModuleFileNameA(module, module_name.data(),
                               static_cast<DWORD>(module_name.len()));
    }
    module_name.resize(len);
    assert_(len > 0, "GetModuleFileNameA failed(): {}", os::win32_err_str());

    MODULEINFO info = {};
    GetModuleInformation(GetCurrentProcess(), module, &info,
                         sizeof(MODULEINFO));

    FreeLibrary(module);

    self.entries_[i] = {
        .caller       = containers::String(),
        .object       = memory::move(module_name),
        .base_address = info.lpBaseOfDll,
        .address      = self.addresses_[i],
    };
  }
  return self.entries_;
}

} // namespace yacsl::debug

#endif
