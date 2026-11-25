#include "test_utils.hpp"
#include "yacsl/containers/hash_set.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <string.h>

using yacsl::containers::HashSet;

namespace {
struct Key {
  explicit Key(char const* name = "key") noexcept : tracker(name), name(name) {}
  SMFTracker  tracker;
  char const* name;

  constexpr bool operator==(this Key const& lhs, Key const& rhs) noexcept {
    return strcmp(lhs.name, rhs.name) == 0;
  }
};
} // namespace

template <typename SizeT> struct yacsl::containers::Hash<Key, SizeT> {
  SizeT operator()(this Hash /*self*/, Key const& key) noexcept {
    return strlen(key.name) ^ key.name[0];
  }
};

static void test_insert() {
  auto map  = HashSet<Key, 0, TestAllocator>(16);
  auto key1 = Key("key1");
  auto key2 = Key("key2");
  clear_logs();

  map.insert(key1);

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'key1'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, "");
  clear_logs();

  auto result1 = map.try_insert(yacsl::memory::move(key2));
  EXPECT(result1.is_value());
  if (result1.is_value()) {
    EXPECT_STREQ(result1.value_unchecked().tracker.name_.ptr, "key2 (moved)");
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "move constructed 'key2'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, "");
  clear_logs();

  auto result2 = map.try_insert(Key("key2"));
  EXPECT(result2.is_error());
  if (result2.is_error()) {
    EXPECT_STREQ(result2.error().tracker.name_.ptr, "key2 (moved)");
    if (result1.is_value()) {
      EXPECT(&result1.value_unchecked() == &result2.error_unchecked());
    }
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'key2'\n"
                                     "destroyed 'key2'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, "");
}

static void test_get() {
  auto map  = HashSet<Key, 0, TestAllocator>(16);
  Key& key1 = map.insert(Key("key1"));
  map.insert(Key("key2"));
  map.insert(Key("longer key"));
  clear_logs();

  auto opt = map.get_option(Key("key1"));
  EXPECT(opt.is_value());
  if (opt.is_value()) {
    EXPECT_STREQ(opt.value_unchecked().tracker.name_.ptr, "key1 (moved)");
    EXPECT(&opt.value_unchecked() == &key1);
    EXPECT_STREQ(map.get(Key("key1")).tracker.name_.ptr, "key1 (moved)");
    EXPECT(map.get_unchecked(Key("key1")) == Key("key1"));
  }
  opt = map.get_option(Key("key2"));
  EXPECT(opt.is_value() && opt.value_unchecked() == Key("key2"));
  opt = map.get_option(Key("longer key"));
  EXPECT(opt.is_value() && opt.value_unchecked() == Key("longer key"));
}

static void test_remove() {
  auto map = HashSet<Key, 0, TestAllocator>(16);
  map.insert(Key("key1"));
  map.insert(Key("key2"));
  map.insert(Key("removed key1"));
  map.insert(Key("removed key2"));
  map.insert(Key("longer key"));
  map.remove(Key("removed key1"));
  map.remove_unchecked(Key("removed key2"));
  clear_logs();

  EXPECT(!map.try_remove(Key("key that never existed")));

  auto opt = map.get_option(Key("key1"));
  EXPECT(opt.is_value() && opt.value_unchecked() == Key("key1"));
  EXPECT(map.try_remove(Key("key1")));
  EXPECT(map.get_option(Key("key1")).is_nil());
  EXPECT(!map.try_remove(Key("key1")));
  EXPECT(map.get_option(Key("key1")).is_nil());

  EXPECT(map.get_option(Key("removed key1")).is_nil());
  EXPECT(map.get_option(Key("removed key2")).is_nil());

  opt = map.get_option(Key("key2"));
  EXPECT(opt.is_value() && opt.value_unchecked() == Key("key2"));
  opt = map.get_option(Key("longer key"));
  EXPECT(opt.is_value() && opt.value_unchecked() == Key("longer key"));
}

static void test_iter() {
  auto map    = HashSet<Key, 0, TestAllocator>(16);
  auto value2 = SMFTracker("value2");
  map.insert(Key("key1"));
  map.insert(Key("key2"));
  map.insert(Key("longer key"));
  clear_logs();

  bool found1 = false;
  bool found2 = false;
  bool found3 = false;

  for (auto&& key : map) {
    if (key == Key("key1")) {
      EXPECT(!found1);
      found1 = true;
    } else if (key == Key("key2")) {
      EXPECT(!found2);
      found2 = true;
    } else {
      EXPECT_STREQ(key.tracker.name_.ptr, "longer key (moved)");
      EXPECT(!found3);
      found3 = true;
    }
  }
  EXPECT(found1);
  EXPECT(found2);
  EXPECT(found3);
}

int main() {
  run_test("insert", test_insert);
  run_test("get", test_get);
  run_test("remove", test_remove);
  run_test("begin/end", test_iter);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
