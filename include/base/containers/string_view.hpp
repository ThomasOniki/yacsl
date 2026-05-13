#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/reverse_iterator.hpp"
#include "base/utilities/is_constant_evaluated.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

template <typename T, typename SizeT> class BasicStringView {
public:
	constexpr BasicStringView() = default;
	constexpr BasicStringView(BASE_LIFETIMEBOUND const T* data, SizeT size)
			: data_(data), size_(size) {}
	constexpr BasicStringView(BASE_LIFETIMEBOUND const T* begin, BASE_LIFETIMEBOUND const T* end)
			: data_(begin), size_(static_cast<SizeT>(end - begin)) {}
	template <SizeT N> // NOLINTNEXTLINE(google-explicit-constructor, modernize-avoid-c-arrays)
	constexpr BasicStringView(BASE_LIFETIMEBOUND const T (&arr)[N]) : data_(arr), size_(N - 1) {}

	[[nodiscard]] const T* data() const noexcept BASE_LIFETIMEBOUND { return data_; }
	[[nodiscard]] SizeT		 size() const noexcept BASE_LIFETIMEBOUND { return size_; }

	[[nodiscard]] constexpr const T& front() const noexcept { return data_[0]; }
	[[nodiscard]] constexpr const T& back() const noexcept { return data_[size_ - 1]; }

	[[nodiscard]] constexpr T&			 operator[](SizeT index) noexcept { return data_[index]; }
	[[nodiscard]] constexpr const T& operator[](SizeT index) const noexcept { return data_[index]; }

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

	[[nodiscard]] constexpr bool operator==(const BasicStringView& other) const noexcept {
		if (size_ != other.size_) {
			return false;
		}
		if (is_constant_evaluated()) {
			for (size_t i = 0; i < size_; ++i) {
				if (other[i] != data_[i]) {
					return false;
				}
			}
			return true;
		}
		return memcmp(data_, other.data_, size_) == 0;
	}

private:
	const T* data_ = nullptr;
	SizeT		 size_ = 0;
};

template <typename Char, typename SizeT>
BasicStringView(const Char*, SizeT) -> BasicStringView<Char, SizeT>;
template <typename Char> BasicStringView(const Char*, const Char*) -> BasicStringView<Char, usize>;
template <typename Char, usize N> // NOLINTNEXTLINE(modernize-avoid-c-arrays)
BasicStringView(const Char (&)[N]) -> BasicStringView<Char, usize>;

using StringView = BasicStringView<char>;

} // namespace base
