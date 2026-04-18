#include "base/containers/function.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace {
struct Counts {
	alignas(16) uint32_t constructor_call_count = 0;
	uint32_t copy_constructor_call_count				= 0;
	uint32_t move_constructor_call_count				= 0;
	uint32_t destructor_call_count							= 0;
};
struct TestStruct {
	inline static Counts counts;

	static void reset_counts() { counts = Counts{}; }

	TestStruct() { ++counts.constructor_call_count; }

	TestStruct(const TestStruct& /*other*/) { ++counts.copy_constructor_call_count; }
	TestStruct(TestStruct&& /*other*/) noexcept { ++counts.move_constructor_call_count; }

	~TestStruct() noexcept { ++counts.destructor_call_count; }

	TestStruct& operator=(const TestStruct&) = delete;
	TestStruct& operator=(TestStruct&&)			 = delete;
};
} // namespace

static bool check_counts(const Counts& expected) {
	bool ret = true;
	if (expected.constructor_call_count != TestStruct::counts.constructor_call_count) {
		(void)fprintf(stderr, "Incorrect contructor_call_count. Expected %u, got %u.\n",
									expected.constructor_call_count, TestStruct::counts.constructor_call_count);
		ret = false;
	} else {
		printf("Got contructor_call_count = %u as expected.\n",
					 TestStruct::counts.constructor_call_count);
	}
	if (expected.copy_constructor_call_count != TestStruct::counts.copy_constructor_call_count) {
		(void)fprintf(stderr, "Incorrect copy_contructor_call_count. Expected %u, got %u.\n",
									expected.copy_constructor_call_count,
									TestStruct::counts.copy_constructor_call_count);
		ret = false;
	} else {
		printf("Got copy_constructor_call_count = %u, as expected.\n",
					 TestStruct::counts.copy_constructor_call_count);
	}
	if (expected.move_constructor_call_count != TestStruct::counts.move_constructor_call_count) {
		(void)fprintf(stderr, "Incorrect move_contructor_call_count. Expected %u, got %u.\n",
									expected.move_constructor_call_count,
									TestStruct::counts.move_constructor_call_count);
		ret = false;
	} else {
		printf("Got move_constructor_call_count = %u as expected.\n",
					 TestStruct::counts.move_constructor_call_count);
	}
	if (expected.destructor_call_count != TestStruct::counts.destructor_call_count) {
		(void)fprintf(stderr, "Incorrect destructor_call_count. Expected %u, got %u\n.",
									expected.destructor_call_count, TestStruct::counts.destructor_call_count);
		ret = false;
	} else {
		printf("Got destructor_call_count = %u as expected.\n",
					 TestStruct::counts.destructor_call_count);
	}
	TestStruct::reset_counts();
	return ret;
}

namespace {
class TestAllocator {
public:
	inline static uint32_t allocate_call_count = 0;
	inline static uint32_t free_call_count		 = 0;

	template <typename T> T* allocate() {
		++allocate_call_count;
		return static_cast<T*>(malloc(sizeof(T)));
	}
	template <typename T> void free(T* ptr) {
		++free_call_count;
		::free(ptr);
	}
};
} // namespace

int main() {
	{
		bool									 ran = false;
		base::Function<void()> func([&ran]() { ran = true; });

		if (ran) {
			(void)fprintf(stderr, "Function ran before it was supposed to.");
			return 1;
		}
		func();
		if (!ran) {
			(void)fprintf(stderr, "Function didn't run.");
			return 1;
		}
	}

	{
		if (!check_counts(Counts{})) {
			return 1;
		}
		TestStruct test;
		if (!check_counts(Counts{.constructor_call_count = 1})) {
			return 1;
		}
		base::Function<int(int)> func([test](int value) { return value * 2; });
		if (!check_counts(Counts{
						.copy_constructor_call_count = 1,
						.move_constructor_call_count = 1,
						.destructor_call_count			 = 1,
				})) {
			return 1;
		}
		int result = func(10);
		if (result != 20) {
			(void)fprintf(stderr, "function returned %d, not 20", result);
		}
	}
	if (!check_counts(Counts{.destructor_call_count = 2})) {
		return 1;
	}

	{
		TestStruct test;
		if (!check_counts(Counts{.constructor_call_count = 1})) {
			return 1;
		}

		base::Function<void(), 0, TestAllocator> func3([&test]() { (void)test; });

		if (!check_counts(Counts{})) {
			return 1;
		}

		if (TestAllocator::allocate_call_count != 1) {
			(void)fprintf(stderr, "Expected 1 allocation, got %u", TestAllocator::allocate_call_count);
		}
		if (TestAllocator::free_call_count != 0) {
			(void)fprintf(stderr, "Expected 0 frees, got %u", TestAllocator::free_call_count);
		}
	}
	if (!check_counts(Counts{.destructor_call_count = 1})) {
		return 1;
	}

	{
		TestStruct test;
		if (!check_counts(Counts{.constructor_call_count = 1})) {
			return 1;
		}
		auto lambda = [test]() {};
		if (!check_counts(Counts{.copy_constructor_call_count = 1})) {
			return 1;
		}
		base::Function<void()> func(lambda);
		if (!check_counts(Counts{.copy_constructor_call_count = 1})) {
			return 1;
		}
	}
	if (!check_counts(Counts{.destructor_call_count = 3})) {
		return 1;
	}
}
