#include "base/memory/arena.hpp"
#include "base/debug/assert.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_aliases.hpp"
#include <stdlib.h>

#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
#endif

namespace base::memory {

namespace {
struct Metadata {
  alignas(16) uchar* last_begin;
  uchar* last_end;
};
} // namespace

// NOLINTNEXTLINE(misc-no-recursion)
void Arena::reserve(this Arena& self, usize capacity) noexcept {
  if (self.begin_ == nullptr && capacity < self.initial_capacity_) {
    capacity = self.initial_capacity_;
  }

  Metadata const metadata = {
      .last_begin = self.begin_,
      .last_end   = self.end_,
  };
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
  self.begin_ = static_cast<uchar*>(malloc(capacity));
  self.end_   = self.begin_ + capacity;
  self.ptr_   = self.begin_ + sizeof(Metadata);

#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
  __lsan_ignore_object(self.begin_);
#endif

  debug::assert_true(self.begin_ != nullptr, "[Arena] malloc failed.");
  construct_at(reinterpret_cast<Metadata*>(self.begin_), metadata);
}

void Arena::free_all(this Arena& self) noexcept {
  while (self.begin_ != nullptr) {
    Metadata const metadata = *reinterpret_cast<Metadata*>(self.begin_);
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    ::free(self.begin_);
    self.begin_ = metadata.last_begin;
  }
  self.begin_ = self.end_ = self.ptr_ = nullptr;
}

Arena::Scope::~Scope() noexcept {
  while (ptr_ < arena_.begin_ || ptr_ > arena_.end_) {
    Metadata const metadata = *reinterpret_cast<Metadata*>(arena_.begin_);
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    ::free(arena_.begin_);
    arena_.begin_ = metadata.last_begin;
    arena_.end_   = metadata.last_end;
    arena_.ptr_   = arena_.begin_ + sizeof(Metadata);
  }
  arena_.ptr_ = ptr_;
}

} // namespace base::memory
