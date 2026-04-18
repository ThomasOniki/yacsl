#include "base/memory/arena.hpp"
#include "test_utils.hpp"

#include <stdio.h>

#include <array>

constexpr uint32_t iteration_count = 10000;

namespace {
struct CtorCounter {
	inline static uint32_t ctor_call_count = 0;
	explicit CtorCounter(uint32_t num) : i_(num) { ++ctor_call_count; }
	[[maybe_unused]] CtorCounter() { ++ctor_call_count; }
	uint32_t i_ = 0;
};

struct Aligned { // NOLINT(altera-struct-pack-align)
	alignas(64) char data;
};
} // namespace

static int test_ctors() {
	int ret											 = 0;
	CtorCounter::ctor_call_count = 0;

	std::array<CtorCounter*, iteration_count> ptrs = {};
	for (uint32_t i = 0; i < iteration_count; ++i) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		ptrs[i] = &base::MainArenaAllocator::get().push<CtorCounter>(i);
	}

	EXPECT_EQ(CtorCounter::ctor_call_count, iteration_count);

	for (uint32_t i = 0; i < iteration_count; ++i) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		EXPECT_EQ(ptrs[i]->i_, i);
	}

	CtorCounter::ctor_call_count = 0;

	for (uint32_t i = 0; i < iteration_count; ++i) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		ptrs[i] = base::MainArenaAllocator::get().push_array<CtorCounter>(10);
	}

	EXPECT_EQ(CtorCounter::ctor_call_count, iteration_count * 10);

	for (uint32_t iter = 0; iter < iteration_count; ++iter) {
		for (uint32_t i = 0; i < 10; ++i) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
			EXPECT_EQ(ptrs[iter]->i_, 0U);
		}
	}
	return ret;
}

static bool is_aligned(Aligned* ptr) {
	auto uptr = reinterpret_cast<uintptr_t>(ptr);
	return uptr % alignof(Aligned) == 0;
}

static int test_align() {
	int ret = 0;
	(void)base::MainArenaAllocator::get().push_array<char>(15);

	Aligned* aligned = &base::MainArenaAllocator::get().push<Aligned>();

	EXPECT(is_aligned(aligned));

	(void)base::MainArenaAllocator::get().push_array<char>(15);

	aligned = base::MainArenaAllocator::get().push_array<Aligned>(10);

	EXPECT(is_aligned(aligned));

	return ret;
}

static int test_rewind() {
	int ret = 0;
	{
		base::Arena::Scope scope(base::MainArenaAllocator::get()); // test scope with ptr_ = 0
	}
	// Calls malloc so that the rewind test uses the same memory in and outside of the scope.
	(void)base::MainArenaAllocator::get().push<int>();
	char* ptr = nullptr;
	{
		base::Arena::Scope scope(base::MainArenaAllocator::get());

		ptr = base::MainArenaAllocator::get().push_array<char>(10);

		test_align();
		test_ctors();
	}

	EXPECT(base::MainArenaAllocator::get().push_array<char>(1) == ptr && "Arena didn't rewind");
	return ret;
}

int main() {
	int ret = 0;
	EXPECT_EQ(test_align(), 0);
	EXPECT_EQ(test_ctors(), 0);
	EXPECT_EQ(test_rewind(), 0);
	return ret;
}
