#pragma once

#include "yacsl/compiler/pragmas.hpp"
#include <libloaderapi.h>
#include <winnt.h>
#include <winternl.h>

namespace yacsl::os::windows {

template <typename F> inline F* load_nt_function(char const* name) noexcept {
  static HMODULE module = GetModuleHandleW(L"ntdll.dll");
  YACSL_GCC_OR_CLANG_PRAGMA(GCC diagnostic push)
  YACSL_GCC_OR_CLANG_PRAGMA(GCC diagnostic ignored "-Wcast-function-type")
  return reinterpret_cast<F*>(GetProcAddress(module, name));
  YACSL_GCC_OR_CLANG_PRAGMA(GCC diagnostic pop)
}

inline NTSTATUS NtWriteFile(HANDLE FileHandle, HANDLE Event,
                            PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext,
                            PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer,
                            ULONG Length, PLARGE_INTEGER ByteOffset,
                            PULONG Key) {
  return load_nt_function<decltype(NtWriteFile)>("NtWriteFile")(
      FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length,
      ByteOffset, Key);
}
inline NTSTATUS NtReadFile(HANDLE FileHandle, HANDLE Event,
                           PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext,
                           PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer,
                           ULONG Length, PLARGE_INTEGER ByteOffset,
                           PULONG Key) {
  return load_nt_function<decltype(NtReadFile)>("NtReadFile")(
      FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length,
      ByteOffset, Key);
}
inline NTSTATUS NtClose(HANDLE handle) {
  return load_nt_function<decltype(NtClose)>("NtClose")(handle);
}

} // namespace yacsl::os::windows
