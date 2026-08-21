#pragma once

namespace base {

template <typename T> class RefWrapper {
public:
  RefWrapper(T& value) noexcept : ptr_(value) {}

  T&       get() noexcept { return *ptr_; }
  T const& get() const noexcept { return *ptr_; }
  T*       operator->() noexcept { return ptr_; }
  T const* operator->() const noexcept { return ptr_; }
  T&       operator*() noexcept { return *ptr_; }
  T const& operator*() const noexcept { return *ptr_; }

private:
  T* ptr_;
};

} // namespace base
