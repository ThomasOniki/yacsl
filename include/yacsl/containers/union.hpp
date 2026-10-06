#pragma once

#include "yacsl/containers/option.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/math/arithmetic.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/limits.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/type_string.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::containers {

template <typename... Ts>
  requires(... && (!meta::ReferenceType<Ts> && meta::MutableType<Ts>))
class Union {
public:
  constexpr Union() noexcept : index_(sizeof...(Ts)) {}

  template <usize I, typename... Args>
  constexpr void emplace(this Union& self, Args&&... args) noexcept {
    self.visit([&]<typename T>(T const& value) { value.~T(); });
    self.index_ = I;
    self.visit([&](auto& value) {
      memory::construct_at(&value, memory::forward<Args>(args)...);
    });
  }

  template <usize I, typename... Args>
  [[nodiscard]] static constexpr Union create(Args&&... args) noexcept {
    Union ret;
    ret.emplace<I>(memory::forward<Args>(args)...);
    return ret;
  }

  ~Union() noexcept {
    visit([]<typename T>(T const& value) { value->~T(); });
  }

  constexpr Union(Union const& other) noexcept { assign(other); }
  constexpr Union(Union&& other) noexcept { assign(memory::move(other)); }

  constexpr Union& operator=(this Union& self, Union const& other) noexcept {
    self.visit([&]<typename T>(T& value) {
      value.~T();
      T const& other_value = *reinterpret_cast<T const*>(other.bytes_.data());
      memory::construct_at(&value, other_value);
    });
  }
  constexpr Union& operator=(this Union& self, Union&& other) noexcept {
    self.visit([&other]<typename T>(T& value) {
      value->~T();
      value.~T();
      T&& other_value =
          memory::move(*reinterpret_cast<T*>(other.bytes_.data()));
      memory::construct_at(&value, memory::move(other_value));
      other_value.~T();
    });
    other.index_ = sizeof...(Ts);
  }

  [[nodiscard]] constexpr usize index(this Union const& self) noexcept {
    return self.index_;
  }

  template <typename Self, typename F>
  constexpr void visit(this Self&& self, F&& func) noexcept {
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

template <usize I, typename Self>
constexpr auto get(this Self&& self) noexcept
    -> meta::CopyCVRef<Self&&, Ts... [I]> {
      debug::assert_eq(self.index_, I, "Union<{}> is not expected value",
                       String(meta::TYPE_STRING<Ts>) + ", " ..., I);
      auto ptr = reinterpret_cast<meta::CopyCV<Self, Ts...[I]>*>(
          memory::forward<Self>(self).bytes_.data());
      return memory::forward<meta::CopyCVRef<Self&&, Ts...[I]>>(*ptr);
    }

template <usize I, typename Self>
constexpr auto get_unchecked(this Self&& self) noexcept
    -> meta::CopyCVRef<Self&&, Ts... [I]> {
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
  meta::Conditional<sizeof...(Ts) <= meta::Limits<u8>::MAX, u8, usize> index_;
};

} // namespace yacsl::containers
