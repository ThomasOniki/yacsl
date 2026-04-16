#pragma once

#include "base/memory/singleton_allocator.hpp"

namespace base {

class Arena;
using MainArenaAllocator		= SingletonAllocator<Arena, "main", true>;
using ScratchArenaAllocator = SingletonAllocator<Arena, "scratch", true>;

} // namespace base
