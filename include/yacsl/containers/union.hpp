#pragma once

#include "yacsl/containers/stack_array.hpp"
#include "yacsl/math/arithmetic.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/limits.hpp"
#include "yacsl/meta/pack.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename... Ts> class Union {
public:
  template <typename T,
            usize I = meta::first_type_index<meta::RemoveCVRef<T>, Ts...>>
  constexpr Union(T&& value) noexcept : index_(I) {
    memory::construct_at(reinterpret_cast<T*>(bytes_.data()),
                         memory::forward<T>(value));
  }
  ~Union() noexcept { destroy(); }

  constexpr Union(Union const& other) noexcept { assign(other); }
  constexpr Union(Union&& other) noexcept { assign(memory::move(other)); }
  constexpr Union& operator=(this Union& self, Union const& other) noexcept {
    self.destroy();
    self.assign(other);
  }
  constexpr Union& operator=(this Union& self, Union&& other) noexcept {
    self.destroy();
    self.assign(memory::move(other));
  }

  [[nodiscard]] constexpr usize index(this Union const& self) noexcept {
    return self.index_;
  }

private:
  constexpr void assign(this Union& self, Union const& other) noexcept {
    self.index_ = other.index_;
    usize index = 0;
    (..., [&] {
      if (index++ == other.index_) {
        memory::construct_at(reinterpret_cast<Ts*>(self.bytes_.data()),
                             *reinterpret_cast<Ts const*>(other.bytes_.data()));
      }
    }());
  }

  constexpr void assign(this Union& self, Union&& other) noexcept {
    self.index_ = other.index_;
    usize index = 0;
    (..., [&] {
      if (index++ == other.index_) {
        Ts&& to_move = memory::move(
            *reinterpret_cast<Ts*>(memory::move(other).bytes_.data()));
        memory::construct_at(reinterpret_cast<Ts*>(self.bytes_.data()),
                             memory::move(to_move));
        to_move->~Ts();
      }
    }());
    other.index_ = sizeof...(Ts);
  }

  constexpr void destroy(this Union& self) noexcept {
    usize index = 0;
    (..., [&] {
      if (index++ == self.index_) {
        reinterpret_cast<Ts const*>(self.bytes_.data())->~Ts();
      }
    }());
  }

private:
  alignas(math::max(alignof(Ts)...))
      StackArray<u8, math::max(sizeof(Ts)...)> bytes_;
  meta::Conditional<sizeof...(Ts) <= meta::Limits<u8>::MAX, u8, usize> index_;
};

} // namespace yacsl::containers
