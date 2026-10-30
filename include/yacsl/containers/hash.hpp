#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/declval.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename H, typename T>
concept HashType = requires(H hash, T value) {
  { hash(value) } -> meta::SameAs<typename H::SizeT>;
};

template <typename T, typename SizeT = usize> struct Hash;

template <meta::Arithmetic T, typename S> struct Hash<T, S> {
  using SizeT = S;
  constexpr SizeT operator()(T value) noexcept {
    return static_cast<SizeT>(value);
  }
};

template <meta::RangeLike R, typename S> struct Hash<R, S> {
  using SizeT = S;

  template <typename... Args>
  constexpr Hash(Args&&... args) noexcept
      : elem_hash_(memory::forward<Args>(args)...) {}

  constexpr SizeT operator()(R const& range) noexcept {
    SizeT hashed = 0;
    for (auto const& value : range) {
      hashed ^= elem_hash_(value);
    }
    return hashed;
  }

  using Elem = meta::RemoveCVRef<decltype(*meta::declval<R>().begin())>;
  YACSL_NO_UNIQUE_ADDRESS Hash<Elem> elem_hash_;
};

} // namespace yacsl::containers

// namespace yacsl::containers
