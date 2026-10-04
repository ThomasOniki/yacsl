#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/allocator_type.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename F, usize BufSz = 8,
          memory::AllocatorType A = memory::MainArenaAllocator>
class Function;

template <typename Ret, typename... Args, usize BufSz, memory::AllocatorType A>
class Function<Ret(Args...), BufSz, A> {
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
  YACSL_NO_UNIQUE_ADDRESS A allocator_;

  alignas(8)
      YACSL_NO_UNIQUE_ADDRESS StackArray<uchar, BufSz, usize> fixed_buffer_;

  void* data_ = fixed_buffer_.data();

  void (*cleanup_)(void* func, A& alloc) = nullptr;
  Ret (*call_)(void* func, Args... args) = nullptr;
};

template <typename Ret, typename... Args, usize BufSz, memory::AllocatorType A>
Function<Ret(Args...), BufSz, A>::Function(auto&& func) {
  using Func = meta::RemoveReference<decltype(func)>;

  if constexpr (sizeof(Func) > sizeof(uchar) * BufSz || alignof(Func) > 8) {
    data_ = allocator_.allocate(sizeof(Func), memory::align_of<Func>).data();

    cleanup_ = [](void* data, A& alloc) {
      static_cast<Func*>(data)->~Func();
      alloc.deallocate({static_cast<uchar*>(data), sizeof(Func)});
    };
  } else {
    cleanup_ = [](void* data, A& /*allocator*/) {
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
template Function<void(), 0>::Function(decltype([] {})& func);
template Function<void()>::Function(decltype([] {})&& func);
#endif

} // namespace yacsl::containers
