#pragma once

#include "base/utilities/type_aliases.hpp"

namespace base {

#ifdef BASE_PLATFORM_UNIX
using FileHandle										 = int;
constexpr FileHandle NIL_FILE_HANDLE = -1;
#else
using FileFileHandle								 = void*;
constexpr FileHandle NIL_FILE_HANDLE = nullptr;
#endif

enum class FileOpenFlags : u8 {
	None		= 0,
	Read		= 1 << 0,
	Write		= 1 << 1,
	Trunc		= 1 << 2,
	Create	= 1 << 3,
	SeekEnd = 1 << 4,
	Append	= 1 << 5,
};
constexpr FileOpenFlags operator|(FileOpenFlags lhs, FileOpenFlags rhs) noexcept {
	return FileOpenFlags(static_cast<u8>(static_cast<u8>(lhs) | static_cast<u8>(rhs)));
}
constexpr FileOpenFlags operator&(FileOpenFlags lhs, FileOpenFlags rhs) noexcept {
	return FileOpenFlags(static_cast<u8>(static_cast<u8>(lhs) & static_cast<u8>(rhs)));
}
constexpr FileOpenFlags operator~(FileOpenFlags flags) noexcept {
	return FileOpenFlags(~static_cast<u8>(flags));
}

} // namespace base
