#pragma once

#ifdef _WIN32
#define BASE_TRY_SYSCALL(syscall)                                                                  \
  if (syscall == FALSE)                                                                            \
  [unlikely] panic(#syscall " failed: {}", win32_err_str())
#else
#define BASE_TRY_SYSCALL(syscall)                                                                  \
  if ((syscall) == -1) [[unlikely]]                                                                \
  panic(#syscall " failed: {}", errno_str())
#endif
