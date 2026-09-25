#pragma once

namespace yacsl::type_traits {

#ifdef __clang__
template <typename T>
concept TriviallyDestructible = __is_trivially_destructible(T);
#else
template <typename T>
concept TriviallyDestructible = __has_trivial_destructor(T);
#endif

template <typename T>
concept TriviallyCopyable = __is_trivially_copyable(T);

} // namespace yacsl::type_traits
