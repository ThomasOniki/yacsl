export module base.utilities.demangle;

import base.containers.char_buffer;
import base.ranges.string_view;
import base.strings.atoi;
import base.utilities.type_aliases;
import base.memory.lifetimes;

export namespace base {

class Demangler {
public:
  const CharBuffer& demangle(StringView mangled_func) noexcept {
    mangled       = mangled_func.begin();
    has_templates = false;
    demangled.clear();

    if (mangled[0] != '_' || mangled[1] != 'Z') {
      demangled.append(mangled);
      return demangled;
    }
    mangled += "_Z"_sv.size();
    demangle_string();

    if (has_templates) {
      CharBuffer func = base::move(demangled);
      demangle_type();
      demangled.append(' ');
      demangled.append(StringView(func.begin(), func.end()));
    }

    demangled.append('(');
    if (*mangled == 'v' && mangled + 1 == mangled_func.end()) {
      ++mangled;
    }
    if (mangled != mangled_func.end()) {
      for (;;) {
        demangle_type();
        if (mangled == mangled_func.end()) {
          break;
        }
        demangled.append(", ");
      }
    }
    demangled.append(')');
    return demangled;
  }

private:
  void demangle_string() noexcept {
    bool nested = false;
    if (*mangled == 'N') {
      nested = true;
      ++mangled;
    }

    bool mod_started = false;
    if (*mangled == 'L') {
      ++mangled;
    }

    for (;;) {
      const char*          end = num_end(mangled);
      const auto           len = atoi<usize>(StringView(mangled, end));
      StringView           chunk(end, len);
      constexpr StringView anonymous_ns = "_GLOBAL__N_";
      if (chunk.size() >= anonymous_ns.size() &&
          StringView(chunk.data(), anonymous_ns.size()) == anonymous_ns) {
        demangled.append("(anonymous namespace)");
      } else {
        demangled.append(chunk);
      }
      mangled = chunk.end();

      demangle_template_list();

      if (!nested) {
        break;
      }

      if (*mangled == 'L') {
        ++mangled;
      }
      if (*mangled == 'W') {
        if (!mod_started) {
          mod_started = true;
          demangled.append('@');
        } else {
          demangled.append('.');
        }
        ++mangled;
      } else if (*mangled == 'E') {
        ++mangled;
        break;
      } else {
        demangled.append("::");
      }
    }
  }

  void demangle_type() noexcept {
    const char* suffix = mangled;
    while (*mangled == 'K' || *mangled == 'V' || *mangled == 'P' || *mangled == 'R') {
      ++mangled;
    }
    const char* suffix_end = mangled;

    if (*mangled >= '0' && *mangled <= '9' || *mangled == 'N') {
      demangle_string();
    } else {
      demangle_builtin_type_id();
    }
    for (const char* it = suffix_end - 1; it != suffix - 1; --it) {
      switch (*it) {
      case 'K':
        demangled.append(" const");
        break;
      case 'V':
        demangled.append(" volatile");
        break;
      case 'R':
        demangled.append('&');
        break;
      case 'P':
        demangled.append('*');
        break;
      default:
        break;
      }
    }
  }

  void demangle_template_list() noexcept {
    if (*mangled != 'I') {
      return;
    }
    ++mangled;
    has_templates = true;
    demangled.append('<');
    for (;;) {
      if (!demangle_litteral() && !demangle_expression()) {
        demangle_type();
      }
      if (*mangled == 'E') {
        ++mangled;
        break;
      }
      demangled.append(", ");
    }
    demangled.append('>');
  }

