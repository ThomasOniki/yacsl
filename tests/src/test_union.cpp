#include "test_utils.hpp"
#include "yacsl/containers/option.hpp"
#include "yacsl/containers/union.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/utils/nil.hpp"

[[maybe_unused]]
static void test_return_types(yacsl::containers::Union<char>&       union1,
                              yacsl::containers::Union<char> const& union2) {
  using yacsl::meta::SameAs, yacsl::containers::Option, yacsl::memory::move;

  static_assert(SameAs<decltype(union1.get<0>()), char&>);
  static_assert(SameAs<decltype(move(union1).get<0>()), char&&>);
  static_assert(SameAs<decltype(union2.get<0>()), char const&>);
  static_assert(SameAs<decltype(move(union2).get<0>()), char const&&>);

  static_assert(SameAs<decltype(union1.get_unchecked<0>()), char&>);
  static_assert(SameAs<decltype(move(union1).get_unchecked<0>()), char&&>);
  static_assert(SameAs<decltype(union2.get_unchecked<0>()), char const&>);
  static_assert(
      SameAs<decltype(move(union2).get_unchecked<0>()), char const&&>);

  static_assert(SameAs<decltype(union1.get_option<0>()), Option<char&>>);
  static_assert(SameAs<decltype(move(union1).get_option<0>()), Option<char&&>>);
  static_assert(SameAs<decltype(union2.get_option<0>()), Option<char const&>>);
  static_assert(
      SameAs<decltype(move(union2).get_option<0>()), Option<char const&&>>);

  static_assert(SameAs<decltype(union1.emplace<0>('a')), char&>);

  int  num     = 10;
  auto lambda1 = [&num](auto const& /*value*/) -> int { return num; };
  auto lambda2 = [&num](auto const& /*value*/) -> int& { return num; };
  auto lambda3 = [&num](auto const& /*value*/) -> int&& {
    return yacsl::memory::move(num);
  };

  static_assert(SameAs<decltype(union2.visit(lambda1)), int>);
  static_assert(SameAs<decltype(union2.visit(lambda2)), int&>);
  static_assert(SameAs<decltype(union2.visit(lambda3)), int&&>);

  static_assert(SameAs<decltype(union2.visit_unchecked(lambda1)), int>);
  static_assert(SameAs<decltype(union2.visit_unchecked(lambda2)), int&>);
  static_assert(SameAs<decltype(union2.visit_unchecked(lambda3)), int&&>);

  static_assert(SameAs<decltype(union2.visit_option(lambda1)), Option<int>>);
  static_assert(SameAs<decltype(union2.visit_option(lambda2)), Option<int&>>);
  static_assert(SameAs<decltype(union2.visit_option(lambda3)), Option<int&&>>);
}

namespace {
struct A {
  explicit constexpr A(int value = 0) : smf_tracker("A"), value(value) {}
  SMFTracker smf_tracker;
  int        value;
};
struct B {
  explicit constexpr B(int value = 0) : smf_tracker("B"), value(value) {}
  SMFTracker smf_tracker;
  int        value;
};
} // namespace

static void test_emplace() {
  SMFTracker::HISTORY.clear();
  {
    yacsl::containers::Union<A, B> test = yacsl::utils::NIL;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");

    B& emplaced = test.emplace<1>(70);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'B'\n");
    EXPECT(emplaced.value == 70);
    SMFTracker::HISTORY.clear();

    test.emplace<0>();
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'B'\n"
                                          "constructed 'A'\n");
    SMFTracker::HISTORY.clear();

    test.emplace<0>();
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'A'\n"
                                          "constructed 'A'\n");
    SMFTracker::HISTORY.clear();
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'A'\n");
}

static void test_get() {
  SMFTracker::HISTORY.clear();
  {
    yacsl::containers::Union<A, B> test = yacsl::utils::NIL;
    test.emplace<0>(42);
    EXPECT(test.get<0>().value == 42);
    test.emplace<0>(5);
    A& ref = test.get<0>();
    EXPECT(ref.value == 5);
    ref.value = 43;
    EXPECT(ref.value == 43);
    EXPECT(test.get<0>().value == 43);
    test.emplace<1>(44);
    EXPECT(test.get<1>().value == 44);
    ref = test.emplace<0>(45);
    EXPECT(test.get_unchecked<0>().value == 45);
    EXPECT(test.get_option<0>().is_value());
    EXPECT(test.get_option<0>().value().value == 45);
    EXPECT(test.get_option<1>().is_nil());
    ref.value = 46;
    EXPECT(test.get<0>().value == 46);
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'A'\n"
                                        "destroyed 'A'\n"
                                        "constructed 'A'\n"
                                        "destroyed 'A'\n"
                                        "constructed 'B'\n"
                                        "destroyed 'B'\n"
                                        "constructed 'A'\n"
                                        "destroyed 'A'\n");
}

static void test_visit() {
  yacsl::containers::Union<A, B> test = yacsl::utils::NIL;
  test.emplace<0>(42);
  int set_by_visit = 0;
  test.visit([&set_by_visit](auto&& value) {
    set_by_visit = value.value;
    value.value  = 43;
  });
  EXPECT(set_by_visit == 42);
  EXPECT(test.get<0>().value == 43);

  EXPECT(test.visit([](auto&& value) { return value.value++; }) == 43);
  EXPECT(test.get<0>().value == 44);

  EXPECT(test.visit_option([](auto&& value) { return value.value++; }) == 44);
  EXPECT(test.get<0>().value == 45);

  EXPECT(test.visit_unchecked([](auto&& value) { return value.value++; }) ==
         45);
  EXPECT(test.get<0>().value == 46);

  test = yacsl::utils::NIL;
  EXPECT(test.visit_option([](auto&& /*value*/) { return 0; }).is_nil(),
         "test.index(): %zu\n", test.index());
}

static void test_copy() {
  SMFTracker::HISTORY.clear();
  {
    yacsl::containers::Union<A, B> test1 = yacsl::utils::NIL;
    yacsl::containers::Union<A, B> test2 = test1;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");
    test1 = test2;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");
    test2.emplace<0>(42);
    test1 = test2;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'A'\n"
                                          "copy constructed 'A'\n");
    SMFTracker::HISTORY.clear();
    yacsl::containers::Union<A, B> test3 = test1;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "copy constructed 'A (copy)'\n");
    SMFTracker::HISTORY.clear();
    EXPECT(test3.get<0>().value == 42);
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'A (copy) (copy)'\n"
                                        "destroyed 'A'\n"
                                        "destroyed 'A (copy)'\n");
}

static void test_move() {
  SMFTracker::HISTORY.clear();
  {
    yacsl::containers::Union<A, B> test1 = yacsl::utils::NIL;
    yacsl::containers::Union<A, B> test2 = yacsl::memory::move(test1);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");
    test1 = yacsl::memory::move(test2);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");
    test2.emplace<0>(42);
    test1 = yacsl::memory::move(test2);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'A'\n"
                                          "move constructed 'A'\n"
                                          "destroyed 'A'\n");
    SMFTracker::HISTORY.clear();
    yacsl::containers::Union<A, B> test3 = yacsl::memory::move(test1);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "move constructed 'A (moved)'\n"
                                          "destroyed 'A (moved)'\n");
    SMFTracker::HISTORY.clear();
    EXPECT(test3.get<0>().value == 42);
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'A (moved) (moved)'\n");
}

int main() {
  run_test("emplace", test_emplace);
  run_test("get", test_get);
  run_test("visit", test_visit);
  run_test("copy", test_copy);
  run_test("move", test_move);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
