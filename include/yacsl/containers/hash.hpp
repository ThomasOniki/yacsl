#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/declval.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename H, typename T, typename SizeT = usize>
concept HashType = requires(H const hash, T value) {
  { hash(value) } -> meta::SameAs<SizeT>;
};

template <typename T, typename SizeT = usize> struct Hash;

template <meta::Arithmetic T, typename SizeT> struct Hash<T, SizeT> {
  constexpr SizeT operator()(this Hash /*self*/, T value) noexcept {
    return static_cast<SizeT>(value);
  }
};

template <typename T, typename SizeT> struct Hash<T*, SizeT> {
  constexpr SizeT operator()(this Hash const& self, T* ptr) noexcept {
    return self.impl_(reinterpret_cast<uptr>(ptr));
  }

  YACSL_NO_UNIQUE_ADDRESS Hash<uptr> impl_;
};

template <meta::RangeLike R, typename SizeT> struct Hash<R, SizeT> {

  template <typename... Args>
  constexpr Hash(Args&&... args) noexcept
      : elem_hash_(memory::forward<Args>(args)...) {}

  constexpr SizeT operator()(this Hash const& self, R const& range) noexcept {
    SizeT hashed = 0;
    for (auto const& value : range) {
      hashed ^= self.elem_hash_(value);
    }
    return hashed;
  }

  using Elem = meta::RemoveCVRef<decltype(*meta::declval<R>().begin())>;
  YACSL_NO_UNIQUE_ADDRESS Hash<Elem> elem_hash_;
};

} // namespace yacsl::containers

// namespace yacsl::containers
