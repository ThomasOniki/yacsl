#pragma once

#include "base/memory/arena.hpp"
#include "base/type_fwds.hpp"
#include "base/utils/initializer_list.hpp"
#include "base/utils/move_forward.hpp"
#include "base/utils/reverse_iterator.hpp"
#include "base/utils/type_operations.hpp"
#include <string.h>

namespace base {

namespace internal {
template <typename T, typename Alloc, typename SizeT, size_t N> struct VectorMembers;
} // namespace internal

template <typename T, typename Alloc, typename SizeT, size_t N> class Vector : internal::VectorMembers<T, Alloc, SizeT, N> {
public:
	constexpr Vector() noexcept = default;
	constexpr Vector(const InitializerList<T>& il);
	constexpr explicit Vector(SizeT count);

	template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
	constexpr Vector(const Vector<T, OtherAlloc, OtherSizeT, OtherN>& other);
	constexpr Vector(const Vector& other);
	template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
	constexpr Vector(Vector<T, OtherAlloc, OtherSizeT, OtherN>&& other) noexcept;
	constexpr Vector(Vector&& other) noexcept;

	constexpr ~Vector() noexcept;

	template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
	constexpr Vector& operator=(const Vector<T, OtherAlloc, OtherSizeT, OtherN>& other);
	constexpr Vector& operator=(const Vector& other);
	template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
	constexpr Vector& operator=(Vector<T, OtherAlloc, OtherSizeT, OtherN>&& other) noexcept;
	constexpr Vector& operator=(Vector&& other) noexcept;

	constexpr Vector& operator=(const InitializerList<T>& il);

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
	[[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept { return data_ + size_; }
	[[nodiscard]] constexpr ReverseIterator<T*>				rend() noexcept { return data_; }
	[[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data_; }

	[[nodiscard]] constexpr const T* cbegin() const noexcept { return data_; }
	[[nodiscard]] constexpr const T* cend() const noexcept { return data_ + size_; }

	[[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept { return data_ + size_; }
	[[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data_; }

	[[nodiscard]] constexpr T& operator[](ptrdiff_t index) noexcept { return data_[index]; }
	[[nodiscard]] constexpr T& operator[](ptrdiff_t index) const noexcept { return data_[index]; }

	[[nodiscard]] constexpr T&			 front() noexcept { return data_[0]; }
	[[nodiscard]] constexpr const T& front() const noexcept { return data_[0]; }
	[[nodiscard]] constexpr T&			 back() noexcept { return data_[size_ - 1]; }
	[[nodiscard]] constexpr const T& back() const noexcept { return data_[size_ - 1]; }

private:
	void maybe_alloc_size();					// if size_ > N
	void maybe_free_and_alloc_size(); // if size_ > current capacity

	void alloc(SizeT capacity);

	void copy(const T* data);
	void move(T* data);

	void definitely_reserve(SizeT capacity);

	template <typename Vector> void steal_ptr(Vector&);

private:
	using Members = internal::VectorMembers<T, Alloc, SizeT, N>;
	using Members::capacity_;
	using Members::data_;
	using Members::size_;
	using Members::small_buffer_;

private:
	template <typename OtherT, typename OtherAlloc, typename OtherSizeT, size_t OtherN> friend class Vector;
};

namespace internal {

template <typename T, typename Alloc, typename SizeT, size_t N> struct VectorMembers {
protected:
	constexpr VectorMembers() = default;
	constexpr VectorMembers(SizeT size) : size_(size) {}

protected:
	static constexpr bool trivial = TriviallyCopyable<T> && TriviallyDestructible<T>;

	using SmallBuffer = ConditionalT<trivial, T[N], unsigned char[sizeof(T) * N]>;

	T*		data_ = reinterpret_cast<T*>(small_buffer_);
	SizeT size_ = 0, capacity_ = N;
	alignas(T) SmallBuffer small_buffer_;
};

template <typename T, typename Alloc, typename SizeT> struct VectorMembers<T, Alloc, SizeT, 0> {
protected:
	constexpr VectorMembers() = default;
	constexpr VectorMembers(SizeT size) : size_(size) {}

protected:
	T*		data_ = nullptr;
	SizeT size_ = 0, capacity_ = 0;

	static constexpr T* small_buffer_ = nullptr;
};

} // namespace internal

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>::Vector(const InitializerList<T>& il) : Members(static_cast<SizeT>(il.size())) {
	maybe_alloc_size();
	copy(il.begin());
}

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>::Vector(SizeT count) : Members(static_cast<SizeT>(count)) {
	maybe_alloc_size();
	for (SizeT i = 0; i < count; ++i)
		new (data_ + i) T();
}

template <typename T, typename Alloc, typename SizeT, size_t N>
template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
constexpr Vector<T, Alloc, SizeT, N>::Vector(const Vector<T, OtherAlloc, OtherSizeT, OtherN>& other) : Members(other.size_) {
	maybe_alloc_size();
	copy(other.data_);
}

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>::Vector(const Vector& other) : Members(other.size_) {
	maybe_alloc_size();
	copy(other.data_);
}

template <typename T, typename Alloc, typename SizeT, size_t N>
template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
constexpr Vector<T, Alloc, SizeT, N>::Vector(Vector<T, OtherAlloc, OtherSizeT, OtherN>&& other) noexcept : Members(other.size_) {
	if constexpr (SameAs<Alloc, OtherAlloc>)
		if constexpr (OtherN > 0) {
			if (other.data_ == other.small_buffer_) {
				if constexpr (OtherN > N)
					maybe_alloc_size();
				move(other.data_);
			} else
				steal_ptr(other);
		} else
			steal_ptr(other);
	else {
		maybe_alloc_size();
		move(other.data_);
	}
}

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>::Vector(Vector&& other) noexcept : Members(other.size_) {
	if constexpr (N > 0) {
		if (other.data_ == reinterpret_cast<T*>(other.small_buffer_))
			move(other.data_);
		else
			steal_ptr(other);
	} else
		steal_ptr(other);
}

template <typename T, typename Alloc, typename SizeT, size_t N> constexpr Vector<T, Alloc, SizeT, N>::~Vector() noexcept {
	clear();
	if constexpr (N > 0) {
		if (data_ != reinterpret_cast<T*>(small_buffer_))
			Alloc::template free<T>(data_, static_cast<Alloc::SizeT>(capacity_));
	} else
		Alloc::template free<T>(data_, static_cast<Alloc::SizeT>(capacity_));
}

template <typename T, typename Alloc, typename SizeT, size_t N>
template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
constexpr Vector<T, Alloc, SizeT, N>& Vector<T, Alloc, SizeT, N>::operator=(const Vector<T, OtherAlloc, OtherSizeT, OtherN>& other) {
	clear();
	size_ = static_cast<SizeT>(other.size_);
	maybe_free_and_alloc_size();
	copy(other.data_);
	return *this;
}

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>& Vector<T, Alloc, SizeT, N>::operator=(const Vector& other) {
	clear();
	size_ = other.size_;
	maybe_free_and_alloc_size();
	copy(other.data_);
	return *this;
}

template <typename T, typename Alloc, typename SizeT, size_t N>
template <typename OtherAlloc, typename OtherSizeT, size_t OtherN>
constexpr Vector<T, Alloc, SizeT, N>& Vector<T, Alloc, SizeT, N>::operator=(Vector<T, OtherAlloc, OtherSizeT, OtherN>&& other) noexcept {
	clear();
	size_ = other.size_;

	if constexpr (SameAs<Alloc, OtherAlloc>)
		if constexpr (OtherN > 0) {
			if (other.data_ == other.small_buffer_) {
				if constexpr (OtherN > N)
					maybe_free_and_alloc_size();
				move(other.data_);
			} else
				steal_ptr(other);
		} else
			steal_ptr(other);
	else {
		maybe_free_and_alloc_size();
		move(other.data_);
	}
	return *this;
}

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>& Vector<T, Alloc, SizeT, N>::operator=(Vector&& other) noexcept {
	clear();
	size_ = other.size_;

	if constexpr (N > 0) {
		if (other.data_ == reinterpret_cast<T*>(other.small_buffer_))
			move(other.data_);
		else
			steal_ptr(other);
	} else
		steal_ptr(other);
	return *this;
}

template <typename T, typename Alloc, typename SizeT, size_t N>
constexpr Vector<T, Alloc, SizeT, N>& Vector<T, Alloc, SizeT, N>::operator=(const InitializerList<T>& il) {
	clear();
	size_ = static_cast<SizeT>(il.size());
	maybe_alloc_size();
	copy(il.begin());
	return *this;
}

template <typename T, typename Alloc, typename SizeT, size_t N>
template <typename... Args>
constexpr void Vector<T, Alloc, SizeT, N>::emplace_back(Args&&... args) {
	if (size_ == capacity_) [[unlikely]] {
		if (capacity_ == 0) [[unlikely]]
			alloc(1);
		else
			definitely_reserve(size_ * 2);
	}
	new (data_ + size_) T(forward<Args>(args)...);
	++size_;
}

template <typename T, typename Alloc, typename SizeT, size_t N> constexpr void Vector<T, Alloc, SizeT, N>::pop_back() noexcept {
	--size_;
	data_[size_].~T();
}

template <typename T, typename Alloc, typename SizeT, size_t N> constexpr void Vector<T, Alloc, SizeT, N>::clear() noexcept {
	if constexpr (!TriviallyDestructible<T>)
		while (size_ > 0)
			pop_back();
	else
		size_ = 0;
}

template <typename T, typename Alloc, typename SizeT, size_t N> constexpr void Vector<T, Alloc, SizeT, N>::reserve(SizeT capacity) {
	if (capacity > capacity_) [[likely]]
		definitely_reserve(capacity);
}

template <typename T, typename Alloc, typename SizeT, size_t N> constexpr void Vector<T, Alloc, SizeT, N>::resize(SizeT count) {
	if (size_ >= count) {
		if constexpr (TriviallyDestructible<T>)
			size_ = count;
		else
			do
				pop_back();
			while (size_ > count);
	} else {
		definitely_reserve(count);
		for (; size_ < count; ++size_)
			new (data_ + size_ - 1) T();
	}
}

template <typename T, typename Alloc, typename SizeT, size_t N> void Vector<T, Alloc, SizeT, N>::maybe_alloc_size() {
	if constexpr (N > 0) {
		if (size_ > static_cast<SizeT>(N))
			alloc(size_);
	} else
		alloc(size_);
}

template <typename T, typename Alloc, typename SizeT, size_t N> void Vector<T, Alloc, SizeT, N>::maybe_free_and_alloc_size() {
	if (size_ > capacity_) {
		Alloc::template free<T>(data_, static_cast<Alloc::SizeT>(capacity_));
		alloc(size_);
	}
}

template <typename T, typename Alloc, typename SizeT, size_t N> void Vector<T, Alloc, SizeT, N>::alloc(SizeT capacity) {
	capacity_ = capacity;
	data_			= Alloc::template alloc<T>(static_cast<Alloc::SizeT>(capacity_));
}

template <typename T, typename Alloc, typename SizeT, size_t N> void Vector<T, Alloc, SizeT, N>::definitely_reserve(SizeT capacity) {
	T* old_data = data_;
	data_				= Alloc::template alloc<T>(static_cast<Alloc::SizeT>(capacity));
	move(old_data);

	SizeT i = size_;

	while (i > 0) {
		--i;
		old_data[i].~T();
	}

	if constexpr (N > 0) {
		if (old_data != reinterpret_cast<T*>(small_buffer_))
			Alloc::template free<T>(old_data, static_cast<Alloc::SizeT>(capacity_));
	} else
		Alloc::template free<T>(old_data, static_cast<Alloc::SizeT>(capacity_));
	capacity_ = capacity;
}

template <typename T, typename Alloc, typename SizeT, size_t N> void Vector<T, Alloc, SizeT, N>::copy(const T* data) {
	if constexpr (TriviallyCopyable<T>)
		memcpy(data_, data, size_ * sizeof(T));
	else
		for (SizeT i = 0; i < size_; ++i)
			new (data_ + i) T(data[i]);
}

template <typename T, typename Alloc, typename SizeT, size_t N> void Vector<T, Alloc, SizeT, N>::move(T* data) {
	if constexpr (TriviallyCopyable<T>)
		memcpy(data_, data, size_ * sizeof(T));
	else
		for (SizeT i = 0; i < size_; ++i)
			new (data_ + i) T(base::move(data[i]));
}

template <typename T, typename Alloc, typename SizeT, size_t N> template <typename V> void Vector<T, Alloc, SizeT, N>::steal_ptr(V& other) {
	data_				= other.data_;
	capacity_		= other.capacity_;
	other.size_ = 0;
	other.data_ = nullptr;
}

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::Vector<int>; // Instantiates to avoid 'arena.hpp is unused'.
#endif
