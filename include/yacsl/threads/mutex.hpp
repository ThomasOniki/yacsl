#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/compiler/pragmas.hpp"
#include <pthread.h>

namespace yacsl::threads {

class Mutex {
public:
  constexpr Mutex() noexcept = default;
  Mutex(Mutex const&)        = delete;
  Mutex(Mutex&&) noexcept    = delete;

  ~Mutex() noexcept = default;

  Mutex& operator=(Mutex const&)     = delete;
  Mutex& operator=(Mutex&&) noexcept = delete;

  void lock(this Mutex& self) noexcept { pthread_mutex_lock(&self.handle_); }
  bool try_lock(this Mutex& self) noexcept {
    return static_cast<bool>(pthread_mutex_trylock(&self.handle_));
  }
  void unlock(this Mutex& self) noexcept {
    pthread_mutex_unlock(&self.handle_);
  }

private:
  YACSL_GCC_PRAGMA(GCC diagnostic push)
  YACSL_GCC_PRAGMA(diagnostic ignored "-Wzero-as-null-pointer-constant")
  pthread_mutex_t handle_ = PTHREAD_MUTEX_INITIALIZER;
  YACSL_GCC_PRAGMA(diagnostic pop)
};

class MutexLock {
public:
  explicit MutexLock(YACSL_LIFETIMEBOUND Mutex& mutex) noexcept
      : mutex_(&mutex) {
    mutex_->lock();
  }
  MutexLock(MutexLock const&)     = delete;
  MutexLock(MutexLock&&) noexcept = delete;

  ~MutexLock() noexcept { mutex_->unlock(); }

  MutexLock& operator=(MutexLock const&)     = delete;
  MutexLock& operator=(MutexLock&&) noexcept = delete;

private:
  Mutex* mutex_;
};

} // namespace yacsl::threads
