module;

#include "base/attributes.hpp"

#ifndef BASE_NO_LIBCPP
#include <new>
#endif

export module base.memory.lifetimes;

import base.utilities.type_aliases;
import base.type_traits.qualifiers;

#ifdef BASE_NO_LIBCPP
export [[__nodiscard__]] constexpr void* operator new(base::usize /*count*/, void* ptr) noexcept {
  return ptr;
}
#else
using ::operator new;
#endif

export namespace base {

template <typename T> // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
[[nodiscard]] constexpr RemoveReference<T>&& move(BASE_LIFETIMEBOUND T&& value) {
  return static_cast<RemoveReference<T>&&>(value);
}

template <typename T> // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
[[nodiscard]] constexpr T&& forward(BASE_LIFETIMEBOUND RemoveReference<T>&& value) {
  return static_cast<T&&>(value);
}
template <typename T>
[[nodiscard]] constexpr T&& forward(BASE_LIFETIMEBOUND RemoveReference<T>& value) {
  return static_cast<T&&>(value);
}

template <typename T, typename... Args> constexpr T* construct_at(T* location, Args&&... args) {
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  return ::new (location) T(base::forward<Args>(args)...);
}

constexpr void* copy_bytes(void* dst, const void* src, usize nbytes) {
  return __builtin_memcpy(dst, src, nbytes);
}

} // namespace base
