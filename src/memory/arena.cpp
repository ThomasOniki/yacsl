module;

#include <stdio.h>
#include <stdlib.h>

module base.memory.arena;

import base.logging.panic;

namespace base {

namespace {
struct Metadata {
  alignas(16) uchar* last_begin;
  uchar* last_end;
};
} // namespace

void Arena::reserve(usize capacity) noexcept {
  if (begin_ == nullptr && capacity < initial_capacity_) {
    capacity = initial_capacity_;
  }

  const Metadata metadata = {
      .last_begin = begin_,
      .last_end   = end_,
  };
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, hicpp-no-malloc, cppcoreguidelines-owning-memory)
  begin_ = static_cast<uchar*>(malloc(capacity));
  end_   = begin_ + capacity;
  ptr_   = begin_ + sizeof(Metadata);

  panic_if(begin_ == nullptr, "[Arena] malloc failed.");
  construct_at(reinterpret_cast<Metadata*>(begin_), metadata);
}

Arena::Scope::~Scope() noexcept {
  while (ptr_ < arena_.begin_ || ptr_ > arena_.end_) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const Metadata metadata = *reinterpret_cast<Metadata*>(arena_.begin_);
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, hicpp-no-malloc, cppcoreguidelines-owning-memory)
    ::free(arena_.begin_);
    arena_.begin_ = metadata.last_begin;
    arena_.end_   = metadata.last_end;
    arena_.ptr_   = arena_.begin_ + sizeof(Metadata);
  }
  arena_.ptr_ = ptr_;
}

} // namespace base
