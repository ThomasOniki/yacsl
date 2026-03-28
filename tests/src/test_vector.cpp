#include "base/containers/vector.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unordered_set>

#include <functional>
#include <initializer_list>
#include <tuple>
#include <type_traits>

void test() {
	static_assert(std::is_trivially_destructible_v<int>);
	auto i = std::move(0);
	std::terminate();
	std::set_terminate(nullptr);
	std::function<int()>	 f;
	std::tuple<int, float> t;
}

class TestAllocator {
public:
	using SizeT = size_t;

	inline static std::vector<std::string> events;
	inline static std::vector<std::string> events_with_ptrs;

	template <typename T> static T* alloc(SizeT count) {
		void* ret = ::operator new(count * sizeof(T), std::align_val_t{alignof(T)});
		if (freed.contains(ret))
			freed.erase(ret);
		allocated.insert(ret);
		events.push_back(std::format("allocated {}", count));
		events_with_ptrs.push_back(std::format("allocated {} at {}", count, ret));
		return static_cast<T*>(ret);
	}

	template <typename T> static void free(T* p, SizeT count) {
		if (p) {
			if (freed.contains(p))
				ASSERT_FALSE("double free");
			if (!allocated.contains(p))
				ASSERT_FALSE("unallocated free");

			freed.emplace(p);

			events.push_back(std::format("freed {}", count));
			events_with_ptrs.push_back(std::format("freed {} at {}", count, static_cast<void*>(p)));
			::operator delete(p, count * sizeof(T));
		} else
			events.push_back(std::format("freed {} (nullptr)", count));
		events_with_ptrs.push_back(std::format("freed {} (nullptr)", count));
	}

private:
	inline static std::unordered_set<void*> freed;
	inline static std::unordered_set<void*> allocated;
};

class TestClass {
public:
	TestClass(const std::string& name) : name_(name) { events.push_back("default constructed " + name_); }
	TestClass(const TestClass& other) : name_(other.name_ + " (copied)") { events.push_back("copy constructed " + name_); }
	TestClass(TestClass&& other) noexcept : name_(other.name_ + " (moved)") { events.push_back("move constructed " + name_); }
	~TestClass() noexcept { events.push_back("destructed " + name_); }
	void operator=(const TestClass& other) {
		name_ = other.name_ + " (copy assigned)";
		events.push_back("copy assigned " + name_);
	}
	void operator=(TestClass&& other) {
		name_ = other.name_ + " (move assigned)";
		events.push_back("move assigned " + name_);
	}

public:
	inline static std::vector<std::string> events = {};

	std::string name_;
};

TEST(vector, allocator) {
	TestAllocator::events.clear();
	uint64_t* p = TestAllocator::alloc<uint64_t>(10);
	TestAllocator::free<uint64_t>(p, 10);

	std::vector<std::string> expected = {"allocated 10", "freed 10"};
	EXPECT_EQ(TestAllocator::events, expected);
}

TEST(vector, push_after_copy) {
	TestAllocator::events.clear();
	{
		base::SmallVector<char, 3, TestAllocator, int32_t> foo = {'f', 'o', 'o', '\0'}; // 1 alloc
		EXPECT_STREQ(foo.data(), "foo");

		base::SmallVector<char, 2, TestAllocator> foo_bar = foo; // 2 allocs

		foo_bar.pop_back(); // pop '\0'

		for (char c : " bar") // 3 allocs
			foo_bar.push_back(c);

		EXPECT_STREQ(foo_bar.data(), "foo bar");
	}

	std::vector<std::string> expected = {
			"allocated 4", // construct foo
			"allocated 4", // construct foo_bar
			"allocated 8", // push_back to foo_bar
			"freed 4",		 // push_back to foo_bar
			"freed 8",		 // destruct foo_bar
			"freed 4",		 // destruct foo
	};
	EXPECT_EQ(TestAllocator::events, expected);
}

using Snapshot = std::vector<std::string>;

class EventTester {
public:
	template <typename Vector0, typename Vector1 = Vector0, typename Vector2 = Vector1, typename Vector3 = Vector2,
						typename Vector4 = Vector3>
	void test() {
		test_events<Vector0, Vector1, Vector2, Vector3, Vector4>([this](bool strict_ordering = true) {
			const auto& expected = expected_snapshots_[snapshot_index_];
			++snapshot_index_;
			if (strict_ordering)
				EXPECT_EQ(TestClass::events, expected);
			else {
				std::unordered_multiset<std::string> expected_events(expected.begin(), expected.end());
				std::unordered_multiset<std::string> events(TestClass::events.begin(), TestClass::events.end());
				EXPECT_EQ(events, expected_events);
			}
			TestClass::events.clear();
		});
	}

