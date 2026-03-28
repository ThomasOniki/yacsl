#include "base/memory/arena.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace base {

struct Metadata {
	uintptr_t last_begin;
	uintptr_t last_end;
};

void Arena::reserve(size_t capacity) {
	if (begin_ == 0 && capacity < initial_capacity_)
		capacity = initial_capacity_;
	uintptr_t last_begin = begin_;
	uintptr_t last_end	 = end_;

	void* data = malloc(capacity);

	if (!data) {
		fprintf(stderr, "[Arena] malloc failed");
		exit(-1);
	}

	begin_ = reinterpret_cast<uintptr_t>(data);
	end_	 = begin_ + capacity;
	ptr_	 = begin_ + sizeof(Metadata);

	new (data) Metadata{last_begin, last_end};
}

Arena::Scope::~Scope() noexcept {
	while (ptr_ < arena_.begin_ || ptr_ > arena_.end_) {
		Metadata metadata = *reinterpret_cast<Metadata*>(arena_.begin_);
		free(reinterpret_cast<void*>(arena_.begin_));
		arena_.begin_ = metadata.last_begin;
		arena_.end_		= metadata.last_end;
		arena_.ptr_		= arena_.begin_ + sizeof(Metadata);
	}
	arena_.ptr_ = ptr_;
}

} // namespace base
