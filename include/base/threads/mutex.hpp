#include <pthread.h>

namespace base {

class Mutex {
public:
  constexpr Mutex() noexcept = default;
  Mutex(const Mutex&)        = delete;
  Mutex(Mutex&&) noexcept    = delete;

  ~Mutex() noexcept = default;

  Mutex& operator=(const Mutex&)     = delete;
  Mutex& operator=(Mutex&&) noexcept = delete;

  void lock() { pthread_mutex_lock(&handle_); }
  bool try_lock() { return static_cast<bool>(pthread_mutex_trylock(&handle_)); }
  void unlock() { pthread_mutex_unlock(&handle_); }

private:
  pthread_mutex_t handle_ = PTHREAD_MUTEX_INITIALIZER;
};

class MutexLock {
public:
  explicit MutexLock(BASE_LIFETIMEBOUND Mutex& mutex) : mutex_(&mutex) { mutex_->lock(); }
  MutexLock(const MutexLock&)     = delete;
  MutexLock(MutexLock&&) noexcept = delete;

  ~MutexLock() noexcept { mutex_->unlock(); }

  MutexLock& operator=(const MutexLock&)     = delete;
  MutexLock& operator=(MutexLock&&) noexcept = delete;

private:
  Mutex* mutex_;
};

} // namespace base
