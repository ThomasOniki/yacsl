#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_utils.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/testing/run.hpp"

// NOLINTBEGIN(bugprone-exception-escape)

namespace {
struct TestAllocator {
  using SizeT = size_t;

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
    dynarray_remove(freed, ret);
    dynarray_insert(allocated, ret);
    HISTORY.printf_to("allocated %zu bytes\n", size);
    POINTER_HIST.push(ret);
    return {
        static_cast<yacsl::u8*>(ret),
        size,
    };
  }

  void deallocate(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> mem) noexcept {
    if (mem.data() != nullptr) {
      EXPECT(!dynarray_has(freed, mem.data()), "double free\n");
      EXPECT(dynarray_has(allocated, mem.data()),
             "Tried to free unallocated pointer.\n");
      dynarray_insert(freed, mem.data());

#ifdef _WIN32
      _aligned_free(mem.data());
#else
      free(mem.data());
#endif
    }
    HISTORY.printf_to("deallocated %zu bytes\n", mem.len());
    POINTER_HIST.push(mem.data());
  }

  yacsl::ranges::Slice<yacsl::u8, SizeT>
  resize_in_place(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> /*mem*/,
                  SizeT /*new_size*/) {
    return {nullptr, 0UZ};
  }

  static constexpr bool dynarray_has(DynamicArray<void*> const& arr,
                                     void*                      ptr) {
    for (size_t i = 0; i < arr.len; ++i) {
      if (arr.ptr[i] == ptr) {
        return true;
      }
    }
    return false;
  }
  static constexpr void dynarray_insert(DynamicArray<void*>& arr,
                                        void*                to_insert) {
    for (size_t i = 0; i < arr.len; ++i) {
      if (arr.ptr[i] == nullptr) {
        arr.ptr[i] = to_insert;
        return;
      }
    }
    arr.push(to_insert);
  }
  static constexpr bool dynarray_remove(DynamicArray<void*>& arr,
                                        void*                to_remove) {
    for (size_t i = 0; i < arr.len; ++i) {
      if (arr.ptr[i] == to_remove) {
        arr.ptr[i] = nullptr;
        return true;
      }
    }
    return false;
  }

  inline static DynamicArray<void*> freed;
  inline static DynamicArray<void*> allocated;

  static constexpr void clear_history() {
    HISTORY.clear();
    POINTER_HIST.clear();
  }

  inline static DynamicArray<void*> POINTER_HIST;
  inline static DynamicCString      HISTORY;

  SMFTracker smf_tracker_ = SMFTracker("allocator");
};

using TestAllocatorSingleton =
    yacsl::memory::SingletonAllocator<TestAllocator, "main", true>;
} // namespace

static void test_allocator() {
  TestAllocator::clear_history();
  TestAllocator allocator;
  // NOLINTNEXTLINE(misc-const-correctness)
  auto mem = allocator.allocate(32, yacsl::memory::Alignment{16});

  EXPECT_STREQ(TestAllocator::HISTORY.ptr, "allocated 32 bytes\n");
  EXPECT(TestAllocator::POINTER_HIST.len == 1 &&
         TestAllocator::POINTER_HIST.ptr[0] == mem.data());
  TestAllocator::clear_history();

  allocator.deallocate(mem);

  EXPECT_STREQ(TestAllocator::HISTORY.ptr, "deallocated 32 bytes\n");
  EXPECT(TestAllocator::POINTER_HIST.len == 1 &&
         TestAllocator::POINTER_HIST.ptr[0] == mem.data());
}

int main() {
  fputs("TODO: proper vector tests\n", stderr);
  run_test("allocator", test_allocator);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}

// NOLINTEND(bugprone-exception-escape)
