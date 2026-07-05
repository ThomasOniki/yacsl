export module base.utilities.demangle;

import base.containers.string;
import base.ranges.string_view;
import base.formatting.atoi;
import base.utilities.type_aliases;
import base.memory.lifetimes;

export namespace base {

class Demangler {
public:
  const String& demangle(StringView mangled_func) noexcept {
    mangled       = mangled_func.begin();
    has_templates = false;
    demangled.clear();

    if (mangled[0] != '_' || mangled[1] != 'Z') {
      demangled += mangled;
      return demangled;
    }
    mangled += "_Z"_sv.size();
    demangle_string();

    if (has_templates) {
      String func = base::move(demangled);
      demangle_type();
      demangled += ' ';
      demangled += func;
    }

    demangled += '(';
    if (*mangled == 'v' && mangled + 1 == mangled_func.end()) {
      ++mangled;
    }
    if (mangled != mangled_func.end()) {
      for (;;) {
        demangle_type();
        if (mangled == mangled_func.end()) {
          break;
        }
        demangled += ", ";
      }
    }
    demangled += ')';
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
      const auto           len = atoi<usize>(mangled, end);
      StringView           chunk(end, len);
      constexpr StringView anonymous_ns = "_GLOBAL__N_";
      if (chunk.size() >= anonymous_ns.size() &&
          StringView(chunk.data(), anonymous_ns.size()) == anonymous_ns) {
        demangled += "(anonymous namespace)";
      } else {
        demangled += chunk;
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
          demangled += '@';
        } else {
          demangled += '.';
        }
        ++mangled;
      } else if (*mangled == 'E') {
        ++mangled;
        break;
      } else {
        demangled += "::";
      }
    }
  }

  void demangle_type() noexcept {
    const char* suffix = mangled;
    while (*mangled == 'K' || *mangled == 'V' || *mangled == 'P' || *mangled == 'R') {
      ++mangled;
    }
    const char* suffix_end = mangled;

    if ((*mangled >= '0' && *mangled <= '9') || *mangled == 'N') {
      demangle_string();
    } else {
      demangle_builtin_type_id();
    }
    for (const char* it = suffix_end - 1; it != suffix - 1; --it) {
      switch (*it) {
      case 'K':
        demangled += " const";
        break;
      case 'V':
        demangled += " volatile";
        break;
      case 'R':
        demangled += '&';
        break;
      case 'P':
        demangled += '*';
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
    demangled += '<';
    for (;;) {
      if (!demangle_litteral() && !demangle_expression()) {
        demangle_type();
      }
      if (*mangled == 'E') {
        ++mangled;
        break;
      }
      demangled += ", ";
    }
    demangled += '>';
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
      demangled += 'u';
      break;
    case 'l':
      ++mangled;
      demangle_until_E();
      demangled += 'l';
      break;
    case 'm':
      ++mangled;
      demangle_until_E();
      demangled += "ul";
      break;
    case 'x':
      ++mangled;
      demangle_until_E();
      demangled += "ll";
      break;
    case 'y':
      ++mangled;
      demangle_until_E();
      demangled += "ull";
      break;
    default:
      demangled += '(';
      demangle_type();
      demangled += ')';
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
    demangled.append(begin, mangled);
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
    demangled += '{';
    for (;;) {
      demangle_litteral();
      if (*mangled == 'E') {
        ++mangled;
        break;
      }
      demangled += ", ";
    }
    demangled += '}';
  }

  void demangle_builtin_type_id() noexcept {
    switch (*mangled) {
    case 'v':
      demangled += "void";
      break;
    case 'w':
      demangled += "wchar_t";
      break;
    case 'b':
      demangled += "bool";
      break;
    case 'c':
      demangled += "char";
      break;
    case 'a':
      demangled += "signed char";
      break;
    case 'h':
      demangled += "unsigned char";
      break;
    case 's':
      demangled += "short";
      break;
    case 't':
      demangled += "unsigned short";
      break;
    case 'i':
      demangled += "int";
      break;
    case 'j':
      demangled += "unsigned int";
      break;
    case 'l':
      demangled += "long";
      break;
    case 'm':
      demangled += "unsigned long";
      break;
    case 'x':
      demangled += "long long";
      break;
    case 'y':
      demangled += "unsigned long long";
      break;
    case 'n':
      demangled += "__int128";
      break;
    case 'o':
      demangled += "unsigned __int128";
      break;
    case 'f':
      demangled += "float";
      break;
    case 'd':
      demangled += "double";
      break;
    case 'e':
      demangled += "long double";
      break;
    case 'g':
      demangled += "__float128 (ABI extension)";
      break;
    case 'z':
      demangled += "...";
      break;
    case 'D':
      switch (mangled[1]) {
      case 'u':
        demangled += "char8_t";
        ++mangled;
        break;
      case 's':
        demangled += "char32_t";
        ++mangled;
        break;
      case 'i':
        demangled += "char32_t";
        ++mangled;
        break;
      case 'h':
        demangled += "_Float16";
        ++mangled;
        break;
      default:
        if (mangled[3] == '6') {
          demangled += "_Float16";
          mangled += "DF16_"_sv.size() - 1;
        } else if (mangled[2] == '3') {
          demangled += "_Float32";
          mangled += "DF32_"_sv.size() - 1;
        } else if (mangled[2] == '6') {
          demangled += "_Float64";
          mangled += "DF64_"_sv.size() - 1;
        } else if (mangled[2] == '1') {
          demangled += "_Float128";
          mangled += "DF128_"_sv.size() - 1;
        }
        if (mangled[4] == 'x') {
          demangled += 'x';
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
  String      demangled;
  const char* mangled       = nullptr;
  bool        has_templates = false;
};

String demangle(StringView mangled) {
  Demangler demangler;
  return demangler.demangle(mangled);
}

} // namespace base
