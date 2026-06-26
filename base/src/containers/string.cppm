export module base.containers.string;

import base.ranges.string_view;
import base.utilities.type_aliases;
import base.containers.stack_array;

export namespace base {

class String {
public:
  String() noexcept  = default;
  ~String() noexcept = default;

  String(const String& other) noexcept;
  String(String&& other) noexcept;
  String& operator=(const String& other) noexcept;
  String& operator=(String&& other) noexcept;

  void append(StringView str) noexcept;
  void append(char chr) noexcept;

  void reserve(usize capacity) noexcept;

  // This doesn't initialize the new memory but adds count to size. It is useful for characters you
  // want to memcpy into. It returns new_end - count.
  char* grow(usize count) noexcept;

  void clear() noexcept { end_ = begin_; }

  [[nodiscard]] usize size() const noexcept { return static_cast<usize>(end_ - begin_); }
  [[nodiscard]] usize capacity() const noexcept {
    return static_cast<usize>(allocated_end_ - begin_);
  }

  [[nodiscard]] char*       data() noexcept { return begin_; }
  [[nodiscard]] const char* data() const noexcept { return begin_; }

  [[nodiscard]] char*       begin() noexcept { return begin_; }
  [[nodiscard]] char*       end() noexcept { return end_; }
  [[nodiscard]] const char* begin() const noexcept { return begin_; }
  [[nodiscard]] const char* end() const noexcept { return end_; }
  [[nodiscard]] const char* cbegin() const noexcept { return begin_; }
  [[nodiscard]] const char* cend() const noexcept { return end_; }

private:
  void definitely_reserve(usize capacity) noexcept;

private:
  StackArray<char, 1> fixed_         = {'\0'};
  char*               begin_         = fixed_.data();
  char*               end_           = fixed_.data();
  char*               allocated_end_ = fixed_.end();
};

} // namespace base
