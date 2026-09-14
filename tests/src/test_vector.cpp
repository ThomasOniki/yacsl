#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/formatting/formatter.hpp"
#include "base/logging/logger.hpp"
#include "base/memory/singleton_allocator.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "test_utils.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <format>
#include <new>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

// NOLINTBEGIN(bugprone-exception-escape)

template <>
class base::Formatter<std::string> : base::Formatter<base::StringView> {
public:
  constexpr char const* parse(char const* fmt) noexcept {
    return base::Formatter<base::StringView>::parse(fmt);
  }
  void format(base::String& buf, std::string_view const& str) const noexcept {
    base::Formatter<base::StringView>::format(
        buf, base::StringView(str.data(), str.size()));
  }
};

namespace {
class TestAllocator {
public:
  using SizeT = size_t;

  inline static std::vector<std::string> events;
  inline static std::vector<std::string> events_with_ptrs;

  template <typename T> base::Slice<T, SizeT> allocate(SizeT count) noexcept {
    void* ret = ::operator new(count * sizeof(T), std::align_val_t{alignof(T)});
    if (freed.contains(ret)) {
      freed.erase(ret);
    }
    allocated.insert(ret);
    events.push_back(std::format("allocated {}", count));
    events_with_ptrs.push_back(std::format("allocated {} at {}", count, ret));
    return {static_cast<T*>(ret), count};
  }

  template <typename T> void free(base::Slice<T, SizeT> mem) noexcept {
    if (mem.data()) {
      if (freed.contains(mem.data())) {
        base::log_error("double free");
        return;
      }
      if (!allocated.contains(mem.data())) {
        base::log_error("unallocated free");
        return;
      }

      freed.emplace(mem.data());
      events.push_back(std::format("freed {}", mem.size()));
      events_with_ptrs.push_back(std::format("freed {} at {}", mem.size(),
                                             static_cast<void*>(mem.data())));
      ::operator delete(mem.data(), mem.size() * sizeof(T),
                        std::align_val_t{alignof(T)});
    } else {
      events.push_back(std::format("freed {} (nullptr)", mem.size()));
    }
    events_with_ptrs.push_back(std::format("freed {} (nullptr)", mem.size()));
  }

private:
  inline static std::unordered_set<void*> freed;
  inline static std::unordered_set<void*> allocated;
};

using TestAllocatorSingleton =
    base::SingletonAllocator<TestAllocator, "main", true>;

class TestClass {
public:
  explicit TestClass(std::string_view const& name) : name_(name) {

    events.push_back("default constructed " + name_);
  }
  TestClass(TestClass const& other) : name_(other.name_ + " (copied)") {

    events.push_back("copy constructed " + name_);
  }
  TestClass(TestClass&& other) noexcept : name_(other.name_ + " (moved)") {

    events.push_back("move constructed " + name_);
  }
  ~TestClass() noexcept { events.push_back("destructed " + name_); }
  TestClass& operator=(TestClass const& other) {
    if (&other == this) {
      return *this;
    }
    name_ = other.name_ + " (copy assigned)";

    events.push_back("copy assigned " + name_);
    return *this;
  }
  [[maybe_unused]] TestClass& operator=(TestClass&& other) noexcept {
    name_ = other.name_ + " (move assigned)";
    events.push_back("move assigned " + name_);
    return *this;
  }

public:
  inline static std::vector<std::string> events = {};

  std::string name_;
};

using Snapshot = std::vector<std::string>;

class EventTester {
public:
  template <typename Vector> int test() {
    int  ret  = 0;
    auto test = [this, &ret]() {
      return test_events<Vector>([this, &ret](bool strict_ordering) {
        auto const& expected = expected_snapshots_[snapshot_index_];
        ++snapshot_index_;
        if (strict_ordering) {
          EXPECT_EQ(TestClass::events, expected);
        } else {
          std::unordered_multiset<std::string> expected_events(expected.begin(),
                                                               expected.end());
          std::unordered_multiset<std::string> events(TestClass::events.begin(),
                                                      TestClass::events.end());
          EXPECT_EQ(events, expected_events);
        }
        TestClass::events.clear();
      });
    };
    EXPECT(test() == 0);
    return ret;
  }

