#pragma once

#include "yacsl/containers/stack_array.hpp"
#include "yacsl/math/arithmetic.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/limits.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename... Ts>
  requires(... && (!meta::ReferenceType<Ts> && meta::MutableType<Ts>))
class Union {
public:
  constexpr Union() noexcept : index_(sizeof...(Ts)) {}

  template <usize I, typename... Args>
  [[nodiscard]] constexpr Union create(Args&&... args) noexcept {
    Union ret;
    ret.index_ = I;
    ret.visit([&](auto& value) {
      memory::construct_at(&value, memory::forward<Args>(args)...);
    });
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

  template <typename F>
  constexpr void visit(this Union& self, F&& func) noexcept {
    usize index = 0;
    (..., [&] {
      if (index++ == self.index_) {
        memory::forward<F>(func)(*reinterpret_cast<Ts*>(self.bytes_.data()));
      }
    }());
  }

private:
  alignas(math::max(alignof(Ts)...))
      StackArray<u8, math::max(sizeof(Ts)...)> bytes_;
  meta::Conditional<sizeof...(Ts) <= meta::Limits<u8>::MAX, u8, usize> index_;
};

} // namespace yacsl::containers
