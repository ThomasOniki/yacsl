#pragma once

#include "yacsl/debug/log.hpp"
#include "yacsl/ranges/string_view.hpp"
#include <string.h>

#define EXPECT_EQ(lhs, rhs)                                                    \
  do {                                                                         \
    if ((lhs) != (rhs)) {                                                      \
      yacsl::debug::log_error(                                                  \
          "EXPECT_EQ failed. Expected {} == {}, got {} and {}.", #lhs, #rhs,   \
          lhs, rhs);                                                           \
      ret = 1;                                                                 \
    }                                                                          \
  } while (false)

#define EXPECT_SVEQ(lhs, rhs)                                                  \
  do {                                                                         \
    if ((lhs) != (rhs)) {                                                      \
      yacsl::debug::log_error(                                                  \
          "EXPECT_SVEQ failed. Expected {} == {}, got {?} and {?}.", #lhs,     \
          #rhs, lhs, rhs);                                                     \
      ret = 1;                                                                 \
    }                                                                          \
  } while (false)

#define EXPECT_STREQ(lhs, rhs)                                                 \
  do {                                                                         \
    char const* _lhs_v = lhs;                                                  \
    char const* _rhs_v = rhs;                                                  \
    if (strcmp(_lhs_v, _rhs_v) != 0) {                                         \
      yacsl::debug::log_error("EXPECT_STREQ failed. Expected " #lhs " == " #rhs \
                             ", got {?} and {?}.",                             \
                             yacsl::ranges::StringView(lhs, strlen(lhs)),       \
                             yacsl::ranges::StringView(rhs, strlen(rhs)));      \
      ret = 1;                                                                 \
    }                                                                          \
  } while (false)

#define EXPECT(cond)                                                           \
  do {                                                                         \
    bool const _expected = cond;                                               \
    if (!_expected) {                                                          \
      yacsl::debug::log_error("EXPECT failed. " #cond " is false.");            \
      ret = 1;                                                                 \
    }                                                                          \
  } while (false)

#define ASSERT(cond)                                                           \
  do {                                                                         \
    bool const _result = cond;                                                 \
    if (!_result) {                                                            \
      yacsl::debug::log_error("ASSERT failed. " #cond " is false.");            \
      return -1;                                                               \
    }                                                                          \
  } while (false)

#ifdef YACSL_LSP_ONLY
namespace yacsl::_lsp {
inline int test_utils_hpp_suppress_unused_headers() {
  int ret = 0;
  EXPECT_EQ(0, 0);
  EXPECT_STREQ("string", "string");
  EXPECT(true);
  ASSERT(true);
  return ret;
}
} // namespace yacsl::_lsp
#endif
