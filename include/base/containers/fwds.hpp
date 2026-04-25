#pragma once

#include "base/memory/fwds.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, usize N = 0, typename Allocator = MainArenaAllocator, typename SizeT = usize>
class Vector;

template <typename T, usize N, typename SizeT = usize> class StackArray;
// THIS DOES NOT KEEP TRACK OF WHICH ELEMENTS WHERE CREATED AND DESTROYED, YOU ARE RESPONSIBLE FOR
// THAT.
template <typename T, usize N, typename SizeT = usize> class StackArrayStorage;

template <typename T, typename SizeT = usize> class BasicStringView;
using StringView = BasicStringView<char>;

template <typename Char, usize N = 16, typename Allocator = MainArenaAllocator,
					typename SizeT = usize>
class BasicCString;
using CString = BasicCString<char>;

template <usize N = 0, typename Allocator = MainArenaAllocator, UnsignedIntegral T = u32,
					typename SizeT = usize>
class Bitfield;
template <usize N = 0, UnsignedIntegral T = u32> class StackBitfield;

} // namespace base
