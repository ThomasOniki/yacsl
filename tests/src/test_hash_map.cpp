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

int main() {
  run_test("insert", test_insert);
  run_test("get", test_get);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
