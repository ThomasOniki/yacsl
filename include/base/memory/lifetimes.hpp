#pragma once

#include "base/attributes.hpp"

#ifndef BASE_NO_LIBCPP
#include <new>
#endif

#include "base/ranges/slice.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

#ifdef BASE_NO_LIBCPP
[[nodiscard]] constexpr void* operator new(base::usize /*count*/, void* ptr) noexcept {
  return ptr;
}
#else
using ::operator new;
#endif

namespace base {

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

template <typename T> constexpr T* copy_bytes(T* dst, const void* src, usize nbytes) {
  return static_cast<T*>(__builtin_memcpy(dst, src, nbytes));
}
template <typename T> constexpr T* copy_bytes(T* dst, const void* src_begin, const void* src_end) {
  return static_cast<T*>(
      __builtin_memcpy(dst, src_begin,
                       static_cast<usize>(static_cast<const uchar*>(src_end) -
                                          static_cast<const uchar*>(src_begin))));
}

template <typename T, typename SizeT>
constexpr T* copy_bytes(T* dst, Slice<const uchar, SizeT> src) {
  return copy_bytes(dst, src.data(), src.size());
}

} // namespace base
