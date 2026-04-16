#pragma once

#include "base/logging/common.hpp"
#include "base/logging/logger.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/terminate.hpp"

namespace base {

template <typename... Args> void panic(const FormatString<Args...>& fmt, Args&&... args) {
	Logger::get_default_instance().critical(fmt, forward<Args>(args)...);
	Logger::flush();
	terminate();
}

} // namespace base
