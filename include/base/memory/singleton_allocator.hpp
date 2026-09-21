#pragma once

#include "base/containers/nttp_string.hpp"

namespace base::memory {

template <typename Allocator, containers::NTTPString Name,
          bool ThreadLocal = true>
class SingletonAllocator {
public:
  using SizeT = Allocator::SizeT;

  static constexpr Allocator& get()
    requires ThreadLocal
  {
    static thread_local Allocator instance;
    return instance;
  }
  static constexpr Allocator& get()
    requires(!ThreadLocal)
  {
    static Allocator instance;
    return instance;
  }

  template <typename T>
  [[nodiscard]] ranges::Slice<T, SizeT>
  allocate(this SingletonAllocator /*self*/, SizeT count) noexcept {
    return get().template allocate<T>(count);
  }
  template <typename T>
  void free(this SingletonAllocator /*self*/,
            ranges::Slice<T, SizeT> mem) noexcept {
    get().template free<T>(mem);
  }

  [[nodiscard]] constexpr bool operator==(this SingletonAllocator /*self*/,
                                          SingletonAllocator /*other*/) {
    return true;
  }
};

} // namespace base::memory
