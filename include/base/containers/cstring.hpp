#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/maybe_allocator_member.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/memory/arena.hpp" // NOLINT(misc-include-cleaner)
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

template <typename Char, usize N, typename Allocator, typename SizeT>
class BasicCString : MaybeAllocatorMember<Allocator> {
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
		if (cstr_ != fixed_.data()) {
			allocator().free(cstr_);
		}
	}

	BasicCString& operator=(const BasicCString&) = delete;
	BasicCString& operator=(BasicCString&&)			 = delete;

	[[nodiscard]] const Char* data() const noexcept { return cstr_; }
	[[nodiscard]] Char*				data() noexcept { return cstr_; }

private:
	StackArray<Char, N, SizeT> fixed_ = {};
	Char*											 cstr_	= fixed_.data();

private:
	using MaybeAllocatorMember<Allocator>::allocator;
};

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::BasicCString<char>;
template base::BasicCString<char>::BasicCString(const BasicStringView<char>& str);
#endif
