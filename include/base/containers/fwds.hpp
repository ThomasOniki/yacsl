#pragma once

#include "base/memory/fwds.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/limits.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, usize N = 0, typename Allocator = MainArenaAllocator, typename SizeT = usize>
class Vector;

template <typename T, usize N, typename SizeT = ConditionalT<(N > Limits<u32>::max), usize, u32>>
class StackArray;
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