  bool demangle_litteral() noexcept {
    if (*mangled != 'L') {
      return false;
    }
    ++mangled;
    switch (*mangled) {
    case 'i':
      ++mangled;
      demangle_until_E();
      break;
    case 'j':
      ++mangled;
      demangle_until_E();
      demangled.append('u');
      break;
    case 'l':
      ++mangled;
      demangle_until_E();
      demangled.append('l');
      break;
    case 'm':
      ++mangled;
      demangle_until_E();
      demangled.append("ul");
      break;
    case 'x':
      ++mangled;
      demangle_until_E();
      demangled.append("ll");
      break;
    case 'y':
      ++mangled;
      demangle_until_E();
      demangled.append("ull");
      break;
    default:
      demangled.append('(');
      demangle_type();
      demangled.append(')');
      demangle_until_E();
      break;
    }
    return true;
  }

  void demangle_until_E() noexcept {
    const char* begin = mangled;
    while (*mangled != 'E') {
      ++mangled;
    }
    demangled.append(StringView(begin, mangled));
    ++mangled;
  }

  bool demangle_expression() noexcept {
    if (*mangled != 'X') {
      return false;
    }
    ++mangled;
    while (*mangled != 'E') {
      demangle_type_litteral();
    }
    ++mangled;
    return true;
  }

  void demangle_type_litteral() noexcept {
    if (mangled[0] != 't' || mangled[1] != 'l') {
      return;
    }
    mangled += 2;
    demangle_type();
    demangled.append('{');
    for (;;) {
      demangle_litteral();
      if (*mangled == 'E') {
        ++mangled;
        break;
      }
      demangled.append(", ");
    }
    demangled.append('}');
  }

  void demangle_builtin_type_id() noexcept {
    switch (*mangled) {
    case 'v':
      demangled.append("void");
      break;
    case 'w':
      demangled.append("wchar_t");
      break;
    case 'b':
      demangled.append("bool");
      break;
    case 'c':
      demangled.append("char");
      break;
    case 'a':
      demangled.append("signed char");
      break;
    case 'h':
      demangled.append("unsigned char");
      break;
    case 's':
      demangled.append("short");
      break;
    case 't':
      demangled.append("unsigned short");
      break;
    case 'i':
      demangled.append("int");
      break;
    case 'j':
      demangled.append("unsigned int");
      break;
    case 'l':
      demangled.append("long");
      break;
    case 'm':
      demangled.append("unsigned long");
      break;
    case 'x':
      demangled.append("long long");
      break;
    case 'y':
      demangled.append("unsigned long long");
      break;
    case 'n':
      demangled.append("__int128");
      break;
    case 'o':
      demangled.append("unsigned __int128");
      break;
    case 'f':
      demangled.append("float");
      break;
    case 'd':
      demangled.append("double");
      break;
    case 'e':
      demangled.append("long double");
      break;
    case 'g':
      demangled.append("__float128 (ABI extension)");
      break;
    case 'z':
      demangled.append("...");
      break;
    case 'D':
      switch (mangled[1]) {
      case 'u':
        demangled.append("char8_t");
        ++mangled;
        break;
      case 's':
        demangled.append("char32_t");
        ++mangled;
        break;
      case 'i':
        demangled.append("char32_t");
        ++mangled;
        break;
      case 'h':
        demangled.append("_Float16");
        ++mangled;
        break;
      default:
        if (mangled[3] == '6') {
          demangled.append("_Float16");
          mangled += "DF16_"_sv.size() - 1;
        } else if (mangled[2] == '3') {
          demangled.append("_Float32");
          mangled += "DF32_"_sv.size() - 1;
        } else if (mangled[2] == '6') {
          demangled.append("_Float64");
          mangled += "DF64_"_sv.size() - 1;
        } else if (mangled[2] == '1') {
          demangled.append("_Float128");
          mangled += "DF128_"_sv.size() - 1;
        }
        if (mangled[4] == 'x') {
          demangled.append('x');
          ++mangled;
        }
      }
      break;
    default:
      break;
    }
    ++mangled;
  }

private:
  CharBuffer  demangled;
  const char* mangled       = nullptr;
  bool        has_templates = false;
};

CharBuffer demangle(StringView mangled) {
  Demangler demangler;
  return demangler.demangle(mangled);
}

} // namespace base
