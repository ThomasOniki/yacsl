#include "base/logging/file_logger.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// NOLINTBEGIN

int main() {
	constexpr size_t buf_size						= 4096;
	char						 expected[buf_size] = {};
	{
		base::FileLogger logger("test_logs.txt", base::LogLevel::Trace3);

		int		space = buf_size;
		char* end		= expected + buf_size;

#define PRINT(...) space -= snprintf(end - space, static_cast<size_t>(space), __VA_ARGS__)

		logger.info("hello, {}", "world!");
		logger.trace1("int: {}, hex: {x}.", 42, 31);
		logger.notice("aligned: {010}, centered: {*^7}, left, hex: {<4x}.", 96, 737, 0xf3a);
		logger.warn("+ sign: {0+10}.", 386);

		PRINT("[INFO] hello, world!\n");
		PRINT("[TRACE 1] int: %d, hex: %x.\n", 42, 31u);
		PRINT("[NOTICE] aligned: %010d, centered: **737**, left, hex: f3a .\n", 96);
		PRINT("[WARNING] + sign: 000000+386.\n");
	}

	char logs[sizeof(expected)] = {};
	{
		FILE* fp = fopen("test_logs.txt", "rb");
		fread(logs, sizeof(logs), 1, fp);
		fclose(fp);
	}

	(void)remove("test_logs.txt");

	if (memcmp(logs, expected, sizeof(expected)) != 0) {
		printf("Logging test failed. Expected\n%s\n,got\n%s\n.\n", expected, logs);
		return 1;
	}
}

// NOLINTEND
