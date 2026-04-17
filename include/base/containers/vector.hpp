#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/initializer_list.hpp"
#include "base/containers/reverse_iterator.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/stack_array_storage.hpp"
#include "base/memory/arena.hpp" // NOLINT(misc-include-cleaner)
#include "base/metaprogramming/type_manip.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

namespace internal {
template <typename T, usize N, typename Allocator, typename SizeT> struct VectorMembers;
} // namespace internal

template <typename T, usize N, typename Allocator, typename SizeT>
class Vector : private internal::VectorMembers<T, N, Allocator, SizeT> {
public:
	constexpr Vector() noexcept = default;
	constexpr Vector(const InitializerList<T>& init);
	constexpr explicit Vector(SizeT count);

	constexpr ~Vector() noexcept;

	template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
	// NOLINTNEXTLINE(google-explicit-constructor)
	constexpr Vector(const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other);
	constexpr Vector(const Vector& other);
	template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
	// NOLINTNEXTLINE(google-explicit-constructor)
	constexpr Vector(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept;
	constexpr Vector(Vector&& other) noexcept;

	template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
	constexpr Vector& operator=(const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other);
	constexpr Vector& operator=(const Vector& other);
	template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
	constexpr Vector& operator=(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept;
	constexpr Vector& operator=(Vector&& other) noexcept;

	constexpr Vector& operator=(const InitializerList<T>& init);

	constexpr void push_back(const T& value) { emplace_back(value); }
	constexpr void push_back(T&& value) { emplace_back(base::move(value)); }

	template <typename... Args> constexpr void emplace_back(Args&&... args);

	constexpr void pop_back() noexcept;
	constexpr void clear() noexcept;

	constexpr void reserve(SizeT capacity);
	constexpr void resize(SizeT count);

	constexpr void shrink_to_fit() { definitely_reserve(size_); }

	[[nodiscard]] constexpr SizeT size() const noexcept { return size_; }
	[[nodiscard]] constexpr SizeT capacity() const noexcept { return capacity_; }

	[[nodiscard]] constexpr T*			 data() noexcept { return data_; }
	[[nodiscard]] constexpr const T* data() const noexcept { return data_; }

	[[nodiscard]] constexpr T*			 begin() noexcept { return data_; }
	[[nodiscard]] constexpr const T* begin() const noexcept { return data_; }
	[[nodiscard]] constexpr T*			 end() noexcept { return data_ + size_; }
	[[nodiscard]] constexpr const T* end() const noexcept { return data_ + size_; }

	[[nodiscard]] constexpr ReverseIterator<T*>				rbegin() noexcept { return data_ + size_; }
	[[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept {
		return data_ + size_;
	}
	[[nodiscard]] constexpr ReverseIterator<T*>				rend() noexcept { return data_; }
	[[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data_; }

	[[nodiscard]] constexpr const T* cbegin() const noexcept { return data_; }
	[[nodiscard]] constexpr const T* cend() const noexcept { return data_ + size_; }

	[[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept {
		return data_ + size_;
	}
	[[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data_; }

	[[nodiscard]] constexpr T& operator[](SizeT index) noexcept {
		return data_[static_cast<usize>(index)];
	}
	[[nodiscard]] constexpr T& operator[](SizeT index) const noexcept {
		return data_[static_cast<usize>(index)];
	}

	[[nodiscard]] constexpr T&			 front() noexcept { return data_[0]; }
	[[nodiscard]] constexpr const T& front() const noexcept { return data_[0]; }
	[[nodiscard]] constexpr T&			 back() noexcept { return data_[size_ - 1]; }
	[[nodiscard]] constexpr const T& back() const noexcept { return data_[size_ - 1]; }

private:
	void maybe_alloc_size();					// if size_ > N
	void maybe_free_and_alloc_size(); // if size_ > current capacity

	void alloc(SizeT capacity);
	void free();

	void copy(const T* data);
	void move(T* data);

	void definitely_reserve(SizeT capacity);

	template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
	void steal_ptr(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other);

	auto& allocator();

private:
	using Members = internal::VectorMembers<T, N, Allocator, SizeT>;
	using Members::capacity_;
	using Members::data_;
	using Members::size_;
	using Members::small_buffer_;

private:
	template <typename OtherT, usize OtherN, typename OtherAllocator, typename OtherSizeT>
	friend class Vector;
};

namespace internal {

template <typename T, usize N, typename Allocator, typename SizeT> struct VectorMembers {
protected:
	constexpr VectorMembers() = default;
	explicit constexpr VectorMembers(SizeT size) : size_(size) {}

protected:
	static constexpr bool trivial = TriviallyCopyable<T> && TriviallyDestructible<T>;

	using SmallBuffer = ConditionalT<trivial, StackArray<T, N>, StackArrayStorage<T, N>>;

	SmallBuffer small_buffer_;
	T*					data_ = small_buffer_.data();
	SizeT				size_ = 0, capacity_ = N;
	Allocator		allocator_;
};

template <typename T, typename Allocator, typename SizeT>
struct VectorMembers<T, 0, Allocator, SizeT> {
protected:
	constexpr VectorMembers() = default;
	explicit constexpr VectorMembers(SizeT size) : size_(size) {}

protected:
	T*				data_ = nullptr;
	SizeT			size_ = 0, capacity_ = 0;
	Allocator allocator_;

	struct DummySmallBuffer {
		[[nodiscard]] const T* data() const noexcept { return nullptr; }
	};
	static constexpr DummySmallBuffer small_buffer_;
};

template <typename T, usize N, SingletonAllocatorLike Allocator, typename SizeT>
struct VectorMembers<T, N, Allocator, SizeT> {
protected:
	constexpr VectorMembers() = default;
	explicit constexpr VectorMembers(SizeT size) : size_(size) {}

protected:
	static constexpr bool trivial = TriviallyCopyable<T> && TriviallyDestructible<T>;

	using SmallBuffer = ConditionalT<trivial, StackArray<T, N>, StackArrayStorage<T, N>>;

	SmallBuffer small_buffer_;
	T*					data_ = small_buffer_.data();
	SizeT				size_ = 0, capacity_ = N;
};

template <typename T, SingletonAllocatorLike Allocator, typename SizeT>
struct VectorMembers<T, 0, Allocator, SizeT> {
protected:
	constexpr VectorMembers() = default;
	explicit constexpr VectorMembers(SizeT size) : size_(size) {}

protected:
	alignas(32) T* data_ = nullptr;
	SizeT size_ = 0, capacity_ = 0;

	struct DummySmallBuffer {
		[[nodiscard]] const T* data() const noexcept { return nullptr; }
	};
	static constexpr DummySmallBuffer small_buffer_;
};

} // namespace internal

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(const InitializerList<T>& init)
		: Members(static_cast<SizeT>(init.size())) {
	maybe_alloc_size();
	copy(init.begin());
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(SizeT count) : Members(count) {
	maybe_alloc_size();
	for (SizeT i = 0; i < count; ++i) {
		new (data_ + i) T();
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>::~Vector() noexcept {
	clear();
	free();
}

template <typename T, usize N, typename Allocator, typename SizeT>
template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(
		const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other)
		: Members(static_cast<SizeT>(other.size_)) {
	maybe_alloc_size();
	copy(other.data_);
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(const Vector& other)
		: Members(static_cast<SizeT>(other.size_)) {
	maybe_alloc_size();
	copy(other.data_);
}

template <typename T, usize N, typename Allocator, typename SizeT>
template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(
		Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept
		: Members(static_cast<SizeT>(other.size_)) {
	if constexpr (!SingletonAllocatorLike<Allocator> || !SameAs<Allocator, OtherAllocator>) {
		maybe_alloc_size();
		move(other.data_);
		return;
	}

	if constexpr (OtherN > 0) {
		if (other.data_ == other.small_buffer_.data()) {
			if constexpr (OtherN > N) {
				maybe_alloc_size();
			}
			move(other.data_);
			return;
		}
	}
	steal_ptr(base::move(other));
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Vector&& other) noexcept
		: Members(static_cast<SizeT>(other.size_)) {
	if constexpr (N > 0) {
		if (other.data_ == other.small_buffer_.data()) {
			move(other.data_);
			return;
		}
	}
	steal_ptr(base::move(other));
}

template <typename T, usize N, typename Allocator, typename SizeT>
template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>& Vector<T, N, Allocator, SizeT>::operator=(
		const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other) {
	clear();
	size_ = static_cast<SizeT>(other.size_);
	reserve(size_);
	copy(other.data_);
	return *this;
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(const Vector& other) {
	if (&other == this) [[unlikely]] {
		return *this;
	}
	clear();
	size_ = static_cast<SizeT>(other.size_);
	reserve(size_);
	copy(other.data_);
	return *this;
}

template <typename T, usize N, typename Allocator, typename SizeT>
template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>& Vector<T, N, Allocator, SizeT>::operator=(
		Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept {
	clear();
	size_ = static_cast<SizeT>(other.size_);

	if constexpr (!SingletonAllocatorLike<Allocator> || !SameAs<Allocator, OtherAllocator>) {
		reserve(size_);
		move(other.data_);
		return *this;
	}

	if constexpr (OtherN > 0) {
		if (other.data_ == other.small_buffer_.data()) {
			if constexpr (OtherN > N) {
				reserve(size_);
			}
			move(other.data_);
			return *this;
		}
	}
	free();
	steal_ptr(base::move(other));
	return *this;
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(Vector&& other) noexcept {
	clear();
	size_ = static_cast<SizeT>(other.size_);

	if constexpr (N > 0) {
		if (other.data_ == other.small_buffer_.data()) {
			move(other.data_);
			return *this;
		}
	}
	free();
	steal_ptr(base::move(other));
	return *this;
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(const InitializerList<T>& init) {
	clear();
	size_ = static_cast<SizeT>(init.size());
	reserve(size_);
	copy(init.begin());
	return *this;
}

template <typename T, usize N, typename Allocator, typename SizeT>
template <typename... Args>
constexpr void Vector<T, N, Allocator, SizeT>::emplace_back(Args&&... args) {
	if (size_ == capacity_) [[unlikely]] {
		definitely_reserve(capacity_ * 2 + 1);
	}
	new (data_ + size_) T(base::forward<Args>(args)...);
	++size_;
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::pop_back() noexcept {
	--size_;
	data_[size_].~T();
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::clear() noexcept {
	if constexpr (!TriviallyDestructible<T>) {
		while (size_ > 0) {
			pop_back();
		}
	} else {
		size_ = 0;
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::reserve(SizeT capacity) {
	if (capacity > capacity_) {
		definitely_reserve(capacity);
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::resize(SizeT count) {
	if (size_ >= count) {
		if constexpr (TriviallyDestructible<T>) {
			size_ = count;
		} else {
			while (size_ > count) {
				pop_back();
			}
		}
	} else {
		definitely_reserve(count);
		for (; size_ < count; ++size_) {
			new (data_ + size_ - 1) T();
		}
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
void Vector<T, N, Allocator, SizeT>::maybe_alloc_size() {
	if constexpr (N > 0) {
		if (size_ > static_cast<SizeT>(N)) {
			alloc(size_);
		}
	} else {
		alloc(size_);
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
void Vector<T, N, Allocator, SizeT>::alloc(SizeT capacity) {
	capacity_ = capacity;
	data_			= allocator().template allocate<T>(static_cast<Allocator::SizeT>(capacity_));
}

template <typename T, usize N, typename Allocator, typename SizeT>
void Vector<T, N, Allocator, SizeT>::free() {
	if constexpr (N > 0) {
		if (data_ != small_buffer_.data()) {
			allocator().template free<T>(data_, static_cast<Allocator::SizeT>(capacity_));
		}
	} else {
		allocator().template free<T>(data_, static_cast<Allocator::SizeT>(capacity_));
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
void Vector<T, N, Allocator, SizeT>::definitely_reserve(SizeT capacity) {
	T* old_data = data_;
	data_				= allocator().template allocate<T>(static_cast<Allocator::SizeT>(capacity));
	move(old_data);

	for (SizeT i = 0; i < size_; ++i) {
		old_data[i].~T();
	}

	if constexpr (N > 0) {
		if (old_data != small_buffer_.data()) {
			allocator().template free<T>(old_data, static_cast<Allocator::SizeT>(capacity_));
		}
	} else {
		allocator().template free<T>(old_data, static_cast<Allocator::SizeT>(capacity_));
	}
	capacity_ = capacity;
}

template <typename T, usize N, typename Allocator, typename SizeT>
void Vector<T, N, Allocator, SizeT>::copy(const T* data) {
	if constexpr (TriviallyCopyable<T>) {
		memcpy(data_, data, static_cast<usize>(size_) * sizeof(T));
	} else {
		for (SizeT i = 0; i < size_; ++i) {
			new (data_ + i) T(data[i]);
		}
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
void Vector<T, N, Allocator, SizeT>::move(T* data) {
	if constexpr (TriviallyCopyable<T>) {
		memcpy(data_, data, static_cast<usize>(size_) * sizeof(T));
	} else {
		for (SizeT i = 0; i < size_; ++i) {
			new (data_ + i) T(base::move(data[i]));
		}
	}
}

template <typename T, usize N, typename Allocator, typename SizeT>
template <usize OtherN, typename OtherAllocator, typename OtherSizeT>
void Vector<T, N, Allocator, SizeT>::steal_ptr(
		Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) {
	data_				= other.data_;
	capacity_		= static_cast<SizeT>(other.capacity_);
	other.size_ = 0;
	other.data_ = nullptr;

	(void)base::move(other);
}

template <typename T, usize N, typename Allocator, typename SizeT>
auto& Vector<T, N, Allocator, SizeT>::allocator() {
	if constexpr (SingletonAllocatorLike<Allocator>) {
		return Allocator::get();
	} else {
		return Members::allocator_;
	}
}

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::Vector<int>; // Instantiates to avoid 'arena.hpp is unused'.
#endif
