#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/reverse_iterator.hpp"
#include "base/containers/structured_bindings.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, usize N, typename SizeT> class StackArray {
public:
	[[nodiscard]] constexpr usize size() const noexcept { return N; }

	[[nodiscard]] constexpr T* data() noexcept BASE_LIFETIMEBOUND { return static_cast<T*>(data_); }
	[[nodiscard]] constexpr const T* data() const noexcept BASE_LIFETIMEBOUND {
		return static_cast<const T*>(data_);
	}

	[[nodiscard]] constexpr T& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
		return data()[index];
	}
	[[nodiscard]] constexpr const T& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
		return data()[index];
	}

	[[nodiscard]] constexpr T&			 front() noexcept BASE_LIFETIMEBOUND { return data_[0]; }
	[[nodiscard]] constexpr T&			 back() noexcept BASE_LIFETIMEBOUND { return data_[N - 1]; }
	[[nodiscard]] constexpr const T& front() const noexcept BASE_LIFETIMEBOUND { return data_[0]; }
	[[nodiscard]] constexpr const T& back() const noexcept BASE_LIFETIMEBOUND { return data_[N - 1]; }

	[[nodiscard]] constexpr T*			 begin() noexcept BASE_LIFETIMEBOUND { return data(); }
	[[nodiscard]] constexpr T*			 end() noexcept BASE_LIFETIMEBOUND { return data() + N; }
	[[nodiscard]] constexpr const T* begin() const noexcept BASE_LIFETIMEBOUND { return data(); }
	[[nodiscard]] constexpr const T* end() const noexcept BASE_LIFETIMEBOUND { return data() + N; }
	[[nodiscard]] constexpr const T* cbegin() const noexcept BASE_LIFETIMEBOUND { return data(); }
	[[nodiscard]] constexpr const T* cend() const noexcept BASE_LIFETIMEBOUND { return data() + N; }

	[[nodiscard]] constexpr ReverseIterator<T*> rbegin() noexcept BASE_LIFETIMEBOUND { data() + N; }
	[[nodiscard]] constexpr ReverseIterator<T*> rend() noexcept BASE_LIFETIMEBOUND { return data(); }
	[[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept BASE_LIFETIMEBOUND {
		return data() + N;
	}
	[[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept BASE_LIFETIMEBOUND {
		return data();
	}
	[[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept BASE_LIFETIMEBOUND {
		return data() + N;
	}
	[[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept BASE_LIFETIMEBOUND {
		return data();
	}

	template <usize I> [[nodiscard]] constexpr T&				get() noexcept { return data_[I]; }
	template <usize I> [[nodiscard]] constexpr const T& get() const noexcept { return data_[I]; }

	constexpr bool operator==(const StackArray& other) const noexcept = default;

public:
	T data_[N]; // NOLINT
};

template <typename T, usize N, typename SizeT> struct TupleSize<StackArray<T, N, SizeT>> {
	static constexpr usize value = N;
};

template <typename T, usize N, typename SizeT, usize I>
struct TupleElement<StackArray<T, N, SizeT>, I> {
	using Type = T;
};

} // namespace base
