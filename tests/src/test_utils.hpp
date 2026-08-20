#pragma once

#include "base/logging/logger.hpp"
#include "base/ranges/string_view.hpp"
#include <string.h>

#define EXPECT_EQ(lhs, rhs)                                                                        \
  do {                                                                                             \
    if ((lhs) != (rhs)) {                                                                          \
      base::log_error("EXPECT_EQ failed. Expected {} == {}, got {} and {}.", #lhs, #rhs, lhs,      \
                      rhs);                                                                        \
      ret = 1;                                                                                     \
    }                                                                                              \
  } while (false)

#define EXPECT_SVEQ(lhs, rhs)                                                                      \
  do {                                                                                             \
    if ((lhs) != (rhs)) {                                                                          \
      base::log_error("EXPECT_SVEQ failed. Expected {} == {}, got {?} and {?}.", #lhs, #rhs, lhs,  \
                      rhs);                                                                        \
      ret = 1;                                                                                     \
    }                                                                                              \
  } while (false)

#define EXPECT_STREQ(lhs, rhs)                                                                     \
  do {                                                                                             \
    const char* _lhs_v = lhs;                                                                      \
    const char* _rhs_v = rhs;                                                                      \
    if (strcmp(_lhs_v, _rhs_v) != 0) {                                                             \
      base::log_error("EXPECT_STREQ failed. Expected " #lhs " == " #rhs ", got {?} and {?}.",      \
                      base::StringView(lhs, strlen(lhs)), base::StringView(rhs, strlen(rhs)));     \
      ret = 1;                                                                                     \
    }                                                                                              \
  } while (false)

#define EXPECT(cond)                                                                               \
  do {                                                                                             \
    const bool _expected = cond;                                                                   \
    if (!_expected) {                                                                              \
      base::log_error("EXPECT failed. " #cond " is false.");                                       \
      ret = 1;                                                                                     \
    }                                                                                              \
  } while (false)

#define ASSERT(cond)                                                                               \
  do {                                                                                             \
    const bool _result = cond;                                                                     \
    if (!_result) {                                                                                \
      base::log_error("ASSERT failed. " #cond " is false.");                                       \
      return -1;                                                                                   \
    }                                                                                              \
  } while (false)

#ifdef BASE_LSP_ONLY
namespace base::_lsp {
inline int test_utils_hpp_suppress_unused_headers() {
  int ret = 0;
  EXPECT_EQ(0, 0);
  EXPECT_STREQ("string", "string");
  EXPECT(true);
  ASSERT(true);
  return ret;
}
} // namespace base::_lsp
#endif
