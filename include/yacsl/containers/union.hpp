#pragma once

#include "yacsl/containers/option.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/math/arithmetic.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/declval.hpp"
#include "yacsl/meta/limits.hpp"
#include "yacsl/meta/pack.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/meta/type_string.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::containers {

template <typename... Ts>
  requires(... && (!meta::ReferenceType<Ts> && meta::MutableType<Ts>)) &&
          (... && !meta::OneOf<Ts, utils::Nil, void>)
class Union {
public:
  constexpr Union(utils::Nil /*nil*/) noexcept {}

  template <usize I, typename... Args>
  constexpr auto& emplace(this Union& self, Args&&... args) noexcept {
    self.visit([&]<typename T>(T const& value) { value.~T(); });
    self.index_ = I;
    self.visit([&](auto& value) {
      memory::construct_at(&value, memory::forward<Args>(args)...);
    });
    return self.get_unchecked<I>();
  }

  template <usize I, typename... Args>
  [[nodiscard]] static constexpr Union create(Args&&... args) noexcept {
    Union ret;
    ret.emplace<I>(memory::forward<Args>(args)...);
    return ret;
  }

  ~Union() noexcept {
    visit([]<typename T>(T const& value) { value.~T(); });
  }

  constexpr Union(Union const& other) noexcept : index_(other.index_) {
    visit([&]<typename T>(T& value) {
      T const& other_value = *reinterpret_cast<T const*>(other.bytes_.data());
      memory::construct_at(&value, other_value);
    });
  }
  constexpr Union(Union&& other) noexcept : index_(other.index_) {
    visit([&]<typename T>(T& value) {
      T&& other_value =
          memory::move(*reinterpret_cast<T*>(other.bytes_.data()));
      memory::construct_at(&value, memory::move(other_value));
      other_value.~T();
    });
    other.index_ = static_cast<IndexMemberType>(meta::Limits<usize>::MAX);
  }

  constexpr Union& operator=(this Union& self, Union const& other) noexcept {
    if (&self == &other) {
      return self;
    }
    self.visit([]<typename T>(T const& value) { value.~T(); });
    self.index_ = other.index_;
    self.visit([&]<typename T>(T& value) {
      T const& other_value = *reinterpret_cast<T const*>(other.bytes_.data());
      memory::construct_at(&value, other_value);
    });
    return self;
  }
  constexpr Union& operator=(this Union& self, Union&& other) noexcept {
    self.visit([]<typename T>(T const& value) { value.~T(); });
    self.index_ = other.index_;
    self.visit([&]<typename T>(T& value) {
      T&& other_value =
          memory::move(*reinterpret_cast<T*>(other.bytes_.data()));
      memory::construct_at(&value, memory::move(other_value));
      other_value.~T();
    });
    other.index_ = static_cast<IndexMemberType>(meta::Limits<usize>::MAX);
    return self;
  }

  [[nodiscard]] constexpr usize index(this Union const& self) noexcept {
    return self.index_;
  }

  template <typename Self, typename F>
  constexpr void visit(this Self&& self, F&& func) noexcept
    requires meta::all_same<void, decltype(func(meta::declval<Ts>()))...>
  {
    usize index = 0;
    (..., [&] {
      if (index++ == self.index_) {
        auto* bytes = memory::forward<Self>(self).bytes_.data();
        using Ptr   = meta::CopyCV<Self, Ts>*;
        Ptr ptr     = reinterpret_cast<Ptr>(bytes);
        using Ref   = meta::CopyCVRef<Self&&, Ts>;
        memory::forward<F>(func)(memory::forward<Ref>(*ptr));
      }
    }());
  }

  template <typename Self, typename F>
  constexpr auto visit_option(this Self&& self, F&& func) noexcept
    requires meta::all_same<decltype(func(meta::declval<Ts>()))...>
  {
    using R                = decltype(func(meta::declval<Ts...[0]>()));
    Option<R> return_value = utils::NIL;
    memory::forward<Self>(self).visit([&]<typename T>(T&& value) {
      return_value.emplace(memory::forward<F>(func)(memory::forward<T>(value)));
    });
    return return_value;
  }

  template <typename Self, typename F>
  constexpr auto visit(this Self&& self, F&& func) noexcept
      -> decltype(func(meta::declval<Ts...[0]>())) {
    return memory::forward<Self>(self)
        .visit_option(memory::forward<F>(func))
        .value();
  }

  template <typename Self, typename F>
  constexpr auto visit_unchecked(this Self&& self, F&& func) noexcept
      -> decltype(func(meta::declval<Ts...[0]>())) {
    return memory::forward<Self>(self)
        .visit_option(memory::forward<F>(func))
        .value_unchecked();
  }

  template <usize I, typename Self>
  constexpr auto&& get(this Self&& self) noexcept {
    debug::assert_eq(self.index_, I, "{} is not expected value {}",
                     meta::TYPE_STRING<Union>, I);
    auto ptr = reinterpret_cast<meta::CopyCV<Self, Ts...[I]>*>(
        memory::forward<Self>(self).bytes_.data());
    return memory::forward<meta::CopyCVRef<Self&&, Ts...[I]>>(*ptr);
  }

  template <usize I, typename Self>
  constexpr auto&& get_unchecked(this Self&& self) noexcept {
    debug::assume_ifdef_ndebug(self.index_ == I);
    return memory::forward<meta::CopyCVRef<Self&&, Ts...[I]>>(
        memory::forward<Self>(self).template get<I>());
  }

template <usize I, typename Self>
constexpr auto get_option(this Self&& self) noexcept
    -> containers::Option<meta::CopyCVRef<Self&&, Ts... [I]>> {
      if (self.index_ != I) [[unlikely]] {
        return utils::NIL;
      }
      auto ptr = reinterpret_cast<meta::CopyCV<Self, Ts...[I]>*>(
          memory::forward<Self>(self).bytes_.data());
      return memory::forward<meta::CopyCVRef<Self&&, Ts...[I]>>(*ptr);
    }

private : alignas(math::max(alignof(Ts)...))
              StackArray<u8, math::max(sizeof(Ts)...)> bytes_;
  using IndexMemberType =
      meta::Conditional<sizeof...(Ts) <= meta::Limits<u8>::MAX, u8, usize>;
  IndexMemberType index_ =
      static_cast<IndexMemberType>(meta::Limits<usize>::MAX);
}; // namespace yacsl::containers

} // namespace yacsl::containers
