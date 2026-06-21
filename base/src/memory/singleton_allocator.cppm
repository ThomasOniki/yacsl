export module base.memory.singleton_allocator;

import base.containers.nttp_string;

export namespace base {

template <typename T, NTTPString Name, bool ThreadLocal = true> class SingletonAllocator {
public:
  using SizeT = T::SizeT;

  static T& get() {
    static thread_local T instance;
    return instance;
  }
};

template <typename T, NTTPString Name> class SingletonAllocator<T, Name, false> {
public:
  using SizeT = T::SizeT;

  static T& get() {
    static T instance;
    return instance;
  }
};

} // namespace base
