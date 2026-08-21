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
        begin_         = allocator().template allocate<Char>(size).begin();
        allocated_end_ = end_ = begin_ + size;
      } else {
        end_ = begin_ + size;
      }
    } else {
      begin_         = allocator().template allocate<Char>(size).begin();
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

  constexpr BasicString& operator=(BasicString const& other) noexcept {
    if (this == &other) {
      return *this;
    }
    SizeT const size = other.size();
    if (capacity() < size) {
      free();
      begin_         = allocator().template allocate<Char>(size).data();
      allocated_end_ = end_ = begin_ + size;
    } else {
      end_ = begin_ + size;
    }
    copy({other.begin_, size}, begin_);
    return *this;
  }

  constexpr BasicString& operator=(BasicString&& other) noexcept {
    if constexpr (N > 0) {
      if (other.begin_ == other.fixed_buffer_.begin()) {
        SizeT const size = other.size();
        copy({other.fixed_buffer_.begin(), size}, fixed_buffer_.begin());
        end_ = begin_ + size;
        return *this;
      }
    }
    free();
    begin_         = other.begin_;
    end_           = other.end_;
    allocated_end_ = other.allocated_end_;
    other.begin_ = other.end_ = other.fixed_buffer_.begin();
    other.allocated_end_      = other.fixed_buffer_.end();
    return *this;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& append(Char const* ptr, SizeT len) noexcept {
    if (static_cast<SizeT>(allocated_end_ - end_) < len) {
      definitely_reserve(capacity() * 2 + len);
    }
    copy({ptr, len}, end_);
    end_ += len;
    return *this;
  }
  constexpr BasicString& append(Char const* begin, Char const* end) noexcept {
    return append(begin, static_cast<SizeT>(end - begin));
  }
  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& append(BasicStringView<Char, SizeT> str) noexcept {
    return append(str.data(), str.size());
  }
  constexpr BasicString& append(Char const* cstr) noexcept {
    return append(BasicStringView<Char, usize>(cstr));
  }
  constexpr BasicString& append(BasicString const& other) noexcept {
    return append(other.begin(), other.size());
  }

  constexpr BasicString& append(Char chr) noexcept {
    if (end_ + 1 > allocated_end_) {
      definitely_reserve(capacity() * 2 + 1);
    }
    *end_ = chr;
    ++end_;
    return *this;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& operator+=(BasicStringView<Char, SizeT> str) { return append(str); }
  constexpr BasicString& operator+=(BasicString const& str) { return append(str); }
  constexpr BasicString& operator+=(Char const* cstr) { return append(cstr); }
  constexpr BasicString& operator+=(Char chr) { return append(chr); }

  constexpr BasicString operator+(BasicString const& rhs) {
    BasicString ret(*this);
    return ret += rhs;
  }
  constexpr BasicString operator+(BasicStringView<Char, SizeT> rhs) {
    BasicString ret(*this);
    return ret += rhs;
  }
  constexpr BasicString operator+(Char const* cstr) {
    BasicString ret(*this);
    return ret += cstr;
  }
  constexpr BasicString operator+(Char chr) {
    BasicString ret(*this);
    return ret += chr;
  }

  constexpr BasicString& reserve(SizeT capacity) noexcept {
    if (capacity > this->capacity()) {
      definitely_reserve(capacity);
    }
    return *this;
  }
  constexpr BasicString& resize(SizeT size) noexcept {
    reserve(size);
    end_ = begin_ + size;
    return *this;
  }

  // This doesn't initialize the new memory but adds count to size. It is useful for characters
  // you want to memcpy into. It returns new_end - count.
  constexpr Char* grow(SizeT count) noexcept {
    if (static_cast<SizeT>(allocated_end_ - end_) < count) {
      definitely_reserve(capacity() * 2 + count);
    }
    Char* ret = end_;
    end_ += count;
    return ret;
  }

  constexpr BasicString& clear() noexcept {
    end_ = begin_;
    return *this;
  }

  [[nodiscard]] constexpr Char* c_str() noexcept {
    if (end_ == allocated_end_) {
      definitely_reserve(capacity() * 2 + 1);
    }
    *end_ = '\0';
    return begin_;
  }

  [[nodiscard]] constexpr SizeT size() const noexcept { return static_cast<SizeT>(end_ - begin_); }
  [[nodiscard]] constexpr SizeT capacity() const noexcept {
    return static_cast<SizeT>(allocated_end_ - begin_);
  }

  [[nodiscard]] constexpr Char& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    return begin_[index];
  }
  [[nodiscard]] constexpr Char const& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return begin_[index];
  }

  [[nodiscard]] constexpr bool operator==(BasicString const& other) const noexcept {
    return view() == other.view();
  }
  [[nodiscard]] constexpr bool operator==(BasicStringView<Char, SizeT> const& rhs) const noexcept {
    return view() == rhs;
  }
  [[nodiscard]] constexpr bool operator==(Char const* cstr) const noexcept {
    return view() == BasicStringView(cstr);
  }

  [[nodiscard]] constexpr BasicStringView<Char, SizeT> view() const noexcept BASE_LIFETIMEBOUND {
    return {begin_, end_};
  }
  [[nodiscard]] constexpr Slice<Char, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {begin_, end_};
  }
  [[nodiscard]] constexpr Slice<Char const, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {begin_, end_};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar*>(begin_), static_cast<SizeT>(size() * sizeof(Char))};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar const*>(begin_), static_cast<SizeT>(size() * sizeof(Char))};
  }

  [[nodiscard]] constexpr Char*       data() noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr Char const* data() const noexcept BASE_LIFETIMEBOUND { return begin_; }

  [[nodiscard]] constexpr Char*       begin() noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr Char*       end() noexcept BASE_LIFETIMEBOUND { return end_; }
  [[nodiscard]] constexpr Char const* begin() const noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr Char const* end() const noexcept BASE_LIFETIMEBOUND { return end_; }
  [[nodiscard]] constexpr Char const* cbegin() const noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr Char const* cend() const noexcept BASE_LIFETIMEBOUND { return end_; }

private:
  using FixedBufferMember<Char, N, SizeT>::fixed_buffer_;
  using AllocatorMember<Allocator>::allocator;

private:
  // NOLINTNEXTLINE(misc-no-recursion)
  void definitely_reserve(SizeT capacity) noexcept {
    Char*       data = allocator().template allocate<Char>(capacity).data();
    SizeT const size = this->size();
    copy({begin_, size}, data);
    free();
    begin_         = data;
    end_           = data + size;
    allocated_end_ = data + capacity;
  }

  void free() noexcept {
    if constexpr (N > 0) {
      if (begin_ != fixed_buffer_.begin()) {
        allocator().template free<Char>({begin_, allocated_end_});
      }
    } else {
      allocator().template free<Char>({begin_, allocated_end_});
    }
  }

private:
  Char* begin_         = fixed_buffer_.begin();
  Char* end_           = fixed_buffer_.begin();
  Char* allocated_end_ = fixed_buffer_.end();
};

using String = BasicString<char>;

} // namespace base
