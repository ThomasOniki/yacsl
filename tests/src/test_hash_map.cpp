#include "test_utils.hpp"
#include "yacsl/containers/hash_map.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <stdio.h>

using yacsl::containers::HashMap;

namespace {
struct Key {
  explicit Key(char const* name = "key") noexcept : tracker(name) {}
  SMFTracker     tracker;
  constexpr bool operator==(this Key const&, Key const&) noexcept = default;
};
} // namespace

template <typename SizeT> struct yacsl::containers::Hash<Key, SizeT> {
  SizeT operator()(Key const& key) noexcept { return key.tracker.name_.len; }
};

static void test_insert() {
  auto map    = HashMap<Key, SMFTracker, 0, TestAllocator>(16);
  auto key1   = Key("key1");
  auto key2   = Key("key2");
  auto value2 = SMFTracker("value2");
  clear_logs();

  map.insert(key1, "value1");
  map.insert(yacsl::memory::move(key2), SMFTracker("value2"));
  EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'key1'\n"
                                     "constructed 'value1'\n"
                                     "constructed 'value2'\n"
                                     "move constructed 'key2'\n"
                                     "move constructed 'value2'\n"
                                     "destroyed 'value2'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, "");
}

static void test_get() {
  auto map = HashMap<Key, SMFTracker, 0, TestAllocator>(16);
  map.insert(Key("key1"), "value1");
  map.insert(Key("key2"), "value2");
  map.insert(Key("longer key"), "value3");
  clear_logs();

  auto opt = map.get_option(Key("key1 (moved)"));
  EXPECT(opt.is_value() && opt.value_unchecked() == SMFTracker("value1"));
  if (opt.is_value()) {
    EXPECT(map.get(Key("key1 (moved)")) == SMFTracker("value1"));
    EXPECT(map.get_unchecked(Key("key1 (moved)")) == SMFTracker("value1"), );
  }
  opt = map.get_option(Key("key2 (moved)"));
  EXPECT(opt.is_value() && opt.value_unchecked() == SMFTracker("value2"));
  opt = map.get_option(Key("longer key (moved)"));
  EXPECT(opt.is_value() && opt.value_unchecked() == SMFTracker("value3"));
}

static void test_iter() {
  auto map    = HashMap<Key, SMFTracker, 0, TestAllocator>(16);
  auto value2 = SMFTracker("value2");
  map.insert(Key("key1"), "value1");
  map.insert(Key("key2"), value2);
  map.insert(Key("longer key"), SMFTracker("value3"));
  clear_logs();

  bool found1 = false;
  bool found2 = false;
  bool found3 = false;

  for (auto&& [key, value] : map) {
    if (key == Key("key1 (moved)")) {
      EXPECT(value == SMFTracker("value1"), "value.name = %s\n",
             value.name_.ptr);
      EXPECT(!found1);
      found1 = true;
    } else if (key == Key("key2 (moved)")) {
      EXPECT(value == SMFTracker("value2 (copy)"), "value.name = %s\n",
             value.name_.ptr);
      EXPECT(!found2);
      found2 = true;
    } else {
      EXPECT(key == Key("longer key (moved)"), "key.name = %s\n",
             key.tracker.name_.ptr);
      EXPECT(value == SMFTracker("value3 (moved)"), "value.name = %s\n",
             value.name_.ptr);
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
  run_test("begin/end", test_iter);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
