#pragma once

#include "base/logging/errors.hpp"
#include <string.h>

#define EXPECT_EQ(lhs, rhs)                                                                        \
	do {                                                                                             \
		if ((lhs) != (rhs)) {                                                                          \
			base::log_error("EXPECT_EQ failed. Expected " #lhs " == " #rhs ", got\n{}\nand\n{}\n.", lhs, \
											rhs);                                                                        \
			ret = 1;                                                                                     \
		}                                                                                              \
	} while (false)

#define EXPECT_STREQ(lhs, rhs)                                                                     \
	do {                                                                                             \
		auto&& lhs_v = lhs;                                                                            \
		auto&& rhs_v = rhs;                                                                            \
		if (strcmp(lhs_v, rhs_v) != 0) {                                                               \
			base::log_error("EXPECT_STREQ failed. Expected " #lhs " == " #rhs ", got\n{}\nand\n{}\n.",   \
											base::StringView(lhs, strlen(lhs)), base::StringView(rhs, strlen(rhs)));     \
			ret = 1;                                                                                     \
		}                                                                                              \
	} while (false)

#define EXPECT(cond)                                                                               \
	do {                                                                                             \
		bool result = cond;                                                                            \
		if (!result) {                                                                                 \
			base::log_error("EXPECT failed. " #cond " == false.");                                       \
			ret = 1;                                                                                     \
		}                                                                                              \
	} while (false)

#define ASSERT(cond)                                                                               \
	do {                                                                                             \
		bool result = cond;                                                                            \
		if (!result) {                                                                                 \
			base::log_critical("ASSERT failed. " #cond " == false.");                                    \
			return -1;                                                                                   \
		}                                                                                              \
	}
