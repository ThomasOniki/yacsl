#pragma once

#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"

namespace base::type_traits {

namespace internal {
template <typename T> constexpr ranges::StringView type_string_impl_pretty() {
  return __PRETTY_FUNCTION__;
}

constexpr usize TYPE_STRING_IMPL_OFFSET_FROM_BEGIN = []() -> usize {
  using base::ranges::operator""_sv;
  constexpr ranges::StringView pretty  = type_string_impl_pretty<int>();
  constexpr ranges::StringView to_find = "T = ";
  for (char const* it = pretty.begin(); it < pretty.end() - to_find.size();
       ++it) {
    for (usize i = 0;; ++i) {
      if (i == to_find.size()) {
        return static_cast<usize>(it - pretty.begin()) + i -
               "_pretty"_sv.size();
      }
      if (it[i] != to_find[i]) {
        break;
      }
    }
  }
  return 0;
}();

constexpr usize TYPE_STRING_IMPL_OFFSET_FROM_END = []() -> usize {
  constexpr ranges::StringView pretty    = type_string_impl_pretty<int>();
  usize                        templates = 0;
  for (usize i = TYPE_STRING_IMPL_OFFSET_FROM_BEGIN; i < pretty.size(); ++i) {
    if (pretty[i] == '<') {
      ++templates;
    } else if (pretty[i] == '>') {
      --templates;
    }
    if (templates > 0) {
      continue;
    }
    if (pretty[i] == ',' || pretty[i] == ';' || pretty[i] == ']') {
      return pretty.size() - i;
    }
  }
  return pretty.size();
}();

template <typename T> constexpr ranges::StringView type_string_impl() {
  constexpr ranges::StringView pretty_function = __PRETTY_FUNCTION__;
  return {pretty_function.begin() + TYPE_STRING_IMPL_OFFSET_FROM_BEGIN,
          pretty_function.end() - TYPE_STRING_IMPL_OFFSET_FROM_END};
}
} // namespace internal

template <typename T>
constexpr ranges::StringView TYPE_STRING = internal::type_string_impl<T>();

} // namespace base::type_traits
