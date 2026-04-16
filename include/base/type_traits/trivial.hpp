#pragma once

namespace base {

#ifdef BASE_COMPILER_GCC
template <typename T>
concept TriviallyDestructible = __has_trivial_destructor(T);
#else
template <typename T>
concept TriviallyDestructible = __is_trivially_destructible(T);
#endif

template <typename T>
concept TriviallyCopyable = __is_trivially_copyable(T);

} // namespace base
