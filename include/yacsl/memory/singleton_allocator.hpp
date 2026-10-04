#pragma once

#include "yacsl/containers/nttp_string.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/allocator_type.hpp"

namespace yacsl::memory {

template <AllocatorType A, containers::NTTPString Name, bool ThreadLocal = true>
class SingletonAllocator {
public:
  using SizeT = A::SizeT;

  static constexpr A& get()
    requires ThreadLocal
  {
    static thread_local A instance;
    return instance;
  }
  static constexpr A& get()
    requires(!ThreadLocal)
  {
    static A instance;
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
