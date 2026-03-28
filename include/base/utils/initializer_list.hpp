#pragma once

#ifdef BASE_NO_LIBCPP

#include "base/utils/size_t.hpp"

namespace std {

template <typename T> class initializer_list {
public:
	constexpr initializer_list() noexcept : data_(nullptr), size_(0) {}

	[[nodiscard]] constexpr base::size_t size() const noexcept { return size_; }
	[[nodiscard]] constexpr const T*		 begin() const noexcept { return data_; }
	[[nodiscard]] constexpr const T*		 end() const noexcept { return data_ + size_; }

public:
	using value_type			= T;
	using reference				= const T&;
	using const_reference = const T&;
	using size_type				= base::size_t;

	using iterator			 = const T*;
	using const_iterator = const T*;

private:
	initializer_list(const T* data, base::size_t size) : data_(data), size_(size) {}

private:
	const T*		 data_;
	base::size_t size_;
};

} // namespace std

#else
#include <initializer_list>
#endif

namespace base {

template <typename T> using InitializerList = std::initializer_list<T>;

}