	static EventTester& get() {
		static EventTester instance;
		return instance;
	}

private:
	EventTester() {
		test_events<std::vector<TestClass>>([this](bool = true) {
			expected_snapshots_.emplace_back(TestClass::events);
			TestClass::events.clear();
		});
	}

	template <typename Vector0, typename Vector1 = Vector0, typename Vector2 = Vector1, typename Vector3 = Vector2,
						typename Vector4 = Vector3, typename F>
	void test_events(F&& func) {
		TestClass::events.clear();
		snapshot_index_ = 0;
		{

			Vector0 temp;
			temp.reserve(2);

			temp.emplace_back("a");
			func();
			temp.emplace_back("b");
			func();

			EXPECT_EQ(temp.size(), 2);
			EXPECT_EQ(temp[0].name_, "a");
			EXPECT_EQ(temp[1].name_, "b");

			Vector1 moved_to(std::move(temp));
			func();

			EXPECT_EQ(moved_to.size(), 2);
			EXPECT_EQ(moved_to[0].name_, "a");
			EXPECT_EQ(moved_to[1].name_, "b");

			moved_to.push_back(TestClass("c"));
			func(false); // Move order on realloc is implementation defined, so the exact order can't be
									 // checked.

			TestClass copied("d");
			func();
			moved_to.push_back(copied);
			func();

			EXPECT_EQ(moved_to.size(), 4);
			EXPECT_EQ(moved_to[0].name_, "a (moved)");
			EXPECT_EQ(moved_to[1].name_, "b (moved)");
			EXPECT_EQ(moved_to[2].name_, "c (moved)");
			EXPECT_EQ(moved_to[3].name_, "d (copied)");
		}
		func();
	};

private:
	std::vector<Snapshot> expected_snapshots_ = {};
	size_t								snapshot_index_			= 0;
};

TEST(vector, events_default_tp) { EventTester::get().test<base::Vector<TestClass>>(); }
TEST(vector, events_custom_alloc) { EventTester::get().test<base::Vector<TestClass, TestAllocator>>(); }

TEST(vector, events_different_vectors) {
	EventTester::get()
			.test<base::Vector<TestClass, base::ScratchArenaAllocator, uint32_t>,
						base::SmallVector<TestClass, 1, base::ScratchArenaAllocator, int16_t>>();
}

TEST(vector, events_different_allocators) {
	auto expect = [](const std::vector<std::string>& expected) {
		EXPECT_EQ(TestClass::events, expected);
		TestClass::events.clear();
	};

	TestClass::events.clear();
	{
		using namespace std::string_literals;
		base::SmallVector<TestClass, 1, base::ScratchArenaAllocator> arena_alloc_vector;
		arena_alloc_vector.reserve(2);
		arena_alloc_vector.emplace_back("a");
		arena_alloc_vector.emplace_back("b");

		expect({"default constructed a", "default constructed b"});

		EXPECT_EQ(arena_alloc_vector.size(), 2);
		EXPECT_EQ(arena_alloc_vector[0].name_, "a");
		EXPECT_EQ(arena_alloc_vector[1].name_, "b");

		base::Vector<TestClass, TestAllocator> test_alloc_vector(std::move(arena_alloc_vector));

		EXPECT_EQ(test_alloc_vector.size(), 2);
		EXPECT_EQ(test_alloc_vector[0].name_, "a (moved)");
		EXPECT_EQ(test_alloc_vector[1].name_, "b (moved)");

		expect({"move constructed a (moved)", "move constructed b (moved)"});

		base::SmallVector<TestClass, 6> to_copy_assign;
		to_copy_assign.push_back(TestClass("e"));
		to_copy_assign.emplace_back("f");

		expect({
				"default constructed e",
				"move constructed e (moved)",
				"destructed e",
				"default constructed f",
		});

		test_alloc_vector = to_copy_assign;

		expect({
				"destructed b (moved)",
				"destructed a (moved)",
				"copy constructed e (moved) (copied)",
				"copy constructed f (copied)",
		});

		base::SmallVector<TestClass> to_move_assign;
		to_move_assign.reserve(2);
		to_move_assign.emplace_back("g");
		to_move_assign.emplace_back("h");

		expect({
				"default constructed g",
				"default constructed h",
		});

		test_alloc_vector = std::move(to_move_assign);

		expect({
				"destructed f (copied)",
				"destructed e (moved) (copied)",
				"move constructed g (moved)",
				"move constructed h (moved)",
		});
	}
	std::vector<std::string> expected = {
			"destructed h",					"destructed g",					"destructed f", "destructed e (moved)",
			"destructed h (moved)", "destructed g (moved)", "destructed b", "destructed a",
	};
	EXPECT_EQ(TestClass::events, expected);
	TestClass::events.clear();
}
