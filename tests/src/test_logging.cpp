#include "base/containers/initializer_list.hpp"
#include "base/logging/file_logger.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// NOLINTBEGIN

int main() {
	int ret = 0;

	constexpr size_t buf_size = 4096;
	{

#define PRINT(func)                                                                                \
	do {                                                                                             \
		base::FileLogger logger("test_logs.txt", base::LogLevel::Trace3);                              \
		logger.func;                                                                                   \
	} while (false)

		char buf[buf_size] = {};
		int	 buf_space		 = buf_size;

#define CHECK(...)                                                                                 \
	do {                                                                                             \
		buf_space -=                                                                                   \
				snprintf(buf + buf_size - buf_space, static_cast<size_t>(buf_space), __VA_ARGS__);         \
		char	logs[buf_size] = {};                                                                     \
		FILE* fp						 = fopen("test_logs.txt", "rb");                                           \
		(void)fread(logs, sizeof(logs), 1, fp);                                                        \
		(void)fclose(fp);                                                                              \
                                                                                                   \
		if (memcmp(static_cast<void*>(logs), static_cast<void*>(buf), buf_size) != 0) {                \
			printf("Logging test failed. Expected\n%s,got\n%s.\n", buf, logs);                           \
			ret = 1;                                                                                     \
		}                                                                                              \
	} while (0)

		PRINT(info("Hello, {}", "world!"));
		CHECK("[INFO] Hello, world!\n");

		PRINT(trace1("int: {}, hex: {x}.", 5, 31));
		CHECK("[TRACE 1] int: %d, hex: %x.\n", 5, 31U);

		PRINT(notice("aligned: {010}, centered: {*^7}, left, hex: {<4x}.", 96, 737, 0xf3a));
		CHECK("[NOTICE] aligned: %010d, centered: **737**, left, hex: f3a .\n", 96);

		PRINT(warn("+ sign: {0+10}.", 386));
		CHECK("[WARNING] + sign: 000000+386.\n");

		PRINT(debug("int list: {}, hex int list: {08x}", base::InitializerList<int>{0, -1, 20, -35},
								base::InitializerList<int>{0xF, -0x1a, 0x1, 0x0}));
		CHECK("[DEBUG] int list: [0, -1, 20, -35], hex int list: [%08x, 00000-1a, %08x, %08x]\n", 0xFU,
					0x1U, 0x0U);

		PRINT(trace2("centered wstring: |{^10}|", L"hello"));
		CHECK("[TRACE 2] centered wstring: |  hello   |\n");

		PRINT(debug("debug char: {?}, debug string: {?}", '\n', "\x17\0abcde\n\t"));
		CHECK("[DEBUG] debug char: '\\n', debug string: %s\n", R"("\x17\0abcde\n\t")");

		PRINT(notice("unicode char: {}, debug wstring with unicode: {*>3?}", U'😁', L"nerd emoji: 🤓"));
		CHECK("[NOTICE] unicode char: 😁, debug wstring with unicode: \"nerd emoji: 🤓\"\n");

		PRINT(critical("char16_t string range: {*^10}",
									 base::InitializerList<base::BasicStringView<char16_t>>{
											 u"foo", u"bar", u"🤓", u"⭐",
											 u"The emoji aren't padded correctly because they use 4 UTF-8 chars."}));
		CHECK("[CRITICAL] char16_t string range: [***foo****, ***bar****, ***🤓***, ***⭐****, "
					"The emoji aren't padded correctly because they use 4 UTF-8 chars.]\n");

		PRINT(notice("debug char16_t string: {?}", u"nerd emoji (again): 🤓, newline: \n"));
		CHECK("[NOTICE] debug char16_t string: \"nerd emoji (again): 🤓, newline: \\n\"\n");

		PRINT(info("Goodbye, world!"));
		CHECK("[INFO] Goodbye, world!\n");
	}

	(void)remove("test_logs.txt");

	return ret;
}

// NOLINTEND
