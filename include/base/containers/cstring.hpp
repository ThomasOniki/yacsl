#pragma once

#include "base/containers/base_clases.hpp"
#include "base/containers/fwds.hpp"
#include "base/containers/string_view.hpp"
#include "base/memory/arena.hpp" // NOLINT(misc-include-cleaner)
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

template <typename Char, usize N, typename Allocator, typename SizeT>
class BasicCString : AllocatorMember<Allocator>, FixedBufferMember<Char, N, SizeT> {
public:
	constexpr BasicCString() noexcept = default;

	template <typename OtherSizeT> // NOLINTNEXTLINE(google-explicit-constructor)
	constexpr BasicCString(const BasicStringView<Char, OtherSizeT>& str) noexcept {
		if (str.size() > N) {
			cstr_ = allocator().template allocate<Char>(str.size() + 1);
		}
		memcpy(cstr_, str.data(), str.size());
		cstr_[str.size()] = '\0';
	}

	BasicCString(const BasicCString& other) = delete;
	BasicCString(BasicCString&& other)			= delete;

	~BasicCString() noexcept {
		if (cstr_ != fixed_buffer_.data()) {
			allocator().free(cstr_);
		}
	}

	BasicCString& operator=(const BasicCString&) = delete;
	BasicCString& operator=(BasicCString&&)			 = delete;

	[[nodiscard]] const Char* data() const noexcept { return cstr_; }
	[[nodiscard]] Char*				data() noexcept { return cstr_; }

private:
	using FixedBufferMember<Char, N, SizeT>::fixed_buffer_;
	using AllocatorMember<Allocator>::allocator;

	Char* cstr_ = fixed_buffer_.data();
};

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::BasicCString<char>;
template base::BasicCString<char>::BasicCString(const BasicStringView<char>& str);
#endif
