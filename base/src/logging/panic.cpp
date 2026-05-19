#include "base/logging/panic.hpp"
#include <stdlib.h>

namespace base {
[[noreturn]] void terminate() noexcept { abort(); }
} // namespace base
