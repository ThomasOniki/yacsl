#pragma once

#ifdef BASE_NO_LIBCPP

namespace base {

#if __has_builtin(__is_trivially_destructible)
template <typename T>
concept TriviallyDestructible = __is_trivially_destructible(T);
#else
#error No __is_trivially_destructible builtin.
#endif

#if __has_builtin(__is_trivially_copyable)
template <typename T>
concept TriviallyCopyable = __is_trivially_copyable(T);
#else
#error No __is_trivially_copyable builtin.
#endif

} // namespace base

#else

#include <type_traits>

namespace base {

template <typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;
template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

} // namespace base

#endif
