#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/allocator.hpp"
#include "yacsl/type_traits/qualifiers.hpp"

namespace yacsl::containers {

template <typename F, usize BufSz = 8,
          type_traits::AllocatorType Allocator = memory::MainArenaAllocator>
class Function;

template <typename Ret, typename... Args, usize BufSz,
          type_traits::AllocatorType Allocator>
class Function<Ret(Args...), BufSz, Allocator> {
public:
  Function(auto&& func);

  Function(Function const&)     = delete;
  Function(Function&&) noexcept = delete;

  ~Function() noexcept { cleanup_(data_, allocator_); }

  Function& operator=(this Function&, Function const&) = delete;
  Function& operator=(this Function&, Function&&)      = delete;

  template <typename Self> Ret operator()(this Self&& self, Args... args) {
    return memory::forward<Self>(self).call_(self.data_,
                                             memory::forward<Args>(args)...);
  }

  [[nodiscard]] auto const& get_allocator() const noexcept {
    return allocator_;
  }

private:
  YACSL_NO_UNIQUE_ADDRESS Allocator allocator_;

  alignas(8)
      YACSL_NO_UNIQUE_ADDRESS StackArray<uchar, BufSz, usize> fixed_buffer_;

  void* data_ = fixed_buffer_.data();

  void (*cleanup_)(void* func, Allocator& alloc) = nullptr;
  Ret (*call_)(void* func, Args... args)         = nullptr;
};

template <typename Ret, typename... Args, usize BufSz,
          type_traits::AllocatorType Allocator>
Function<Ret(Args...), BufSz, Allocator>::Function(auto&& func) {
  using Func = type_traits::RemoveReference<decltype(func)>;

  if constexpr (sizeof(Func) > sizeof(uchar) * BufSz || alignof(Func) > 8) {
    data_ = allocator_.template allocate<Func>(1).data();

    cleanup_ = [](void* data, Allocator& alloc) {
      static_cast<Func*>(data)->~Func();
      alloc.template free<Func>({static_cast<Func*>(data), 1});
    };
  } else {
    cleanup_ = [](void* data, Allocator& /*allocator*/) {
      static_cast<Func*>(data)->~Func();
    };
  }

  memory::construct_at(static_cast<Func*>(data_),
                       memory::forward<decltype(func)>(func));

  call_ = [](void* data, Args... args) {
    return (*static_cast<Func*>(data))(memory::forward<Args>(args)...);
  };
}

#ifdef YACSL_LSP_ONLY
template Function<void(), 0>::Function(decltype([]() {})& func);
template Function<void()>::Function(decltype([]() {})&& func);
#endif

} // namespace yacsl::containers
