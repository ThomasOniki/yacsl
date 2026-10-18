#pragma once

#ifdef _WIN32

#include "yacsl/compiler/export.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/ranges/string_view.hpp"
#include <errhandlingapi.h>
#include <windef.h>

namespace yacsl::os::windows {

ranges::StringView        win_err_str(DWORD err) noexcept YACSL_API;
inline ranges::StringView win_err_str() noexcept {
  return win_err_str(GetLastError());
}

inline void try_win32(WINBOOL               status,
                      debug::SourceLocation location =
                          debug::SourceLocation::current()) noexcept {
  debug::assert_(status == TRUE, "win32 function failed at {}:\n{}", location,
                 win_err_str());
}
inline void check_win_err(DWORD                 err,
                          debug::SourceLocation location =
                              debug::SourceLocation::current()) noexcept {
  debug::assert_(err == ERROR_SUCCESS, "win32 function failed at {}:\n{}",
                 location, win_err_str(err));
}

} // namespace yacsl::os::windows

#endif
