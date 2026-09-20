#pragma once

#include "base/containers/tuple.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/meta/integer_sequence.hpp"
#include "base/threads/atomic.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/qualifiers.hpp"
#include <pthread.h>

namespace base::internal {
template <typename T> struct StorableFunctionType {
  using Type = T;
};
template <typename Ret, typename... Args>
struct StorableFunctionType<Ret(Args...)> {
  using Type = Ret (*)(Args...);
};
} // namespace base::internal

namespace base {

class Thread {
public:
  Thread() noexcept = default;

  template <typename Func, typename... Args>
  explicit Thread(Func&& func, Args&&... args) {
    start(base::forward<Func>(func), base::forward<Args>(args)...);
  }

  Thread(void* func(void*), void* arg) {
    pthread_create(&handle_, nullptr, func, arg);
  }
  Thread(void* func(void*) noexcept, void* arg) {
    pthread_create(&handle_, nullptr, func, arg);
  }

  void start(this Thread& self, void* func(void*), void* arg) noexcept {
    self.join();
    pthread_create(&self.handle_, nullptr, func, arg);
  }
  void start(this Thread& self, void* func(void*) noexcept,
             void* arg) noexcept {
    self.join();
    pthread_create(&self.handle_, nullptr, func, arg);
  }

  template <typename Func, typename... Args>
  void start(this Thread& self, Func&& func, Args&&... args) {
    self.join();
    static_assert(SameAs<void, decltype(func(base::forward<Args>(args)...))>);

    using StoredFunction =
        internal::StorableFunctionType<RemoveReference<Func>>::Type;
    using Pack         = Tuple<StoredFunction, Atomic<bool>&, Args...>;
    Atomic<bool> ready = false;
    Pack pack = Pack(forward<Func>(func), ready, base::forward<Args>(args)...);
    pthread_create(
        &self.handle_, nullptr,
        Callback<Func, IndexSequenceToN<sizeof...(Args)>, Args...>::callback,
        &pack);
    while (!ready.load(MemoryOrder::Acquire)) {
    }
  }

  Thread(Thread&& other) noexcept : handle_(other.handle_) {
    other.handle_ = NIL_PTHREAD;
  }
  Thread(Thread const&) = delete;

  Thread& operator=(this Thread& self BASE_LIFETIMEBOUND,
                    Thread&&          other) noexcept {
    self.handle_  = other.handle_;
    other.handle_ = NIL_PTHREAD;
    return self;
  }
  Thread& operator=(Thread const&) = delete;

  ~Thread() noexcept { join(); }

  [[nodiscard]] bool joinable(this Thread const& self) noexcept {
    return self.handle_ != NIL_PTHREAD;
  }

  void join(this Thread& self) {
    if (self.joinable()) {
      void* result = nullptr;
      pthread_join(self.handle_, &result);
      self.handle_ = NIL_PTHREAD;
    }
  }

private:
  template <typename Func, typename Indices, typename... Args> struct Callback;

  template <typename Func, usize... Indices, typename... Args>
  struct Callback<Func, IndexSequence<Indices...>, Args...> {
    static void* callback(void* data) {
      using StoredFunction =
          internal::StorableFunctionType<RemoveReference<Func>>::Type;
      using Pack = Tuple<StoredFunction, Atomic<bool>&, Args...>;
      Pack pack(base::move(*static_cast<Pack*>(data)));

      pack.template get<1>().store(true, MemoryOrder::Release);

      pack.template get<0>()(
          base::forward<Args>(pack.template get<Indices + 2>())...);
      return nullptr;
    }
  };

private:
  static constexpr pthread_t NIL_PTHREAD{};

  pthread_t handle_ = NIL_PTHREAD;
};

} // namespace base
