#pragma once

#include "yacsl/containers/nttp_string.hpp"

namespace yacsl::meta {

template <containers::NTTPString Tag> struct TaggedType {
  constexpr bool operator==(this TaggedType /*self*/,
                            TaggedType /*other*/) noexcept {
    return true;
  }
};

} // namespace yacsl::meta
