#include "base/memory/arena.hpp"

#include <gtest/gtest.h>
#include <stdio.h>

#include <array>

static uint32_t		 ctor_call_count = 0;
constexpr uint32_t iteration_count = 10000;

namespace {
struct CtorCounter {
	explicit CtorCounter(uint32_t num) : i_(num) { ++ctor_call_count; }
	[[maybe_unused]] CtorCounter() { ++ctor_call_count; }
	uint32_t i_ = 0;
};

struct Aligned {
	alignas(64) char data;
};
} // namespace

static void test_ctors() {
	ctor_call_count = 0;

	std::array<CtorCounter*, iteration_count> ptrs = {};
	for (uint32_t i = 0; i < iteration_count; ++i) {
		ptrs[i] = &base::MainArenaAllocator::get().push<CtorCounter>(i);
	}

	EXPECT_EQ(ctor_call_count, iteration_count);

	for (uint32_t i = 0; i < iteration_count; ++i) {
		EXPECT_EQ(ptrs[i]->i_, i);
	}

	ctor_call_count = 0;

	for (uint32_t i = 0; i < iteration_count; ++i) {
		ptrs[i] = base::MainArenaAllocator::get().push_array<CtorCounter>(10);
	}

	EXPECT_EQ(ctor_call_count, iteration_count * 10);

	for (uint32_t iter = 0; iter < iteration_count; ++iter) {
		for (uint32_t i = 0; i < 10; ++i) {
			EXPECT_EQ(ptrs[iter]->i_, 0);
		}
	}
}

static bool is_aligned(Aligned* ptr) {
	auto uptr = reinterpret_cast<uintptr_t>(ptr);
	return uptr % alignof(Aligned) == 0;
}

static void test_align() {
	(void)base::MainArenaAllocator::get().push_array<char>(15);

	Aligned* aligned = &base::MainArenaAllocator::get().push<Aligned>();

	EXPECT_TRUE(is_aligned(aligned));

	(void)base::MainArenaAllocator::get().push_array<char>(15);

	aligned = base::MainArenaAllocator::get().push_array<Aligned>(10);

	EXPECT_TRUE(is_aligned(aligned));
}

TEST(arena, align) { test_align(); }
TEST(arena, constructors) { test_ctors(); }

TEST(arena, rewind) {
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

	EXPECT_TRUE(base::MainArenaAllocator::get().push_array<char>(1) == ptr && "Arena didn't rewind");
}
