#pragma once

#include "yacsl/type_aliases.hpp"

namespace yacsl::memory {

enum class Alignment : usize {};

template <typename T> constexpr Alignment align_of = Alignment{alignof(T)};

} // namespace yacsl::memory
