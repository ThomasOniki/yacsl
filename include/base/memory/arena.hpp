#pragma once

#include "base/utils/KiB_MiB_GiB.hpp"
#include "base/utils/move_forward.hpp"
#include "base/utils/placement_new.hpp"
#include "base/utils/trivially_x.hpp"
#include <stdint.h>

namespace base {

class Arena {
public:
	// capacity can be bigger than RAM, even disk size, as long as you don't touch the memory.
	explicit consteval Arena(size_t capacity) : initial_capacity_((capacity + 4095) / 4096 * 4096) {}

	Arena(Arena&)	 = delete;
	Arena(Arena&&) = delete;

	template <TriviallyDestructible T, typename... Args> [[nodiscard]] T& push(Args&&... args);
	template <TriviallyDestructible T> [[nodiscard]] T*										push_array(uintptr_t count);

	template <typename T> [[nodiscard]] T* push_storage();
	template <typename T> [[nodiscard]] T* push_array_storage(uintptr_t count);

public:
	class Scope {
	public:
		Scope(Arena& arena) : arena_(arena), ptr_(arena.ptr_) {}
		~Scope() noexcept;

	private:
		Arena&		arena_;
		uintptr_t ptr_;
	};

private:
	void reserve(size_t capacity);

private:
	uintptr_t begin_ = 0, end_ = 0, ptr_ = 0;
	size_t		initial_capacity_ = 0;
};

template <TriviallyDestructible T, typename... Args> T& Arena::push(Args&&... args) {
	T* value = push_storage<T>();
	return *new (value) T(forward<Args>(args)...);
}

template <TriviallyDestructible T> T* Arena::push_array(uintptr_t count) {
	T* ptr = push_array_storage<T>(count);

	for (uintptr_t i = 0; i < count; ++i)
		new (ptr + i) T();
	return ptr;
}

template <typename T> T* Arena::push_storage() {
	ptr_ = (ptr_ + alignof(T) - 1) & ~(alignof(T) - 1);

	T* ret = reinterpret_cast<T*>(ptr_);
	ptr_ += sizeof(T);

	if (ptr_ > end_) [[unlikely]] {
		reserve(end_ - begin_ + sizeof(T) * 2);
		return push_storage<T>();
	}

	return ret;
}

template <typename T> T* Arena::push_array_storage(uintptr_t count) {
	ptr_ = (ptr_ + alignof(T) - 1) & ~(alignof(T) - 1);

	T* ret = reinterpret_cast<T*>(ptr_);
	ptr_ += sizeof(T) * count;

	if (ptr_ > end_) [[unlikely]] {
		reserve(end_ - begin_ + sizeof(T) * count * 2);
		return push_array_storage<T>(count);
	}

	return ret;
}

inline constinit thread_local Arena main_arena(4 * KiB);
inline constinit thread_local Arena scratch_arena(4 * KiB);

class MainArenaAllocator {
public:
	using SizeT = uintptr_t;

	template <typename T> static T* alloc(SizeT count) { return main_arena.push_array_storage<T>(count); }
	template <typename T> static T* alloc() { return main_arena.push_storage<T>(); }

	template <typename T> static void free(T*, SizeT) noexcept {}
	template <typename T> static void free(T*) noexcept {}
};

class ScratchArenaAllocator {
public:
	using SizeT = uintptr_t;

	template <typename T> static T* alloc(SizeT count) { return scratch_arena.push_array_storage<T>(count); }
	template <typename T> static T* alloc() { return scratch_arena.push_storage<T>(); }

	template <typename T> static void free(T*, SizeT) noexcept {}
	template <typename T> static void free(T*) noexcept {}
};

#ifdef BASE_LSP_ONLY
template int& Arena::push<int>(); // Instantiates to avoid 'placement_new.hpp is unsed'.
#endif

} // namespace base
