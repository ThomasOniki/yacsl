#include <stddef.h>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "test_utils.hpp"
#include "yacsl/containers/option.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/format/formatter.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"

using SMFCall  = std::string;
using SMFCalls = std::vector<std::string>;

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

  yacsl::ranges::Slice<yacsl::uchar, SizeT>
  allocate(this TestAllocator& /*self*/, SizeT size,
           yacsl::memory::Alignment align) noexcept {
    void* ret = ::operator new(size, static_cast<std::align_val_t>(align));
    if (freed.contains(ret)) {
      freed.erase(ret);
    }
    allocated.insert(ret);
    events.push_back(Event{
        .kind       = Event::Kind::Alloc,
        .address    = ret,
        .byte_count = size,
    });
    return {static_cast<yacsl::uchar*>(ret), size};
  }

  void deallocate(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::uchar, SizeT> mem) noexcept {
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
      ::operator delete(mem.data(), mem.size());
    }
    events.push_back(Event{
        .kind       = Event::Kind::Free,
        .address    = mem.data(),
        .byte_count = mem.size(),
    });
  }

  yacsl::containers::Option<yacsl::ranges::Slice<yacsl::uchar, SizeT>>
  resize_in_place(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::uchar, SizeT> /*mem*/,
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

static int test_allocator() {
  int ret = 0;
  TestAllocator::events.clear();
  TestAllocator allocator;
  auto          mem = allocator.allocate(10, yacsl::memory::Alignment{16});

  EXPECT_EQ(TestAllocator::events,
            std::vector<TestAllocator::Event>({{
                .kind       = TestAllocator::Event::Kind::Alloc,
                .address    = mem.data(),
                .byte_count = mem.size(),
            }}));
  TestAllocator::events.clear();

  allocator.deallocate(mem);

  EXPECT_EQ(TestAllocator::events, std::vector<TestAllocator::Event>({{
                                       .kind = TestAllocator::Event::Kind::Free,
                                       .address    = mem.data(),
                                       .byte_count = mem.size(),
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
