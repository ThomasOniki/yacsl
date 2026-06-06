#ifdef _WIN32

#pragma once

#include "base/containers/result.hpp"
#include "base/containers/string_view.hpp"
#include "base/sys/common.hpp"
#include "base/sys/file.hpp"
#include <errhandlingapi.h>
#include <intsafe.h>
#include <windef.h>
#include <winnt.h>

namespace base {

bool              strip_file_open_flag(FileOpenFlags* flags, FileOpenFlags flag) noexcept;
int               unix_flags(FileOpenFlags flags) noexcept;
StringView        win32_err_str(DWORD err) noexcept;
inline StringView win32_err_str() noexcept { return win32_err_str(GetLastError()); }

Result<HANDLE, File::OpenError> open_file(const char* path, FileOpenFlags flags) noexcept;

void try_syscall(BOOL ret, StringView function_name) noexcept;

#ifdef BASE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#define BASE_TRY_SYSCALL(syscall) try_syscall(syscall, #syscall)
#pragma clang diagnostic pop
#else
#define BASE_TRY_SYSCALL(syscall) try_syscall(syscall, #syscall)
#endif

} // namespace base

#endif
