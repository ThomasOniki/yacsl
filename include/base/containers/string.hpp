#pragma once

#include "base/attributes.hpp"
#include "base/containers/base_classes.hpp"
#include "base/memory/arena.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <Character Char, usize N = 0, AllocatorLike Allocator = MainArenaAllocator,
          Integral SizeT = usize>
class BasicString : AllocatorMember<Allocator>, FixedBufferMember<Char, N, SizeT> {
public:
  constexpr BasicString() noexcept = default;

  constexpr ~BasicString() noexcept { free(); }

  constexpr BasicString(Char const* cstr) noexcept
      : BasicString(BasicStringView<Char, usize>(cstr)) {}

  constexpr BasicString(BasicStringView<Char, SizeT> str) noexcept {
    SizeT const size = str.size();

    if constexpr (N > 0) {
      if (size > fixed_buffer_.size()) {
        begin_         = allocator().template allocate<Char>(size).data();
        allocated_end_ = end_ = begin_ + size;
      } else {
        end_ = begin_ + size;
      }
    } else {
      begin_         = allocator().template allocate<Char>(size).data();
      allocated_end_ = end_ = begin_ + size;
    }

    copy(str.slice(), begin_);
  }

  constexpr BasicString(BasicString const& other) noexcept : BasicString(other.view()) {}

  constexpr BasicString(BasicString&& other) noexcept
      : begin_(other.begin_), end_(other.end_), allocated_end_(other.allocated_end_) {
    other.begin_ = other.end_ = other.fixed_buffer_.begin();
    other.allocated_end_      = other.fixed_buffer_.end();
  }

  constexpr BasicString& operator=(this BasicString& self BASE_LIFETIMEBOUND,
                                   BasicString const&     other) noexcept {
    if (&self == &other) {
      return self;
    }
    SizeT const size = other.size();
    if (self.capacity() < size) {
      self.free();
      self.begin_         = self.allocator().template allocate<Char>(size).data();
      self.allocated_end_ = self.end_ = self.begin_ + size;
    } else {
      self.end_ = self.begin_ + size;
    }
    copy({other.begin_, size}, self.begin_);
    return self;
  }

