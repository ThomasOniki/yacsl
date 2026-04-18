#include "base/containers/function.hpp"
#include "test_utils.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace {
struct Counts {
	alignas(16) uint32_t constructor_call_count = 0;
	uint32_t copy_constructor_call_count				= 0;
	uint32_t move_constructor_call_count				= 0;
	uint32_t destructor_call_count							= 0;

	constexpr bool operator==(const Counts&) const noexcept = default;
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

template <> class base::Formatter<Counts> {
public:
	constexpr const char* parse(const char* fmt) {
		base::FormatStringParser parser(fmt);
		return parser.end_parse();
	}
	void format(base::CharBuffer& buf, const Counts& counts) const {
		base::format_to(
				buf, "constructors: {}, copy_constructors: {}, move_contructors: {}, destructors: {}",
				counts.constructor_call_count, counts.copy_constructor_call_count,
				counts.move_constructor_call_count, counts.destructor_call_count);
	}
};

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

#define CHECK_COUNTS(...)                                                                          \
	do {                                                                                             \
		const Counts expected = Counts{__VA_ARGS__};                                                   \
		EXPECT_EQ(TestStruct::counts, expected);                                                       \
		TestStruct::reset_counts();                                                                    \
	} while (false)

static int test_function_running() {
	int										 ret = 0;
	bool									 ran = false;
	base::Function<void()> func([&ran]() { ran = true; });

	EXPECT(!ran);
	func();
	EXPECT(ran);
	return ret;
}

static int test_moved_lambda() {
	int ret = 0;
	{
		CHECK_COUNTS(0);

		TestStruct test;

		CHECK_COUNTS(.constructor_call_count = 1);

		base::Function<int(int)> func([test](int value) { return value * 2; });

		CHECK_COUNTS(.copy_constructor_call_count = 1, .move_constructor_call_count = 1,
								 .destructor_call_count = 1);

		int result = func(10);
		if (result != 20) {
			(void)fprintf(stderr, "function returned %d, not 20", result);
		}
	}
	CHECK_COUNTS(.destructor_call_count = 2);
	return ret;
}

static int test_copied_lambda() {
	int ret = 0;
	{
		TestStruct test;
		CHECK_COUNTS(.constructor_call_count = 1);

		base::Function<void(), 0, TestAllocator> func3([&test]() { (void)test; });

		CHECK_COUNTS(0);

		if (TestAllocator::allocate_call_count != 1) {
			(void)fprintf(stderr, "Expected 1 allocation, got %u", TestAllocator::allocate_call_count);
		}
		if (TestAllocator::free_call_count != 0) {
			(void)fprintf(stderr, "Expected 0 frees, got %u", TestAllocator::free_call_count);
		}
	}
	CHECK_COUNTS(.destructor_call_count = 1);
	return ret;
}

static int test_ref_capture_lamdba() {
	int ret = 0;
	{
		TestStruct test;
		CHECK_COUNTS(.constructor_call_count = 1);

		auto lambda = [test]() {};
		CHECK_COUNTS(.copy_constructor_call_count = 1);

		base::Function<void()> func(lambda);
		CHECK_COUNTS(.copy_constructor_call_count = 1);
	}
	CHECK_COUNTS(.destructor_call_count = 3);

	return ret;
}

int main() {
	int ret = 0;
	EXPECT_EQ(test_function_running(), 0);
	EXPECT_EQ(test_moved_lambda(), 0);
	EXPECT_EQ(test_copied_lambda(), 0);
	EXPECT_EQ(test_ref_capture_lamdba(), 0);
	return ret;
}
