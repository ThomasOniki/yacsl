#include "test_utils.hpp"
#include "yacsl/containers/hash_map.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <stdio.h>

using yacsl::containers::HashMap;

namespace {
struct Key {
  explicit Key(char const* name = "key") noexcept : tracker(name) {}
  SMFTracker tracker;
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

int main() {
  run_test("insert", test_insert);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
