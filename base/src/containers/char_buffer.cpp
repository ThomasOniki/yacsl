#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/memory/arena.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

void CharBuffer::definitely_reserve(usize capacity) {
  char* data = ScratchArenaAllocator::get().allocate<char>(capacity);
  memcpy(data, begin_, size());
  end_           = data + size();
  begin_         = data;
  allocated_end_ = data + capacity;
}

void CharBuffer::reserve(usize capacity) {
  if (capacity > this->capacity()) {
    definitely_reserve(capacity);
  }
}

char* CharBuffer::grow(usize count) {
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

void CharBuffer::append(const StringView& str) {
  if (end_ + str.size() > allocated_end_) {
    definitely_reserve(capacity() * 2 + str.size());
  }
  memcpy(end_, str.data(), str.size());
  end_ += str.size();
}

void CharBuffer::append(char chr) {
  if (end_ + 1 > allocated_end_) {
    definitely_reserve(capacity() * 2 + 1);
  }
  *end_ = chr;
  ++end_;
}

} // namespace base
