export module base.containers.function;

import base.containers.base_classes;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.type_traits.allocator;
import base.type_traits.qualifiers;
import base.memory.arena;

export namespace base {

template <typename F, usize BufSz = 8, typename Allocator = MainArenaAllocator> class Function;

template <typename Ret, typename... Args, usize BufSz, typename Allocator>
class Function<Ret(Args...), BufSz, Allocator> : AllocatorMember<Allocator>,
                                                 FixedBufferMember<char, 16, usize> {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  Function(auto&& func);

  Function(const Function&)     = delete;
  Function(Function&&) noexcept = delete;

  ~Function() noexcept { cleanup_(data_, allocator()); }

  Function& operator=(const Function&) = delete;
  Function& operator=(Function&&) noexcept;

  Ret operator()(Args... args) { return call_(data_, base::forward<Args>(args)...); }

  [[nodiscard]] const auto& get_allocator() const noexcept { return allocator(); }

private:
  using AllocatorMember<Allocator>::allocator;
  using FixedBufferMember<char, 16, usize>::fixed_buffer_;

  void* data_ = fixed_buffer_.data();

  void (*cleanup_)(void* func, UnderlyingAllocator<Allocator>& alloc) = nullptr;
  Ret (*call_)(void* func, Args... args)                              = nullptr;
};

template <typename Ret, typename... Args, usize BufSz, typename Allocator>
Function<Ret(Args...), BufSz, Allocator>::Function(auto&& func) {
  using Func = RemoveReference<decltype(func)>;

  if constexpr (sizeof(Func) > sizeof(uchar) * BufSz || alignof(Func) > 8) {
    data_ = allocator().template allocate<Func>();

    cleanup_ = [](void* data, UnderlyingAllocator<Allocator>& alloc) {
      static_cast<Func*>(data)->~Func();
      alloc.template free<Func>(static_cast<Func*>(data));
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
