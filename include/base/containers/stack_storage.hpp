#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T> class StackStorage {
public:
  template <typename... Args> constexpr T& construct(Args&&... args) {
    return *base::construct_at(&get(), base::forward<Args>(args)...);
  }
  constexpr void destruct() { get().~T(); }

  [[nodiscard]] constexpr T*       operator->() noexcept BASE_LIFETIMEBOUND { return &get(); }
  [[nodiscard]] constexpr T const* operator->() const noexcept BASE_LIFETIMEBOUND { return &get(); }
  [[nodiscard]] constexpr T&       operator*() noexcept BASE_LIFETIMEBOUND { return get(); }
  [[nodiscard]] constexpr T const& operator*() const noexcept BASE_LIFETIMEBOUND { return get(); }

  [[nodiscard]] T& get() noexcept BASE_LIFETIMEBOUND { return *reinterpret_cast<T*>(data_.data()); }
  [[nodiscard]] T const& get() const noexcept BASE_LIFETIMEBOUND {
    return *reinterpret_cast<T const*>(data_.data());
  }

private:
  alignas(T) StackArray<uchar, sizeof(T)> data_{};
};

} // namespace base
