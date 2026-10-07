#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/type_identity.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/is_constant_evaluated.hpp"
#include <string.h>

#ifndef YACSL_NO_LIBCPP
#include <new> // IWYU pragma: keep
#endif

#ifdef YACSL_NO_LIBCPP
[[nodiscard]] constexpr void* operator new(yacsl::usize /*count*/,
                                           void* ptr) noexcept {
  return ptr;
}
#endif

namespace yacsl::memory {

namespace _export_placement_new {
using ::operator new;
} // namespace _export_placement_new

template <typename T>
[[nodiscard]] constexpr meta::RemoveReference<T>&&
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
move(YACSL_LIFETIMEBOUND T&& value) {
  return static_cast<meta::RemoveReference<T>&&>(value);
}

template <typename T>
[[nodiscard]] constexpr T&&
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
forward(YACSL_LIFETIMEBOUND meta::RemoveReference<T>&& value) {
  return static_cast<T&&>(value);
}
template <typename T>
[[nodiscard]] constexpr T&&
forward(YACSL_LIFETIMEBOUND meta::RemoveReference<T>& value) {
  return static_cast<T&&>(value);
}

template <typename T, typename... Args>
constexpr T* construct_at(T* location YACSL_LIFETIMEBOUND, Args&&... args) {
  return ::new (location) T(memory::forward<Args>(args)...);
}

template <meta::MutableType T>
constexpr ranges::Slice<T>
copy(ranges::Slice<meta::TypeIdentity<T> const> src,
     ranges::Slice<T> dst                       YACSL_LIFETIMEBOUND) noexcept {
  usize const size = dst.len() < src.len() ? dst.len() : src.len();
  if (!utils::is_constant_evaluated() && meta::TriviallyCopyable<T>) {
    return {
        static_cast<T*>(::memcpy(dst.data(), src.data(), size * sizeof(T))),
        size,
    };
  }
  for (usize i = 0; i < size; ++i) {
    dst[i] = src[i];
  }
  return {dst.data(), size};
}
template <meta::MutableType T>
constexpr ranges::Slice<T> copy(ranges::Slice<meta::TypeIdentity<T> const> src,
                                T* dst YACSL_LIFETIMEBOUND) noexcept {
  if (!utils::is_constant_evaluated() && meta::TriviallyCopyable<T>) {
    return {
        static_cast<T*>(::memcpy(dst, src.data(), src.len() * sizeof(T))),
        src.len(),
    };
  }
  for (usize i = 0; i < src.len(); ++i) {
    dst[i] = src[i];
  }
  return {dst, src.len()};
}

} // namespace yacsl::memory
