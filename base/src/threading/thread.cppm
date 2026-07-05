module;

#include <pthread.h>

export module base.threading.thread;

import base.threading.atomic;
import base.containers.tuple;
import base.memory.lifetimes;
import base.utilities.type_aliases;
import base.type_traits.qualifiers;
import base.type_traits.comparaison;
import base.metaprogramming.integer_sequence;

namespace base::internal {
template <typename T> struct StorableFunctionType {
  using Type = T;
};
template <typename Ret, typename... Args> struct StorableFunctionType<Ret(Args...)> {
  using Type = Ret (*)(Args...);
};
} // namespace base::internal

export namespace base {

class Thread {
public:
  Thread() noexcept = default;

  template <typename Func, typename... Args>
  explicit Thread(Func&& func, Args&&... args) { // NOLINT(cppcoreguidelines-missing-std-forward)
    static_assert(SameAs<void, decltype(func(base::forward<Args>(args)...))>);

    using StoredFunction = internal::StorableFunctionType<RemoveReference<Func>>::Type;
    using Pack           = Tuple<StoredFunction, Atomic<bool>&, Args...>;
    Atomic<bool> ready   = false;
    Pack         pack    = Pack(forward<Func>(func), ready, base::forward<Args>(args)...);
    pthread_create(&handle_, nullptr,
                   Callback<Func, IndexSequenceToN<sizeof...(Args)>, Args...>::callback, &pack);
    while (!ready.load(MemoryOrder::SeqCst)) {
    }
  }

  Thread(Thread&& other) noexcept : handle_(other.handle_) { other.handle_ = NIL_PTHREAD; }
  Thread(const Thread&) = delete;

  Thread& operator=(Thread&& other) noexcept {
    handle_       = other.handle_;
    other.handle_ = NIL_PTHREAD;
    return *this;
  }
  Thread& operator=(const Thread&) = delete;

  ~Thread() noexcept {
    if (joinable()) {
      join();
    }
  }

  [[nodiscard]] bool joinable() const noexcept { return handle_ != NIL_PTHREAD; }

  void join() {
    void* result = nullptr;
    pthread_join(handle_, &result);
    handle_ = NIL_PTHREAD;
  }

private:
  template <typename Func, typename Indices, typename... Args> struct Callback;

  template <typename Func, usize... Indices, typename... Args>
  struct Callback<Func, IndexSequence<Indices...>, Args...> {
    static void* callback(void* data) {
      using StoredFunction = internal::StorableFunctionType<RemoveReference<Func>>::Type;
      using Pack           = Tuple<StoredFunction, Atomic<bool>&, Args...>;
      Pack& ref            = *static_cast<Pack*>(data);
      Pack  pack(ref.template get<0>(), ref.template get<1>(),
                 base::forward<Args>(ref.template get<Indices + 2>())...);

      pack.template get<1>().store(true, MemoryOrder::SeqCst);

      pack.template get<0>()(base::forward<Args>(pack.template get<Indices + 2>())...);
      return nullptr;
    }
  };

private:
  static constexpr pthread_t NIL_PTHREAD{};

  pthread_t handle_ = NIL_PTHREAD;
};

} // namespace base
