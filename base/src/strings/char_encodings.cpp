module base.strings.char_encodings;

import base.utilities.type_aliases;

namespace base {

u32 utf8_char_len(uchar utf8) {
  if (utf8 <= 127) {
    return 1;
  }
  if (utf8 >> 5 == 0b110) {
    return 2;
  }
  if (utf8 >> 4 == 0b1110) {
    return 3;
  }
  if (utf8 >> 3 == 0b11110) {
    return 4;
  }
  return 0;
}

} // namespace base
