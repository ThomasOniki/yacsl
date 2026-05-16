#pragma once

#include "base/containers/string_view.hpp"
#include "base/utilities/type_aliases.hpp"
#include <stdio.h>

namespace base {

namespace internal {
template <typename T> constexpr StringView type_string_impl() {
  constexpr StringView pretty_function = __PRETTY_FUNCTION__;
  constexpr usize      offset =
#ifdef BASE_COMPILER_GCC
      StringView("with ").size() +
#endif
      StringView("StringView base::internal::type_string_impl() [T = ").size();
  return {pretty_function.begin() + offset, pretty_function.end() - 1};
}
} // namespace internal

template <typename T> constexpr StringView TYPE_STRING = internal::type_string_impl<T>();

} // namespace base
