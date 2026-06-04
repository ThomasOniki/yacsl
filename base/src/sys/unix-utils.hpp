#pragma once

#include "base/containers/result.hpp"
#include "base/containers/string_view.hpp"
#include "base/sys/file.hpp"
#include <errno.h>

namespace base {

bool              strip_file_open_flag(FileOpenFlags* flags, FileOpenFlags flag) noexcept;
int               unix_flags(FileOpenFlags flags) noexcept;
StringView        errno_str(int err) noexcept;
inline StringView errno_str() noexcept { return errno_str(errno); }

Result<FileHandle, File::OpenError> open_file(const char* path, FileOpenFlags flags) noexcept;

void try_syscall(bool success, StringView function_name) noexcept;

#ifdef BASE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#define BASE_TRY_SYSCALL(syscall) try_syscall((syscall) != -1, #syscall)
#pragma clang diagnostic pop
#else
#define BASE_TRY_SYSCALL(syscall) try_syscall((syscall) != -1, #syscall)
#endif

} // namespace base
