module;

#include "base/attributes.hpp"

export module base.containers.stack_storage;

import base.containers.stack_array;
import base.utilities.type_aliases;
import base.memory.lifetimes;

export namespace base {

template <typename T> class StackStorage {
public:
  template <typename... Args> constexpr T& construct(Args&&... args) {
    return *base::construct_at(&get(), base::forward<Args>(args)...);
  }
  constexpr void destruct() { get().~T(); }

  [[nodiscard]] constexpr T*       operator->() noexcept BASE_LIFETIMEBOUND { return &get(); }
  [[nodiscard]] constexpr const T* operator->() const noexcept BASE_LIFETIMEBOUND { return &get(); }
  [[nodiscard]] constexpr T&       operator*() noexcept BASE_LIFETIMEBOUND { return get(); }
  [[nodiscard]] constexpr const T& operator*() const noexcept BASE_LIFETIMEBOUND { return get(); }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  [[nodiscard]] T& get() noexcept BASE_LIFETIMEBOUND { return *reinterpret_cast<T*>(data_.data()); }
  [[nodiscard]] const T& get() const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return *reinterpret_cast<const T*>(data_.data());
  }

private:
  StackArray<uchar, sizeof(T)> data_;
};

} // namespace base
