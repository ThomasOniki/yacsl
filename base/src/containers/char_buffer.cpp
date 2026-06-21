module;

#include <string.h>

module base.containers.char_buffer;
import base.memory.arena;

namespace base {

CharBuffer::CharBuffer(const CharBuffer& other) noexcept {
  const usize size = other.size();
  begin_ = size > fixed_.size() ? MainArenaAllocator::get().allocate<char>(size) : fixed_.data();
  allocated_end_ = end_ = begin_ + size;
  memcpy(begin_, other.begin_, size);
}
CharBuffer::CharBuffer(CharBuffer&& other) noexcept
    : begin_(other.begin_), end_(other.end_), allocated_end_(other.allocated_end_) {
  other.end_ = other.allocated_end_ = other.begin_;
}
CharBuffer& CharBuffer::operator=(const CharBuffer& other) noexcept {
  if (&other == this) {
    return *this;
  }
  const usize size = other.size();
  begin_ = size > fixed_.size() ? MainArenaAllocator::get().allocate<char>(size) : fixed_.data();
  allocated_end_ = end_ = begin_ + size;
  memcpy(begin_, other.begin_, size);
  return *this;
}
CharBuffer& CharBuffer::operator=(CharBuffer&& other) noexcept {
  begin_         = other.begin_;
  end_           = other.end_;
  allocated_end_ = other.allocated_end_;
  other.end_ = other.allocated_end_ = other.begin_;
  return *this;
}

void CharBuffer::definitely_reserve(usize capacity) noexcept {
  char* data = ScratchArenaAllocator::get().allocate<char>(capacity);
  memcpy(data, begin_, size());
  end_           = data + size();
  begin_         = data;
  allocated_end_ = data + capacity;
}

void CharBuffer::reserve(usize capacity) noexcept {
  if (capacity > this->capacity()) {
    definitely_reserve(capacity);
  }
}

char* CharBuffer::grow(usize count) noexcept {
  char* ret = end_;
  end_ += count;
  if (end_ > allocated_end_) {
    end_ -= count;
    definitely_reserve(capacity() * 2 + count);
    ret = end_;
    end_ += count;
  }
  return ret;
}

void CharBuffer::append(StringView str) noexcept {
  if (end_ + str.size() > allocated_end_) {
    definitely_reserve(capacity() * 2 + str.size());
  }
  memcpy(end_, str.data(), str.size());
  end_ += str.size();
}

void CharBuffer::append(char chr) noexcept {
  if (end_ + 1 > allocated_end_) {
    definitely_reserve(capacity() * 2 + 1);
  }
  *end_ = chr;
  ++end_;
}

} // namespace base
