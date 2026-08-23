#pragma once

#include "base/containers/nttp_string.hpp"

namespace base {

template <typename T, NTTPString Name, bool ThreadLocal = true> class SingletonAllocator {
public:
  using SizeT = T::SizeT;

  static constexpr T& get() {
    static thread_local T instance;
    return instance;
  }
};

template <typename T, NTTPString Name> class SingletonAllocator<T, Name, false> {
public:
  using SizeT = T::SizeT;

  static constexpr T& get() {
    static T instance;
    return instance;
  }
};

} // namespace base
