#pragma once

#include "base/utils/size_t.hpp"

namespace base {

class MainArenaAllocator;
class ScratchArenaAllocator;

template <typename T, typename Alloc = MainArenaAllocator, typename SizeT = size_t, size_t N = 0> class Vector;

namespace internal {
template <typename T, size_t N, typename Alloc, typename SizeT>
using SmallVector = Vector<T, Alloc, SizeT, N == -1 ? (64 - sizeof(Vector<T, Alloc, SizeT>)) / sizeof(T) : N>;
}

template <typename T, size_t N = static_cast<size_t>(-1), typename Alloc = MainArenaAllocator, typename SizeT = size_t>
using SmallVector = internal::SmallVector<T, N, Alloc, SizeT>;

} // namespace base
