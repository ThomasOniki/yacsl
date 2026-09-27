#pragma once

#include "yacsl/containers/nttp_string.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/type_traits/allocator.hpp"

namespace yacsl::memory {

template <type_traits::AllocatorType Allocator, containers::NTTPString Name,
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

  [[nodiscard]] auto allocate(this SingletonAllocator /*self*/, SizeT size,
                              Alignment align) noexcept {
    return get().allocate(size, align);
  }
  void deallocate(this SingletonAllocator /*self*/,
                  ranges::Slice<uchar, SizeT> mem) noexcept {
    get().deallocate(mem);
  }
  auto resize_in_place(this SingletonAllocator /*self*/,
                       ranges::Slice<uchar, SizeT> mem, SizeT new_size) {
    return get().resize_in_place(mem, new_size);
  }

  [[nodiscard]] constexpr bool operator==(this SingletonAllocator /*self*/,
                                          SingletonAllocator /*other*/) {
    return true;
  }
};

} // namespace yacsl::memory
