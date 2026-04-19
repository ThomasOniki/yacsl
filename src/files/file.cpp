#ifdef BASE_PLATFORM_UNIX

#include "base/files/file.hpp"
#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/files/common.hpp"
#include "base/utilities/type_aliases.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace base {

static bool strip_flag(FileOpenFlags* flags, FileOpenFlags flag) {
	const FileOpenFlags stripped = *flags & ~flag;
	if (stripped != *flags) {
		*flags = stripped;
		return true;
	}
	return false;
}

static int unix_flags(FileOpenFlags flags) {
	unsigned int ret = 0;

	if (strip_flag(&flags, FileOpenFlags::Trunc)) {
		ret |= O_TRUNC;
	}
	if (strip_flag(&flags, FileOpenFlags::Create)) {
		ret |= O_CREAT;
	}
	if (strip_flag(&flags, FileOpenFlags::Append)) {
		ret |= O_APPEND;
	}
	if (flags == FileOpenFlags::Read) {
		ret |= O_RDONLY;
	} else if (flags == FileOpenFlags::Write) {
		ret |= O_WRONLY;
	} else {
		ret |= O_RDWR;
	}
	return static_cast<int>(ret);
}

void File::open(const StringView& path, FileOpenFlags flags) {
	CharBuffer buf;
	char*			 cstr_path = buf.grow(path.size() + 1);
	memcpy(cstr_path, path.data(), path.size());
	cstr_path[path.size()] = '\0';
	const bool seek_end		 = strip_flag(&flags, FileOpenFlags::SeekEnd) &&
													 (flags & FileOpenFlags::Trunc) == FileOpenFlags::None;
	handle_ = ::open(cstr_path, unix_flags(flags), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (handle_ == -1) {
		perror("::open failed");
		return;
	}
	if (seek_end) {
		if (lseek(handle_, 0, SEEK_END) == -1) [[unlikely]] {
			perror("::lseek failed");
		}
	}
}

void File::close() {
	if (is_open()) {
		if (::close(handle_) == -1) [[unlikely]] {
			perror("::close failed");
		}
		handle_ = NIL_FILE_HANDLE;
	}
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void File::write(const StringView& data) {
	if (::write(handle_, data.data(), data.size()) == -1) [[unlikely]] {
		perror("::write() failed");
	}
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void File::write(const BasicStringView<uchar>& data) {
	if (::write(handle_, data.data(), data.size()) == -1) [[unlikely]] {
		perror("::write() failed");
	}
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void File::read(char* buf, usize size) {
	if (::read(handle_, buf, size) == -1) [[unlikely]] {
		perror("::read() failed");
	}
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void File::read(uchar* buf, usize size) {
	if (::read(handle_, buf, size) == -1) [[unlikely]] {
		perror("::read() failed");
	}
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::get_cursor() {
	isize result = ::lseek(handle_, 0, SEEK_CUR);
	if (result == -1) [[unlikely]] {
		perror("::lseek failed");
	}
	return result;
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::seek_begin(isize offset) {
	isize result = ::lseek(handle_, offset, SEEK_SET);
	if (result == -1) [[unlikely]] {
		perror("::lseek() failed");
	}
	return result;
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::seek_end(isize offset) {
	isize result = ::lseek(handle_, offset, SEEK_END);
	if (result == -1) [[unlikely]] {
		perror("::lseek() failed");
	}
	return result;
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::seek_cursor(isize offset) {
	isize result = ::lseek(handle_, offset, SEEK_CUR);
	if (result == -1) [[unlikely]] {
		perror("::lseek() failed");
	}
	return result;
}

} // namespace base

#else

OutFile::OutFile(const StringView& path) {}

#endif
