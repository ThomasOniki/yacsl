#pragma once

#include "yacsl/meta/same_as.hpp"

namespace yacsl::containers {

template <typename H, typename T>
concept HashType = requires(H hash, T value) {
  { hash(value) } -> meta::SameAs<typename H::SizeT>;
};

} // namespace yacsl::containers
