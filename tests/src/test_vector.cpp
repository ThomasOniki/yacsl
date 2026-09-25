#include <string>
#include <unordered_set>
#include <vector>

#include "yacsl/debug/log.hpp"
#include "yacsl/format/formatter.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "test_utils.hpp"

using SMFCall  = std::string;
using SMFCalls = std::vector<std::string>;

template <> class yacsl::format::Formatter<std::string> {
public:
  constexpr char const* parse(this Formatter& self, char const* fmt) noexcept {
    return self.impl_.parse(fmt);
  }
  void format(this Formatter const& self, containers::String& buf,
              std::string const& str) noexcept {
    self.impl_.format(buf, {str.data(), str.size()});
  }

private:
  yacsl::format::Formatter<yacsl::ranges::StringView> impl_;
};

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

  template <typename T>
  yacsl::ranges::Slice<T, SizeT> allocate(SizeT count) noexcept {
    void* ret = ::operator new(count * sizeof(T), std::align_val_t{alignof(T)});
    if (freed.contains(ret)) {
      freed.erase(ret);
    }
    allocated.insert(ret);
    events.push_back(Event{
        .kind       = Event::Kind::Alloc,
        .address    = ret,
        .byte_count = count * sizeof(T),
    });
    return {static_cast<T*>(ret), count};
  }

  template <typename T> void free(yacsl::ranges::Slice<T, SizeT> mem) noexcept {
    if (mem.data()) {
      if (freed.contains(mem.data())) {
        yacsl::debug::log_error("double free");
        return;
      }
      if (!allocated.contains(mem.data())) {
        yacsl::debug::log_error("unallocated free");
        return;
      }

      freed.emplace(mem.data());
      ::operator delete(mem.data(), mem.size() * sizeof(T),
                        std::align_val_t{alignof(T)});
    }
    events.push_back(Event{
        .kind       = Event::Kind::Free,
        .address    = mem.data(),
        .byte_count = mem.size() * sizeof(T),
    });
  }

private:
  inline static std::unordered_set<void*> freed;
  inline static std::unordered_set<void*> allocated;

  SMFTracker smf_tracker_ = SMFTracker("allocator");
};

using TestAllocatorSingleton =
    yacsl::memory::SingletonAllocator<TestAllocator, "main", true>;

} // namespace

static int test_allocator() {
  int ret = 0;
  TestAllocator::events.clear();
  TestAllocator allocator;
  auto          mem = allocator.allocate<uint64_t>(10);

  EXPECT_EQ(TestAllocator::events,
            std::vector<TestAllocator::Event>({{
                .kind       = TestAllocator::Event::Kind::Alloc,
                .address    = mem.data(),
                .byte_count = mem.size() * sizeof(uint64_t),
            }}));
  TestAllocator::events.clear();

  allocator.free<uint64_t>(mem);

  EXPECT_EQ(TestAllocator::events,
            std::vector<TestAllocator::Event>({{
                .kind       = TestAllocator::Event::Kind::Free,
                .address    = mem.data(),
                .byte_count = mem.size() * sizeof(uint64_t),
            }}));

  return ret;
}

int main() {
  int ret = 0;
  EXPECT(test_allocator() == 0);
  yacsl::debug::log_warning("TODO: vector tests");
  yacsl::debug::Logger::get_default_instance().flush();
  return ret;
}

// NOLINTEND(bugprone-exception-escape)
