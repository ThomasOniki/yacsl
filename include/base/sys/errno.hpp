#pragma once

#include "base/containers/string_view.hpp"

namespace base {

StringView errno_str() noexcept;
StringView errno_str(int err) noexcept;

} // namespace base
