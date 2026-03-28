#ifdef BASE_NO_LIBCPP

#include <stdlib.h>

namespace std {

[[noreturn]] void terminate();
[[noreturn]] void terminate() { abort(); }

} // namespace std

#endif
