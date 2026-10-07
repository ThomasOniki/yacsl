#pragma once

#include "yacsl/containers/tuple.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/integer_sequence.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/type_aliases.hpp"
#include <pthread.h>

namespace yacsl::threads::detail {
template <typename T> struct StorableFunctionType {
  using Type = T;
};
template <typename Ret, typename... Args>
struct StorableFunctionType<Ret(Args...)> {
  using Type = Ret (*)(Args...);
};
} // namespace yacsl::threads::detail

namespace yacsl::threads {

class Thread {
public:
  Thread() noexcept = default;

  template <typename Func, typename... Args>
  explicit Thread(Func&& func, Args&&... args) {
    start(memory::forward<Func>(func), memory::forward<Args>(args)...);
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
    static_assert(
        meta::SameAs<void, decltype(func(memory::forward<Args>(args)...))>);

    using StoredFunction =
        detail::StorableFunctionType<meta::RemoveReference<Func>>::Type;
    using Pack = containers::Tuple<StoredFunction, Atomic<bool>&, Args...>;
    Atomic<bool> ready = false;
    Pack         pack  = Pack(memory::forward<Func>(func), ready,
                              memory::forward<Args>(args)...);
    pthread_create(&self.handle_, nullptr,
                   Callback<Func, meta::IndexSequenceToN<sizeof...(Args)>,
                            Args...>::callback,
                   &pack);
    while (!ready.load(MemoryOrder::Acquire)) {
    }
  }

  Thread(Thread&& other) noexcept : handle_(other.handle_) {
    other.handle_ = NIL_PTHREAD;
  }
  Thread(Thread const&) = delete;

  Thread& operator=(this Thread& self YACSL_LIFETIMEBOUND,
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
  struct Callback<Func, meta::IndexSequence<Indices...>, Args...> {
    static void* callback(void* data) {
      using StoredFunction =
          detail::StorableFunctionType<meta::RemoveReference<Func>>::Type;
      using Pack = containers::Tuple<StoredFunction, Atomic<bool>&, Args...>;
      Pack pack  = memory::move(*static_cast<Pack*>(data));

      pack.template get<1>().store(true, MemoryOrder::Release);

      pack.template get<0>()(
          memory::forward<Args>(pack.template get<Indices + 2>())...);
      return nullptr;
    }
  };

private:
  static constexpr pthread_t NIL_PTHREAD{};

  pthread_t handle_ = NIL_PTHREAD;
};

} // namespace yacsl::threads
