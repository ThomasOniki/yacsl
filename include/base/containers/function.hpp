#pragma once

#include "base/containers/base_classes.hpp"
#include "base/memory/arena.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename F, usize BufSz = 8, AllocatorLike Allocator = MainArenaAllocator> class Function;

template <typename Ret, typename... Args, usize BufSz, AllocatorLike Allocator>
class Function<Ret(Args...), BufSz, Allocator> : AllocatorMember<Allocator>,
                                                 FixedBufferMember<char, 16, usize> {
public:
  Function(auto&& func);

  Function(Function const&)     = delete;
  Function(Function&&) noexcept = delete;

  ~Function() noexcept { cleanup_(data_, allocator()); }

  Function& operator=(this Function&, Function const&) = delete;
  Function& operator=(this Function&, Function&&)      = delete;

  template <typename Self> Ret operator()(this Self&& self, Args... args) {
    return base::forward<Self>(self).call_(self.data_, base::forward<Args>(args)...);
  }

  [[nodiscard]] auto const& get_allocator() const noexcept { return allocator(); }

private:
  using AllocatorMember<Allocator>::allocator;
  using FixedBufferMember<char, 16, usize>::fixed_buffer_;

  void* data_ = fixed_buffer_.data();

  void (*cleanup_)(void* func, UnderlyingAllocator<Allocator>& alloc) = nullptr;
  Ret (*call_)(void* func, Args... args)                              = nullptr;
};

template <typename Ret, typename... Args, usize BufSz, AllocatorLike Allocator>
Function<Ret(Args...), BufSz, Allocator>::Function(auto&& func) {
  using Func = RemoveReference<decltype(func)>;

  if constexpr (sizeof(Func) > sizeof(uchar) * BufSz || alignof(Func) > 8) {
    data_ = allocator().template allocate<Func>(1).data();

    cleanup_ = [](void* data, UnderlyingAllocator<Allocator>& alloc) {
      static_cast<Func*>(data)->~Func();
      alloc.template free<Func>({static_cast<Func*>(data), 1});
    };
  } else {
    cleanup_ = [](void* data, UnderlyingAllocator<Allocator>& /*allocator*/) {
      static_cast<Func*>(data)->~Func();
    };
  }

  base::construct_at(static_cast<Func*>(data_), base::forward<decltype(func)>(func));

  call_ = [](void* data, Args... args) {
    return (*static_cast<Func*>(data))(base::forward<Args>(args)...);
  };
}

#ifdef BASE_LSP_ONLY
template Function<void(), 0>::Function(decltype([]() {})& func);
template Function<void()>::Function(decltype([]() {})&& func);
#endif

} // namespace base
