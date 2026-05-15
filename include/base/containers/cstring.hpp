#pragma once

#include "base/containers/base_clases.hpp"
#include "base/containers/string_view.hpp"
#include "base/memory/arena.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

template <typename... T>
constexpr bool BBB = (... && requires() {
  { T() };
});
static_assert(BBB<int, int>);
static_assert(BBB<>);
static_assert(!BBB<int&, int>);

template <typename Char, usize N = 16, typename Allocator = MainArenaAllocator,
          typename SizeT = usize>
class BasicCString : AllocatorMember<Allocator>, FixedBufferMember<Char, N, SizeT> {
public:
  constexpr BasicCString() noexcept = default;

  template <typename OtherSizeT> // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr BasicCString(const BasicStringView<Char, OtherSizeT>& str) noexcept {
    if (str.size() > N) {
      cstr_ = allocator().template allocate<Char>(str.size() + 1);
    }
    memcpy(cstr_, str.data(), str.size());
    cstr_[str.size()] = '\0';
  }

  BasicCString(const BasicCString& other) = delete;
  BasicCString(BasicCString&& other)      = delete;

  ~BasicCString() noexcept {
    if (cstr_ != fixed_buffer_.data()) {
      allocator().free(cstr_);
    }
  }

  BasicCString& operator=(const BasicCString&) = delete;
  BasicCString& operator=(BasicCString&&)      = delete;

  [[nodiscard]] const Char* data() const noexcept { return cstr_; }
  [[nodiscard]] Char*       data() noexcept { return cstr_; }

private:
  using FixedBufferMember<Char, N, SizeT>::fixed_buffer_;
  using AllocatorMember<Allocator>::allocator;

  Char* cstr_ = fixed_buffer_.data();
};

using CString = BasicCString<char>;

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::BasicCString<char>;
template base::BasicCString<char>::BasicCString(const BasicStringView<char>& str);
#endif
