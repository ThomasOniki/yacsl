#include "base/memory/arena.hpp"
#include <gtest/gtest.h>
#include <stdio.h>

static uint32_t		 ctor_call_count = 0;
constexpr uint32_t iteration_count = 10000;

struct Test {
	Test(uint32_t i) : i(i) { ++ctor_call_count; }
	Test() : i(0) { ++ctor_call_count; }
	uint32_t i;
};

struct Aligned {
	alignas(64) char data;
};

void test_ctors() {
	ctor_call_count = 0;

	Test* ptrs[iteration_count];
	for (uint32_t i = 0; i < iteration_count; ++i)
		ptrs[i] = &base::main_arena.push<Test>(i);

	EXPECT_EQ(ctor_call_count, iteration_count);

	for (uint32_t i = 0; i < iteration_count; ++i)
		EXPECT_EQ(ptrs[i]->i, i);

	ctor_call_count = 0;

	for (uint32_t i = 0; i < iteration_count; ++i)
		ptrs[i] = base::main_arena.push_array<Test>(10);

	EXPECT_EQ(ctor_call_count, iteration_count * 10);

	for (uint32_t iter = 0; iter < iteration_count; ++iter)
		for (uint32_t i = 0; i < 10; ++i)
			EXPECT_EQ(ptrs[iter]->i, 0);
}

bool is_aligned(Aligned* p) {
	uintptr_t uptr = reinterpret_cast<uintptr_t>(p);
	return uptr % alignof(Aligned) == 0;
}

void test_align() {
	[[maybe_unused]] char* bad_align = base::main_arena.push_array<char>(15);

	Aligned* aligned = &base::main_arena.push<Aligned>();

	EXPECT_TRUE(is_aligned(aligned));

	bad_align = base::main_arena.push_array<char>(15);

	aligned = base::main_arena.push_array<Aligned>(10);

	EXPECT_TRUE(is_aligned(aligned));
}

TEST(arena, align) { test_align(); }
TEST(arena, constructors) { test_ctors(); }

TEST(arena, rewind) {
	{
		base::Arena::Scope scope(base::main_arena); // test scope with ptr_ = 0
	}

	int& _ = base::main_arena.push<int>(); // call malloc so that the rewind test uses the same memory
																				 // in and outside of the scope

	char* p;
	{
		base::Arena::Scope scope(base::main_arena);

		p = base::main_arena.push_array<char>(10);

		test_align();
		test_ctors();
	}

	EXPECT_TRUE(base::main_arena.push_array<char>(1) == p && "Arena didn't rewind");
}
