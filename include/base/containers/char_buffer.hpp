#pragma once

#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class CharBuffer {
public:
	void append(const StringView& str);
	void append(char chr);

	void reserve(usize capacity);

	// This doesn't initialize the new memory but adds count to size. It is usuful for characters you
	// want to memcpy into. It returns new_end - count.
	char* grow(usize count);

	[[nodiscard]] usize size() const noexcept { return static_cast<usize>(end_ - begin_); }
	[[nodiscard]] usize capacity() const noexcept {
		return static_cast<usize>(allocated_end_ - begin_);
	}

	[[nodiscard]] char*				data() noexcept { return begin_; }
	[[nodiscard]] const char* data() const noexcept { return begin_; }

	[[nodiscard]] char*				begin() noexcept { return begin_; }
	[[nodiscard]] char*				end() noexcept { return end_; }
	[[nodiscard]] const char* begin() const noexcept { return begin_; }
	[[nodiscard]] const char* end() const noexcept { return end_; }
	[[nodiscard]] const char* cbegin() const noexcept { return begin_; }
	[[nodiscard]] const char* cend() const noexcept { return end_; }

private:
	void definitely_reserve(usize capacity);

private:
	StackArray<char, 1> fixed_				 = {'\0'};
	char*								begin_				 = fixed_.data();
	char*								end_					 = fixed_.data();
	char*								allocated_end_ = fixed_.end();
};

} // namespace base
