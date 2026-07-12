module;

#include "base/attributes.hpp"

export module base.containers.string;

import base.ranges.string_view;
import base.ranges.slice;
import base.utilities.type_aliases;
import base.containers.stack_array;
import base.containers.base_classes;
import base.type_traits.categories;
import base.type_traits.allocator;
import base.memory.arena;
import base.memory.lifetimes;

export namespace base {

template <Character Char, usize N = 0, AllocatorLike Allocator = MainArenaAllocator,
          Integral SizeT = usize>
class BasicString : AllocatorMember<Allocator>, FixedBufferMember<Char, N, SizeT> {
public:
  constexpr BasicString() noexcept = default;

  constexpr ~BasicString() noexcept { free(); }

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr BasicString(StringView str) noexcept {
    const SizeT size = str.size();

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

    copy_bytes(begin_, str.data(), size);
  }

  constexpr BasicString(const BasicString& other) noexcept : BasicString(other.view()) {}

  constexpr BasicString(BasicString&& other) noexcept
      : begin_(other.begin_), end_(other.end_), allocated_end_(other.allocated_end_) {
    other.begin_ = other.end_ = other.fixed_buffer_.begin();
    other.allocated_end_      = other.fixed_buffer_.end();
  }

  constexpr BasicString& operator=(const BasicString& other) noexcept {
    if (this == &other) {
      return *this;
    }
    const SizeT size = other.size();
    if (capacity() < size) {
      free();
      begin_         = allocator().template allocate<Char>(size);
      allocated_end_ = end_ = begin_ + size;
    } else {
      end_ = begin_ + size;
    }
    copy_bytes(begin_, other.begin_, size);
    return *this;
  }

  constexpr BasicString& operator=(BasicString&& other) noexcept {
    if constexpr (N > 0) {
      if (other.begin_ == other.fixed_buffer_.begin()) {
        const SizeT size = other.size();
        copy_bytes(fixed_buffer_.begin(), other.fixed_buffer_.begin(), size);
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

  constexpr BasicString& append(const Char* ptr, SizeT len) noexcept {
    if (static_cast<SizeT>(allocated_end_ - end_) < len) {
      definitely_reserve(capacity() * 2 + len);
    }
    copy_bytes(end_, ptr, len);
    end_ += len;
    return *this;
  }
  constexpr BasicString& append(const Char* begin, const Char* end) noexcept {
    return append(begin, static_cast<SizeT>(end - begin));
  }

  constexpr BasicString& append(BasicStringView<Char, SizeT> str) noexcept {
    return append(str.data(), str.size());
  }
  constexpr BasicString& append(const BasicString& other) noexcept {
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

  constexpr BasicString& operator+=(BasicStringView<Char, SizeT> str) { return append(str); }
  constexpr BasicString& operator+=(const BasicString& str) { return append(str); }
  constexpr BasicString& operator+=(Char chr) { return append(chr); }

  constexpr BasicString operator+(const BasicString& rhs) {
    BasicString ret(*this);
    return ret += rhs;
  }
  constexpr BasicString operator+(BasicStringView<Char, SizeT> rhs) {
    BasicString ret(*this);
    return ret += rhs;
  }
  constexpr BasicString operator+(Char chr) {
    BasicString ret(*this);
    return ret += chr;
  }

  constexpr void reserve(SizeT capacity) noexcept {
    if (capacity > this->capacity()) {
      definitely_reserve(capacity);
    }
  }

  // This doesn't initialize the new memory but adds count to size. It is useful for characters you
  // want to copy_bytes into. It returns new_end - count.
  constexpr Char* grow(SizeT count) noexcept {
    if (static_cast<SizeT>(allocated_end_ - end_) < count) {
      definitely_reserve(capacity() * 2 + count);
    }
    Char* ret = end_;
    end_ += count;
    return ret;
  }

  constexpr void clear() noexcept { end_ = begin_; }

  [[nodiscard]] constexpr const char* c_str() noexcept {
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
  [[nodiscard]] constexpr const Char& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return begin_[index];
  }

  [[nodiscard]] constexpr BasicStringView<Char, SizeT> view() const noexcept BASE_LIFETIMEBOUND {
    return {begin_, end_};
  }
  [[nodiscard]] constexpr Slice<Char, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {begin_, end_};
  }
  [[nodiscard]] constexpr Slice<const Char, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {begin_, end_};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uchar*>(begin_), static_cast<SizeT>(size() * sizeof(Char))};
  }
  [[nodiscard]] constexpr Slice<const uchar, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uchar*>(begin_), static_cast<SizeT>(size() * sizeof(Char))};
  }

  [[nodiscard]] constexpr Char*       data() noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr const Char* data() const noexcept BASE_LIFETIMEBOUND { return begin_; }

  [[nodiscard]] constexpr Char*       begin() noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr Char*       end() noexcept BASE_LIFETIMEBOUND { return end_; }
  [[nodiscard]] constexpr const Char* begin() const noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr const Char* end() const noexcept BASE_LIFETIMEBOUND { return end_; }
  [[nodiscard]] constexpr const Char* cbegin() const noexcept BASE_LIFETIMEBOUND { return begin_; }
  [[nodiscard]] constexpr const Char* cend() const noexcept BASE_LIFETIMEBOUND { return end_; }

private:
  using FixedBufferMember<Char, N, SizeT>::fixed_buffer_;
  using AllocatorMember<Allocator>::allocator;

private:
  void definitely_reserve(SizeT capacity) noexcept {
    Char*       data = allocator().template allocate<Char>(capacity).data();
    const SizeT size = this->size();
    copy_bytes(data, begin_, size);
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