  static EventTester& get() {
    static EventTester instance;
    return instance;
  }

private:
  EventTester() {
    test_events<std::vector<TestClass>>([this](bool) {
      expected_snapshots_.emplace_back(TestClass::events);
      TestClass::events.clear();
    });
  }

  template <typename Vector> int test_events(auto&& func) {
    int ret = 0;
    TestClass::events.clear();
    snapshot_index_ = 0;
    {

      Vector temp;
      temp.reserve(2);

      temp.emplace_back("a");
      func(true);
      temp.emplace_back("b");
      func(true);

      EXPECT_EQ(temp.size(), 2);
      EXPECT_EQ(temp[0].name_, "a");
      EXPECT_EQ(temp[1].name_, "b");

      Vector moved_to(std::move(temp));
      func(true);

      EXPECT_EQ(moved_to.size(), 2);
      EXPECT_EQ(moved_to[0].name_, "a");
      EXPECT_EQ(moved_to[1].name_, "b");

      moved_to.push_back(TestClass("c"));
      // Move order on realloc is implementation defined, so the exact order
      // can't be checked.
      func(false);

      TestClass const copied("d");
      func(true);
      moved_to.push_back(copied);
      func(true);

      EXPECT_EQ(moved_to.size(), 4);
      EXPECT_EQ(moved_to[0].name_, "a (moved)");
      EXPECT_EQ(moved_to[1].name_, "b (moved)");
      EXPECT_EQ(moved_to[2].name_, "c (moved)");
      EXPECT_EQ(moved_to[3].name_, "d (copied)");
    }
    // Destruction order is also implementation defined.
    func(false);
    return ret;
  }

private:
  std::vector<Snapshot> expected_snapshots_;
  size_t                snapshot_index_ = 0;
};
} // namespace

static int test_allocator() {
  int ret = 0;
  TestAllocator::events.clear();
  TestAllocator allocator;
  auto          mem = allocator.allocate<uint64_t>(10);
  allocator.free<uint64_t>(mem);

  std::vector<std::string> expected = {"allocated 10", "freed 10"};
  EXPECT_EQ(TestAllocator::events, expected);
  return ret;
}

static int test_push_after_copy() {
  int ret = 0;
  TestAllocator::events.clear();
  {
    base::Vector<char, 0, TestAllocator, int32_t> foo = {
        'f',
        'o',
        'o',
        '\0',
    }; // 1 alloc
    EXPECT_STREQ(foo.data(), "foo");

    base::Vector<char, 2, TestAllocator> foo_bar(foo); // 2 allocs

    foo_bar.pop_back(); // pop '\0'

    for (char const chr : " bar") { // 3 allocs
      foo_bar.push_back(chr);
    }

    EXPECT_STREQ(foo_bar.data(), static_cast<char const*>("foo bar"));
  }

  std::vector<std::string> expected = {
      "allocated 4", // construct foo
      "allocated 4", // construct foo_bar
      "allocated 9", // push_back to foo_bar
      "freed 4",     // push_back to foo_bar
      "freed 9",     // destruct foo_bar
      "freed 4",     // destruct foo
  };
  EXPECT_EQ(TestAllocator::events, expected);
  return ret;
}

static int test_default_template_args() {
  return EventTester::get().test<base::Vector<TestClass>>();
}
static int test_custom_allocator() {
  return EventTester::get().test<base::Vector<TestClass, 0, TestAllocator>>();
}

int main() {
  int ret = 0;
  EXPECT(test_allocator() == 0);
  EXPECT(test_push_after_copy() == 0);
  EXPECT(test_default_template_args() == 0);
  EXPECT(test_custom_allocator() == 0);
  base::Logger::get_default_instance().flush();
  return ret;
}

// NOLINTEND(bugprone-exception-escape)
