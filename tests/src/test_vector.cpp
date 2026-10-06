#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "yacsl/containers/option.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/singleton_allocator.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"

using SMFCall  = std::string;
using SMFCalls = std::vector<std::string>;

// NOLINTBEGIN(bugprone-exception-escape)

namespace {
class SMFTracker {
public:
  explicit SMFTracker(std::string_view const& name) : name_(name) {

    calls.push_back("default constructed " + name_);
  }
  SMFTracker(SMFTracker const& other) : name_(other.name_ + " (copied)") {

    calls.push_back("memory::copy constructed " + name_);
  }
  SMFTracker(SMFTracker&& other) noexcept
      : name_(other.name_ + " (memory::moved)") {

    calls.push_back("memory::move constructed " + name_);
  }
  ~SMFTracker() noexcept { calls.push_back("destructed " + name_); }
  SMFTracker& operator=(SMFTracker const& other) {
    if (&other == this) {
      return *this;
    }
    name_ = other.name_ + " (memory::copy assigned)";

    calls.push_back("memory::copy assigned " + name_);
    return *this;
  }
  [[maybe_unused]] SMFTracker& operator=(SMFTracker&& other) noexcept {
    name_ = other.name_ + " (memory::move assigned)";
    calls.push_back("memory::move assigned " + name_);
    return *this;
  }

public:
  inline static SMFCalls calls = {};

  std::string name_;
};

class TestAllocator {
public:
  struct Event {
    enum class Kind : yacsl::u8 {
      Alloc,
      Free,
    };

    [[nodiscard]] constexpr bool operator==(this Event self,
                                            Event other) noexcept = default;

    Kind   kind;
    void*  address;
    size_t byte_count;
  };

  using SizeT = size_t;

  inline static std::vector<Event> events;

  [[nodiscard]] constexpr bool operator==(this TestAllocator const& self,
                                          TestAllocator const&      other) {
    return &self == &other;
  }

  yacsl::ranges::Slice<yacsl::u8, SizeT>
  allocate(this TestAllocator& /*self*/, SizeT size,
           yacsl::memory::Alignment align) noexcept {
#ifdef _WIN32
    void* ret = _aligned_malloc(static_cast<size_t>(align), size);
#else
    void* ret = aligned_alloc(static_cast<size_t>(align), size);
#endif
    if (freed.contains(ret)) {
      freed.erase(ret);
    }
    allocated.insert(ret);
    events.push_back(Event{
        .kind       = Event::Kind::Alloc,
        .address    = ret,
        .byte_count = size,
    });
    return {
        static_cast<yacsl::u8*>(ret),
        size,
    };
  }

  void deallocate(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> mem) noexcept {
    if (mem.data() != nullptr) {
      if (freed.contains(mem.data())) {
        yacsl::debug::log_error("double free");
        return;
      }
      if (!allocated.contains(mem.data())) {
        yacsl::debug::log_error("unallocated free");
        return;
      }

      freed.emplace(mem.data());

#ifdef _WIN32
      _aligned_free(mem.data());
#else
      free(mem.data());
#endif
    }
    events.push_back(Event{
        .kind       = Event::Kind::Free,
        .address    = mem.data(),
        .byte_count = mem.size(),
    });
  }

  yacsl::containers::Option<yacsl::ranges::Slice<yacsl::u8, SizeT>>
  resize_in_place(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> /*mem*/,
                  SizeT /*new_size*/) {
    return yacsl::utils::NIL;
  }

private:
  inline static std::unordered_set<void*> freed;
  inline static std::unordered_set<void*> allocated;

  SMFTracker smf_tracker_ = SMFTracker("allocator");
};

using TestAllocatorSingleton =
    yacsl::memory::SingletonAllocator<TestAllocator, "main", true>;

} // namespace

static void test_allocator(yacsl::testing::Test& test) {
  TestAllocator::events.clear();
  TestAllocator allocator;
  // NOLINTNEXTLINE(misc-const-correctness)
  auto mem = allocator.allocate(10, yacsl::memory::Alignment{16});

  if (TestAllocator::events != std::vector<TestAllocator::Event>({{
                                   .kind    = TestAllocator::Event::Kind::Alloc,
                                   .address = mem.data(),
                                   .byte_count = mem.size(),
                               }})) {
    test.fail_quietly();
    fprintf(stderr, "TestAllocator::events is incorrect.");
  }
  TestAllocator::events.clear();

  allocator.deallocate(mem);

  if (TestAllocator::events != std::vector<TestAllocator::Event>({{
                                   .kind    = TestAllocator::Event::Kind::Alloc,
                                   .address = mem.data(),
                                   .byte_count = mem.size(),
                               }})) {
    test.fail_quietly();
    fprintf(stderr, "TestAllocator::events is incorrect.");
  }
}

int main() {
  int ret = 0;
  fputs("TODO: proper vector tests\n", stderr);
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("allocator", test_allocator),
  };
  for (auto& test : tests) {
    printf("Running test '%s'...\n", test.cstr_name());
    if (test.run_quietly() == yacsl::testing::Test::Result::Failure) {
      fprintf(stderr, "test '%s' failed.\n", test.cstr_name());
      ret = 1;
    }
  }
  return ret;
}

// NOLINTEND(bugprone-exception-escape)
