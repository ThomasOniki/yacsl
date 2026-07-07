export module base.containers.cstring;

import base.containers.base_classes;
import base.utilities.type_aliases;
import base.type_traits.categories;
import base.memory.arena;
import base.memory.lifetimes;
import base.ranges.string_view;

export namespace base {

template <typename Char, usize N = 16, typename Allocator = MainArenaAllocator,
          Integral SizeT = usize>
class BasicCString : AllocatorMember<Allocator>, FixedBufferMember<Char, N, SizeT> {
public:
  constexpr BasicCString() noexcept = default;

  template <Integral OtherSizeT> // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr BasicCString(BasicStringView<Char, OtherSizeT> str) noexcept {
    if (str.size() > N) {
      cstr_ = allocator().template allocate<Char>(str.size() + 1);
    }
    copy_bytes(cstr_, str.data(), str.size());
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
template base::BasicCString<char>::BasicCString(BasicStringView<char> str);
#endif
