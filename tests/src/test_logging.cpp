#include "base/logging/file_logger.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int main() {
	constexpr size_t buf_size = 4096;
	// NOLINTNEXTLINE(modernize-avoid-c-arrays)
	char expected[buf_size] = {};
	{
		base::FileLogger logger("test_logs.txt", base::LogLevel::Trace3);

		int		space = buf_size;
		char* end		= static_cast<char*>(expected) + buf_size;

#define PRINT(...) space -= snprintf(end - space, static_cast<size_t>(space), __VA_ARGS__)

		logger.info("hello, {}", "world!");
		logger.trace1("int: {}, hex: {x}.", 5, 31);
		logger.notice("aligned: {010}, centered: {*^7}, left, hex: {<4x}.", 96, 737, 0xf3a);
		logger.warn("+ sign: {0+10}.", 386);
		logger.trace2("centered wstring: |{^10}|", L"hello");
		logger.debug("debug char: {?}, debug string: {?}", '\n', "\x17\0abcde\n\t");
		logger.info("unicode char: {}, debug wstring with unicode: {*>3?}", U'😁', L"nerd emoji: 🤓");

		PRINT("[INFO] hello, world!\n");
		PRINT("[TRACE 1] int: %d, hex: %x.\n", 5, 31U);
		PRINT("[NOTICE] aligned: %010d, centered: **737**, left, hex: f3a .\n", 96);
		PRINT("[WARNING] + sign: 000000+386.\n");
		PRINT("[TRACE 2] centered wstring: |  hello   |\n");
		PRINT("[DEBUG] debug char: '\\n', debug string: %s\n", R"("\x17\0abcde\n\t")");

		const char* unicode = "[INFO] unicode char: 😁, debug wstring with unicode: \"nerd emoji: 🤓\"";
		strcat(end - space, unicode);
		space -= static_cast<int>(strlen(unicode));
	}

	// NOLINTNEXTLINE(modernize-avoid-c-arrays)
	char logs[sizeof(expected)] = {};
	{
		FILE* fp = fopen("test_logs.txt", "rb");
		(void)fread(logs, sizeof(logs), 1, fp);
		(void)fclose(fp);
	}

	(void)remove("test_logs.txt");

	if (memcmp(static_cast<void*>(logs), static_cast<void*>(expected), sizeof(expected)) != 0) {
		printf("Logging test failed. Expected\n%s\n,got\n%s\n.\n", static_cast<const char*>(expected),
					 static_cast<const char*>(logs));
		return 1;
	}
}
