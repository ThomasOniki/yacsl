#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/reverse_iterator.hpp"

namespace base {

template <typename T, typename SizeT> class BasicStringView {
public:
	constexpr BasicStringView() = default;
	constexpr BasicStringView(const T* data, SizeT size) : data_(data), size_(size) {}
	constexpr BasicStringView(const T* begin, const T* end)
			: data_(begin), size_(static_cast<SizeT>(end - begin)) {}
	// NOLINTNEXTLINE
	template <SizeT N> constexpr BasicStringView(const T (&arr)[N]) : data_(arr), size_(N - 1) {}

	[[nodiscard]] const T* data() const noexcept { return data_; }
	[[nodiscard]] SizeT		 size() const noexcept { return size_; }

	[[nodiscard]] constexpr const T& front() const noexcept { return data_[0]; }
	[[nodiscard]] constexpr const T& back() const noexcept { return data_[size_ - 1]; }

	[[nodiscard]] constexpr T& operator[](SizeT index) noexcept { return data_[index]; }
	[[nodiscard]] constexpr T& operator[](SizeT index) const noexcept { return data_[index]; }

	[[nodiscard]] constexpr const T* begin() const noexcept { return data_; }
	[[nodiscard]] constexpr const T* end() const noexcept { return data_ + size_; }
	[[nodiscard]] constexpr const T* cbegin() const noexcept { return data_; }
	[[nodiscard]] constexpr const T* cend() const noexcept { return data_ + size_; }

	[[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept {
		return data_ + size_;
	}
	[[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data_; }
	[[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept {
		return data_ + size_;
	}
	[[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data_; }

private:
	const T* data_ = nullptr;
	SizeT		 size_ = 0;
};

using StringView = BasicStringView<char>;

} // namespace base
