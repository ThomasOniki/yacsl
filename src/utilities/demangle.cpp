#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/formatting/atoi.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

namespace {
class Demangler {
public:
  const String& demangle(StringView mangled_func) noexcept {
    mangled_       = mangled_func.begin();
    has_templates_ = false;
    demangled_.clear();

    if (!skip("_Z")) {
      demangled_ += mangled_;
      return demangled_;
    }
    demangle_string();

    // There is no substitute for the whole function name for some reason.
    substitutes_.pop_back();

    String return_type;

    bool has_templates = has_templates_;

    if (has_templates) {
      String func = base::move(demangled_);
      demangle_type();
      return_type = base::move(demangled_);
      demangled_  = base::move(func);
    }

    demangled_ += '(';
    if (*mangled_ == 'v' && mangled_ + 1 == mangled_func.end()) {
      ++mangled_;
    }
    if (mangled_ != mangled_func.end()) {
      for (;;) {
        demangle_type();
        if (mangled_ == mangled_func.end()) {
          break;
        }
        demangled_ += ", ";
      }
    }
    demangled_ += ')';

    if (has_templates) {
      String func = base::move(demangled_);
      demangled_  = base::move(return_type);
      demangled_ += ' ';
      demangled_ += func;
    }

    return demangled_;
  }

private:
  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_string() noexcept {
    substitute_beginings_.push_back(demangled_.size());
    bool nested = skip('N');

    bool is_first = true;

    for (;;) {
      String module_string;

      struct Submodule {
        alignas(16) usize end;
        bool is_substitution = false;
      };

      Vector<Submodule> submodules;
      for (;;) {
        StringView to_add;
        bool       is_substituted = !skip('W');
        if (is_substituted) {
          if (!is_substitution()) {
            break;
          }
          const char* before_sub = mangled_;
          StringView  sub        = substitution();
          if (sub.data() == demangled_.data() || sub.data()[-1] != '@') {
            mangled_ = before_sub;
            break;
          }
          to_add = sub;
        } else {
          const char* end = num_end(mangled_);
          const auto  len = atoi<usize>(mangled_, end);
          mangled_        = end + len;
          to_add          = StringView(end, mangled_);
        }

        char* bytes = module_string.grow(to_add.size() + 1);

        if (module_string.size() == to_add.size() + 1) {
          bytes[0] = '@';
        } else {
          bytes[0] = '.';
        }

        copy_bytes(bytes + 1, to_add.bytes());
        submodules.emplace_back(module_string.size(), is_substituted);
      }

      bool is_substituted = is_substitution();

      if (is_substituted) {
        if (!is_first) {
          demangled_ += "::";
        }
        demangled_ += substitution();
      } else {
        if (!is_first) {
          demangled_ += "::";
        }

        skip('L');

        const char* end = num_end(mangled_);
        const auto  len = atoi<usize>(mangled_, end);

        StringView           chunk(end, len);
        constexpr StringView anonymous_ns = "_GLOBAL__N_";
        if (chunk.size() >= anonymous_ns.size() &&
            StringView(chunk.data(), anonymous_ns.size()) == anonymous_ns) {
          demangled_ += "(anonymous namespace)";
        } else {
          demangled_ += chunk;
        }
        mangled_ = chunk.end();
      }

      for (Submodule submodule : submodules) {
        if (submodule.is_substitution) {
          continue;
        }
        substitutes_.emplace_back(demangled_.size() + 1, submodule.end + demangled_.size());
      }
      if (!is_substituted) {
        substitutes_.emplace_back(substitute_beginings_[substitute_beginings_.size() - 1],
                                  demangled_.size());
      }
      demangled_ += module_string;

      if (*mangled_ == 'I') {
        demangle_template_args();
        substitutes_.emplace_back(substitute_beginings_[substitute_beginings_.size() - 1],
                                  demangled_.size());
      }

      if (!nested || skip('E')) {
        break;
      }

      is_first = false;
    }
    substitute_beginings_.pop_back();
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_type() noexcept {
    StringView qualifiers = get_mangled_qualifiers();

    if (mangled_[0] == 'D' && mangled_[1] == 'p') {
      demangle_template_parameter_pack(qualifiers);
      return;
    }

    if (*mangled_ == 'A') {
      demangle_array_type(qualifiers);
      return;
    }
    if (skip('T')) {
      usize       index = 0;
      const char* end   = mangled_;
      if (*mangled_ != '_') {
        end   = num_end(mangled_);
        index = atoi<usize>(mangled_, end) + 1;
      }
      if (index < template_args_.size()) {
        demangled_.append(&demangled_[template_args_[index].begin],
                          template_args_[index].end - template_args_[index].begin);
      }
      mangled_ = end + 1;
    } else if ((*mangled_ >= '0' && *mangled_ <= '9') || *mangled_ == 'N' || *mangled_ == 'W' ||
               is_substitution()) {
      demangle_string();
    } else {
      demangle_builtin_type_id();
    }
    demangle_type_qualifiers(qualifiers);
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_template_parameter_pack(StringView qualifiers) noexcept {
    mangled_ += "Dp"_sv.size();
    if (*mangled_ == 'T') {
      demangle_type();
      demangle_type_qualifiers(qualifiers);
    } else {
      demangle_type();
      demangle_type_qualifiers(qualifiers);
      demangled_ += "...";
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_array_type(StringView qualifiers) noexcept {
    ++mangled_;
    const char* end = num_end(mangled_);
    StringView  arr_len(mangled_, end);
    mangled_ = end + 1;
    demangle_type();
    StringView prefix = " [";
    if (qualifiers.size() > 0) {
      demangled_ += " (";
      demangle_type_qualifiers(qualifiers);
      prefix = ") [";
    }
    char* data = demangled_.grow(prefix.size() + arr_len.size() + 1);
    copy_bytes(data, prefix.bytes());
    copy_bytes(data + prefix.size(), arr_len.bytes());
    data[prefix.size() + arr_len.size()] = ']';
  }

  void demangle_type_qualifiers(StringView mangled) noexcept {
    for (const char* it = mangled.end() - 1; it != mangled.begin() - 1; --it) {
      switch (*it) {
      case 'K':
        demangled_ += " const";
        break;
      case 'V':
        demangled_ += " volatile";
        break;
      case 'P':
        demangled_ += '*';
        break;
      case 'R':
        demangled_ += '&';
        break;
      case 'O':
        demangled_ += "&&";
        break;
      default:
        break;
      }
    }
  }

  StringView get_mangled_qualifiers() noexcept {
    const char* begin = mangled_;
    while (*mangled_ == 'K' || *mangled_ == 'V' || *mangled_ == 'P' || *mangled_ == 'R' ||
           *mangled_ == 'O') {
      ++mangled_;
    }
    return {begin, mangled_};
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_template_args() noexcept {
    has_templates_ = true;
    ++mangled_;
    demangled_ += '<';

    if (skip('E')) {
      demangled_ += '>';
      return;
    }

    ++template_nesting_level_;
    for (;;) {
      usize arg_begin = demangled_.size();
      if (skip('J')) {
        if (!skip('E')) {
          for (;;) {
            demangle_template_arg();
            if (skip('E')) {
              break;
            }
            demangled_ += ", ";
          }
        }
      } else {
        demangle_template_arg();
      }
      if (template_nesting_level_ == 1) {
        template_args_.emplace_back(arg_begin, demangled_.size());
      }
      if (skip('E')) {
        break;
      }
      if (arg_begin != demangled_.size()) {
        demangled_ += ", ";
      }
    }
    --template_nesting_level_;
    demangled_ += '>';
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_template_arg() noexcept {
    if (*mangled_ == 'L') {
      demangle_litteral();
    } else if (skip('X')) {
      while (*mangled_ != 'E') {
        demangle_expression();
      }
      ++mangled_;
    } else {
      demangle_type();
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_litteral() noexcept {
    ++mangled_;
    switch (*mangled_) {
    case 'i':
      ++mangled_;
      demangle_until_E();
      break;
    case 'j':
      ++mangled_;
      demangle_until_E();
      demangled_ += 'u';
      break;
    case 'l':
      ++mangled_;
      demangle_until_E();
      demangled_ += 'l';
      break;
    case 'm':
      ++mangled_;
      demangle_until_E();
      demangled_ += "ul";
      break;
    case 'x':
      ++mangled_;
      demangle_until_E();
      demangled_ += "ll";
      break;
    case 'y':
      ++mangled_;
      demangle_until_E();
      demangled_ += "ull";
      break;
    default:
      demangled_ += '(';
      demangle_type();
      demangled_ += ')';
      demangle_until_E();
      break;
    }
  }

  void demangle_until_E() noexcept {
    const char* begin = mangled_;
    while (*mangled_ != 'E') {
      ++mangled_;
    }
    demangled_ += {begin, mangled_};
    ++mangled_;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_expression() noexcept {
    if (skip("tl")) {
      demangle_type_litteral();
    } else if (skip("sP")) {
      demangled_ += "sizeof ...(";
      while (*mangled_ != 'E') {
        demangle_template_arg();
      }
      ++mangled_;
      demangled_ += ')';
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_type_litteral() noexcept {
    demangle_type();
    demangled_ += '{';
    for (;;) {
      if (*mangled_ == 'L') {
        demangle_litteral();
      }
      demangle_expression();
      if (skip('E')) {
        break;
      }
      demangled_ += ", ";
    }
    demangled_ += '}';
  }

  [[nodiscard]] bool is_substitution() const noexcept {
    return mangled_[0] == 'S' &&
           (mangled_[1] == '_' || (mangled_[1] >= '0' && mangled_[1] <= '9') ||
            (mangled_[1] >= 'A' && mangled_[1] <= 'Z'));
  }

  [[nodiscard]] StringView substitution() noexcept {
    skip('S');

    usize index = 0;
    if (!skip('_')) {
      index = 1;
      for (;;) {
        if (*mangled_ <= '9') {
          index += static_cast<usize>(*mangled_) - '0';
        } else {
          index += static_cast<usize>(*mangled_) - 'A' + 10;
        }
        ++mangled_;
        if (skip('_')) {
          break;
        }
        index *= 36;
      }
    }
    if (index >= substitutes_.size()) {
      return {demangled_.data(), static_cast<usize>(0)};
    }
    return {&demangled_[substitutes_[index].begin],
            substitutes_[index].end - substitutes_[index].begin};
  }

  void demangle_builtin_type_id() noexcept {
    switch (*mangled_) {
    case 'v':
      demangled_ += "void";
      break;
    case 'w':
      demangled_ += "wchar_t";
      break;
    case 'b':
      demangled_ += "bool";
      break;
    case 'c':
      demangled_ += "char";
      break;
    case 'a':
      demangled_ += "signed char";
      break;
    case 'h':
      demangled_ += "unsigned char";
      break;
    case 's':
      demangled_ += "short";
      break;
    case 't':
      demangled_ += "unsigned short";
      break;
    case 'i':
      demangled_ += "int";
      break;
    case 'j':
      demangled_ += "unsigned int";
      break;
    case 'l':
      demangled_ += "long";
      break;
    case 'm':
      demangled_ += "unsigned long";
      break;
    case 'x':
      demangled_ += "long long";
      break;
    case 'y':
      demangled_ += "unsigned long long";
      break;
    case 'n':
      demangled_ += "__int128";
      break;
    case 'o':
      demangled_ += "unsigned __int128";
      break;
    case 'f':
      demangled_ += "float";
      break;
    case 'd':
      demangled_ += "double";
      break;
    case 'e':
      demangled_ += "long double";
      break;
    case 'g':
      demangled_ += "__float128 (ABI extension)";
      break;
    case 'z':
      demangled_ += "...";
      break;
    case 'D':
      switch (mangled_[1]) {
      case 'u':
        demangled_ += "char8_t";
        ++mangled_;
        break;
      case 's':
        demangled_ += "char16_t";
        ++mangled_;
        break;
      case 'i':
        demangled_ += "char32_t";
        ++mangled_;
        break;
      case 'h':
        demangled_ += "_Float16";
        ++mangled_;
        break;
      default:
        if (mangled_[3] == '6') {
          demangled_ += "_Float16";
          mangled_ += "DF16_"_sv.size() - 1;
        } else if (mangled_[2] == '3') {
          demangled_ += "_Float32";
          mangled_ += "DF32_"_sv.size() - 1;
        } else if (mangled_[2] == '6') {
          demangled_ += "_Float64";
          mangled_ += "DF64_"_sv.size() - 1;
        } else if (mangled_[2] == '1') {
          demangled_ += "_Float128";
          mangled_ += "DF128_"_sv.size() - 1;
        }
        if (mangled_[4] == 'x') {
          demangled_ += 'x';
          ++mangled_;
        }
      }
      break;
    default:
      break;
    }
    ++mangled_;
  }

  bool skip(char chr) noexcept {
    if (*mangled_ == chr) {
      ++mangled_;
      return true;
    }
    return false;
  }

  bool skip(StringView str) noexcept {
    if (StringView(mangled_, mangled_ + str.size()) == str) {
      mangled_ += str.size();
      return true;
    }
    return false;
  }

private:
  String demangled_;
  struct Range {
    alignas(16) usize begin;
    usize end;
  };
  Vector<Range> substitutes_;
  Vector<usize> substitute_beginings_;
  Vector<Range> template_args_;
  const char*   mangled_ = nullptr;

  usize template_nesting_level_ = 0;

  bool has_templates_ = false;
};
} // namespace

String demangle(StringView mangled) {
  Demangler demangler;
  return demangler.demangle(mangled);
}

} // namespace base
