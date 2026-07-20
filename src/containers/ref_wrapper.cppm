export module base.containers.ref_wrapper;

export namespace base {

template <typename T> class RefWrapper {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  RefWrapper(T& value) noexcept : ptr_(value) {}

  T&       get() noexcept { return *ptr_; }
  const T& get() const noexcept { return *ptr_; }
  T*       operator->() noexcept { return ptr_; }
  const T* operator->() const noexcept { return ptr_; }
  T&       operator*() noexcept { return *ptr_; }
  const T& operator*() const noexcept { return *ptr_; }

private:
  T* ptr_;
};

} // namespace base
