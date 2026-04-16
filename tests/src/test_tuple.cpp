#include "base/containers/tuple.hpp"
#include <array>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
	base::Tuple<int, const char*> tpl = {0, "hello"};

	auto [num, str] = tpl;

	std::array<char, 64> buf = {};
	(void)snprintf(buf.data(), 64, "int: %i, string: %s", num, str);

	assert(strcmp(buf.data(), "int: 0, string: hello") == 0);
}
