#include "base/utilities/terminate.hpp"
#include <stdlib.h>

namespace base {

[[noreturn]] void terminate() { abort(); }

} // namespace base
