#include "base/containers/fwds.hpp"
#include "base/containers/initializer_list.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/file_logger.hpp"
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int& status() {
	static int status;
	return status;
};

static void check(const char* expected) {
	static constexpr size_t buf_size		 = 4096;
	const size_t						expected_len = strlen(expected);

	assert(expected_len - 2 < buf_size);
	base::StackArray<char, buf_size, size_t> buf = {};
	memcpy(buf.data(), expected, expected_len);
	buf[expected_len] = '\n';

	base::StackArray<char, buf_size> logs = {};
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	FILE* file = fopen("test_logs.txt", "rbe");
	assert(file);
	(void)fread(logs.data(), sizeof(logs), 1, file);
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	(void)fclose(file);

	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	[[maybe_unused]] FILE* truncated = fopen("test_logs.txt", "w+e");
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	(void)fclose(truncated);

	if (memcmp(static_cast<void*>(logs.data()), static_cast<const void*>(buf.data()), buf_size) !=
			0) {
		printf("Logging test failed. Expected\n%s,got\n%s.\n", buf.data(), logs.data());
		status() = 1;
	}
}

int main() {
	base::FileLogger logger("test_logs.txt", base::LogLevel::Trace3);

	logger.info("Hello, {}", "world!");
	check("[INFO] Hello, world!");

	logger.trace1("int: {}, hex: {x}.", 5, 0x3a);
	check("[TRACE 1] int: 5, hex: 3a.");

	logger.notice("aligned: {010}, centered: {*^7}, left, hex: {<4x}.", 96, 737, 0xf3a);
	check("[NOTICE] aligned: 0000000096, centered: **737**, left, hex: f3a .");

	logger.warn("+ sign: {0+10}.", 386);
	check("[WARNING] + sign: 000000+386.");

	logger.debug("int list: {}, hex int list: {08x}", base::InitializerList<int>{0, -1, 20, -35},
							 base::InitializerList<int>{0xF, -0x1a, 0x1, 0x0});
	check("[DEBUG] int list: [0, -1, 20, -35], hex int list: [0000000f, 00000-1a, 00000001, "
				"00000000]");

	logger.trace2("centered wstring: |{^10}|", L"hello");
	check("[TRACE 2] centered wstring: |  hello   |");

	logger.debug("debug char: {?}, debug string: {?}", '\n', "\x17\0abcde\n\t");
	check(R"([DEBUG] debug char: '\n', debug string: "\x17\0abcde\n\t")");

	logger.notice("unicode char: {}, debug wstring with unicode: {*>3?}", U'😁', L"nerd emoji: 🤓");
	check("[NOTICE] unicode char: 😁, debug wstring with unicode: \"nerd emoji: 🤓\"");

	logger.critical("char16_t string range: {*^10}",
									base::InitializerList<base::BasicStringView<char16_t>>{
											u"foo", u"bar", u"🤓", u"⭐",
											u"The emoji aren't padded correctly because they use 4 UTF-8 chars."});
	check("[CRITICAL] char16_t string range: [***foo****, ***bar****, ***🤓***, ***⭐****, "
				"The emoji aren't padded correctly because they use 4 UTF-8 chars.]");

	logger.notice("debug char16_t string: {?}", u"nerd emoji (again): 🤓, newline: \n");
	check("[NOTICE] debug char16_t string: \"nerd emoji (again): 🤓, newline: \\n\"");

	logger.info("Goodbye, world!");
	check("[INFO] Goodbye, world!");

	(void)remove("test_logs.txt");

	return status();
}