  constexpr BasicString& operator=(this BasicString& self BASE_LIFETIMEBOUND,
                                   BasicString&&          other) noexcept {
    if constexpr (N > 0) {
      if (other.begin_ == other.fixed_buffer_.begin()) {
        SizeT const size = other.size();
        copy({other.fixed_buffer_.begin(), size}, fixed_buffer_.begin());
        self.end_ = self.begin_ + size;
        return self;
      }
    }
    self.free();
    self.begin_         = other.begin_;
    self.end_           = other.end_;
    self.allocated_end_ = other.allocated_end_;
    other.begin_ = other.end_ = other.fixed_buffer_.begin();
    other.allocated_end_      = other.fixed_buffer_.end();
    return self;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND, Char const* ptr,
                                SizeT len) noexcept {
    if (static_cast<SizeT>(self.allocated_end_ - self.end_) < len) {
      self.definitely_reserve(self.capacity() * 2 + len);
    }
    copy({ptr, len}, self.end_);
    self.end_ += len;
    return self;
  }
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND, Char const* begin,
                                Char const* end) noexcept {
    return self.append(begin, static_cast<SizeT>(end - begin));
  }
  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& append(this BasicString& self       BASE_LIFETIMEBOUND,
                                BasicStringView<Char, SizeT> str) noexcept {
    return self.append(str.data(), str.size());
  }
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                Char const*            cstr) noexcept {
    return self.append(BasicStringView<Char, usize>(cstr));
  }
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                BasicString const&     other) noexcept {
    return self.append(other.begin(), other.size());
  }

  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND, Char chr) noexcept {
    if (self.end_ + 1 > self.allocated_end_) {
      self.definitely_reserve(self.capacity() * 2 + 1);
    }
    *self.end_ = chr;
    ++self.end_;
    return self;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& operator+=(this BasicString& self       BASE_LIFETIMEBOUND,
                                    BasicStringView<Char, SizeT> str) {
    return self.append(str);
  }
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND,
                                    BasicString const&     str) {
    return self.append(str);
  }
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND, Char const* cstr) {
    return self.append(cstr);
  }
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND, Char chr) {
    return self.append(chr);
  }

  template <typename Self>
  constexpr BasicString operator+(this Self&& self, BasicString const& rhs) {
    BasicString lhs = base::forward<Self>(self);
    return lhs += rhs;
  }
  template <typename Self>
  constexpr BasicString operator+(this Self&& self, BasicStringView<Char, SizeT> rhs) {
    BasicString lhs = base::forward<Self>(self);
    return lhs += rhs;
  }
  template <typename Self> constexpr BasicString operator+(this Self&& self, Char const* cstr) {
    BasicString lhs = base::forward<Self>(self);
    return lhs += cstr;
  }
  template <typename Self> constexpr BasicString operator+(this Self&& self, Char chr) {
    BasicString lhs = base::forward<Self>(self);
    return lhs += chr;
  }

  constexpr BasicString& reserve(this BasicString& self BASE_LIFETIMEBOUND,
                                 SizeT                  capacity) noexcept {
    if (capacity > self.capacity()) {
      self.definitely_reserve(capacity);
    }
    return self;
  }
  constexpr BasicString& resize(this BasicString& self BASE_LIFETIMEBOUND, SizeT size) noexcept {
    self.reserve(size);
    self.end_ = self.begin_ + size;
    return self;
  }

  // This doesn't initialize the new memory but adds count to size. It is useful for characters
  // you want to memcpy into. It returns new_end - count.
  constexpr Char* grow(this BasicString& self BASE_LIFETIMEBOUND, SizeT count) noexcept {
    if (static_cast<SizeT>(self.allocated_end_ - self.end_) < count) {
      self.definitely_reserve(self.capacity() * 2 + count);
    }
    Char* ret = self.end_;
    self.end_ += count;
    return ret;
  }

  constexpr BasicString& clear(this BasicString& self BASE_LIFETIMEBOUND) noexcept {
    self.end_ = self.begin_;
    return self;
  }

  template <typename Self>
  [[nodiscard]] constexpr Char* c_str(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (self.end_ == self.allocated_end_) {
      self.definitely_reserve(self.capacity() * 2 + 1);
    }
    *self.end_ = '\0';
    return base::forward<Self>(self).begin_;
  }

  [[nodiscard]] constexpr SizeT size(this BasicString const& self) noexcept {
    return static_cast<SizeT>(self.end_ - self.begin_);
  }
  [[nodiscard]] constexpr SizeT capacity(this BasicString const& self) noexcept {
    return static_cast<SizeT>(self.allocated_end_ - self.begin_);
  }

  [[nodiscard]] constexpr Char& operator[](this BasicString& self BASE_LIFETIMEBOUND,
                                           SizeT                  index) noexcept {
    return self.begin_[index];
  }
  [[nodiscard]] constexpr Char&& operator[](this BasicString&& self BASE_LIFETIMEBOUND,
                                            SizeT                   index) noexcept {
    return base::move(base::move(self).begin_[index]);
  }
  [[nodiscard]] constexpr Char const& operator[](this BasicString const& self BASE_LIFETIMEBOUND,
                                                 SizeT                        index) noexcept {
    return self.begin_[index];
  }
  [[nodiscard]] constexpr Char const&& operator[](this BasicString const&& self BASE_LIFETIMEBOUND,
                                                  SizeT                         index) noexcept {
    return base::move(base::move(self).begin_[index]);
  }

  [[nodiscard]] constexpr bool operator==(this BasicString const& self,
                                          BasicString const&      other) noexcept {
    return self.view() == other.view();
  }
  [[nodiscard]] constexpr bool operator==(this BasicString const&             self,
                                          BasicStringView<Char, SizeT> const& rhs) noexcept {
    return self.view() == rhs;
  }
  [[nodiscard]] constexpr bool operator==(this BasicString const& self, Char const* cstr) noexcept {
    return self.view() == BasicStringView(cstr);
  }

  [[nodiscard]] constexpr BasicStringView<Char, SizeT>
  view(this BasicString const& self BASE_LIFETIMEBOUND) noexcept {
    return {self.begin_, self.end_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> Slice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).begin_, base::forward<Self>(self).end_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> ReversedSlice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).begin_, base::forward<Self>(self).end_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr Char* data(this BasicString& self BASE_LIFETIMEBOUND) noexcept {
    return self.begin_;
  }
  [[nodiscard]] constexpr Char* data(this BasicString&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(self).begin_;
  }
  [[nodiscard]] constexpr Char const*
  data(this BasicString const& self BASE_LIFETIMEBOUND) noexcept {
    return self.begin_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).begin_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).end_;
  }

private:
  using FixedBufferMember<Char, N, SizeT>::fixed_buffer_;
  using AllocatorMember<Allocator>::allocator;

private:
  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr void definitely_reserve(this BasicString& self, SizeT capacity) noexcept {
    Char*       data = self.allocator().template allocate<Char>(capacity).data();
    SizeT const size = self.size();
    copy({self.begin_, size}, data);
    self.free();
    self.begin_         = data;
    self.end_           = data + size;
    self.allocated_end_ = data + capacity;
  }

  void free(this BasicString& self) noexcept {
    if constexpr (N > 0) {
      if (self.begin_ != self.fixed_buffer_.begin()) {
        self.allocator().template free<Char>({self.begin_, self.allocated_end_});
      }
    } else {
      self.allocator().template free<Char>({self.begin_, self.allocated_end_});
    }
  }

private:
  Char* begin_         = fixed_buffer_.begin();
  Char* end_           = fixed_buffer_.begin();
  Char* allocated_end_ = fixed_buffer_.end();
};

using String = BasicString<char>;

} // namespace base
