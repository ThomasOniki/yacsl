export module base.containers.stack_storage;

import base.containers.stack_array;
import base.utilities.type_aliases;
import base.memory.lifetimes;

export namespace base {

template <typename T> class StackStorage {
public:
  template <typename... Args> constexpr T& construct(Args&&... args) {
    return *base::construct_at(get(), base::forward<Args>(args)...);
  }
  constexpr void destruct() { get()->~T(); }

  [[nodiscard]] constexpr T*       operator->() noexcept { return get(); }
  [[nodiscard]] constexpr const T* operator->() const noexcept { return get(); }
  [[nodiscard]] constexpr T&       operator*() noexcept { return *get(); }
  [[nodiscard]] constexpr const T& operator*() const noexcept { return *get(); }

private:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  T* get() noexcept { return reinterpret_cast<T*>(data_.data()); }

private:
  StackArray<uchar, sizeof(T)> data_;
};

} // namespace base
