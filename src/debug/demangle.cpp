#include "base/debug/demangle.hpp"
#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/format/atoi.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"

namespace base::debug {

namespace {
class Demangler {
public:
  containers::String const& demangle(this Demangler&    self,
                                     ranges::StringView mangled_func) noexcept {
    self.mangled_       = mangled_func.begin();
    self.has_templates_ = false;
    self.demangled_.clear();

    if (!self.skip("_Z")) {
      self.demangled_ += self.mangled_;
      return self.demangled_;
    }
    self.demangle_string();

    // There is no substitute for the whole function name for some reason.
    self.substitutes_.pop_back();

    containers::String return_type;

    bool const has_templates = self.has_templates_;

    if (has_templates) {
      containers::String const func = memory::move(self.demangled_);
      self.demangle_type();
      return_type     = memory::move(self.demangled_);
      self.demangled_ = func;
    }

    self.demangled_ += '(';
    if (*self.mangled_ == 'v' && self.mangled_ + 1 == mangled_func.end()) {
      ++self.mangled_;
    }
    if (self.mangled_ != mangled_func.end()) {
      for (;;) {
        self.demangle_type();
        if (self.mangled_ == mangled_func.end()) {
          break;
        }
        self.demangled_ += ", ";
      }
    }
    self.demangled_ += ')';

    if (has_templates) {
      containers::String const func = memory::move(self.demangled_);
      self.demangled_               = memory::move(return_type);
      self.demangled_ += ' ';
      self.demangled_ += func;
    }

    return self.demangled_;
  }

private:
  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_string(this Demangler& self) noexcept {
    self.substitute_beginings_.push_back(self.demangled_.size());
    bool const is_nested = self.skip('N');

    for (bool is_first = true;; is_first = false) {
      Modules const modules        = self.demangle_modules();
      bool const    is_substituted = self.is_substitution();

      if (!is_first) {
        self.demangled_ += "::";
      }

      if (is_substituted) {
        self.demangled_ += self.substitution();
      } else {
        self.skip('L');

        char const* end = format::num_end(self.mangled_);
        auto const  len = format::atoi<usize>(self.mangled_, end);

        ranges::StringView const     chunk(end, len);
        constexpr ranges::StringView anonymous_ns = "_GLOBAL__N_";
        if (chunk.size() >= anonymous_ns.size() &&
            ranges::StringView(chunk.data(), anonymous_ns.size()) ==
                anonymous_ns) {
          self.demangled_ += "(anonymous namespace)";
        } else {
          self.demangled_ += chunk;
        }
        self.mangled_ = chunk.end();
      }

      for (Modules::Submodule const submodule : modules.submodules) {
        if (submodule.is_substitution) {
          continue;
        }
        self.substitutes_.emplace_back(self.demangled_.size() + 1,
                                       submodule.end + self.demangled_.size());
      }
      if (!is_substituted) {
        self.substitutes_.emplace_back(
            self.substitute_beginings_[self.substitute_beginings_.size() - 1],
            self.demangled_.size());
      }
      self.demangled_ += modules.module_string;

      if (*self.mangled_ == 'I') {
        self.demangle_template_args();
        self.substitutes_.emplace_back(
            self.substitute_beginings_[self.substitute_beginings_.size() - 1],
            self.demangled_.size());
      }

      if (!is_nested || self.skip('E')) {
        break;
      }
    }
    self.substitute_beginings_.pop_back();
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_type(this Demangler& self) noexcept {
    ranges::StringView const qualifiers = self.get_mangled_qualifiers();

    if (self.mangled_[0] == 'D' && self.mangled_[1] == 'p') {
      self.demangle_template_parameter_pack(qualifiers);
      return;
    }

    if (*self.mangled_ == 'A') {
      self.demangle_array_type(qualifiers);
      return;
    }
    if (self.skip('T')) {
      usize       index = 0;
      char const* end   = self.mangled_;
      if (*self.mangled_ != '_') {
        end   = format::num_end(self.mangled_);
        index = format::atoi<usize>(self.mangled_, end) + 1;
      }
      if (index < self.template_args_.size()) {
        self.demangled_.append(
            &self.demangled_[self.template_args_[index].begin],
            self.template_args_[index].end - self.template_args_[index].begin);
      }
      self.mangled_ = end + 1;
    } else if ((*self.mangled_ >= '0' && *self.mangled_ <= '9') ||
               *self.mangled_ == 'N' || *self.mangled_ == 'W' ||
               self.is_substitution()) {
      self.demangle_string();
    } else {
      self.demangle_builtin_type_id();
    }
    self.demangle_type_qualifiers(qualifiers);
  }

  void // NOLINTNEXTLINE(misc-no-recursion)
  demangle_template_parameter_pack(this Demangler&    self,
                                   ranges::StringView qualifiers) noexcept {
    using base::ranges::operator""_sv;
    self.mangled_ += "Dp"_sv.size();
    if (*self.mangled_ == 'T') {
      self.demangle_type();
      self.demangle_type_qualifiers(qualifiers);
    } else {
      self.demangle_type();
      self.demangle_type_qualifiers(qualifiers);
      self.demangled_ += "...";
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_array_type(this Demangler&    self,
                           ranges::StringView qualifiers) noexcept {
    ++self.mangled_;
    char const*              end = format::num_end(self.mangled_);
    ranges::StringView const arr_len(self.mangled_, end);
    self.mangled_ = end + 1;
    self.demangle_type();
    ranges::StringView prefix = " [";
    if (qualifiers.size() > 0) {
      self.demangled_ += " (";
      self.demangle_type_qualifiers(qualifiers);
      prefix = ") [";
    }
    char* data = self.demangled_.grow(prefix.size() + arr_len.size() + 1);
    memory::copy(prefix.slice(), data);
    memory::copy(arr_len.slice(), data + prefix.size());
    data[prefix.size() + arr_len.size()] = ']';
  }

  void demangle_type_qualifiers(this Demangler&    self,
                                ranges::StringView mangled) noexcept {
    for (char const* it = mangled.end() - 1; it != mangled.begin() - 1; --it) {
      switch (*it) {
      case 'K':
        self.demangled_ += " const";
        break;
      case 'V':
        self.demangled_ += " volatile";
        break;
      case 'P':
        self.demangled_ += '*';
        break;
      case 'R':
        self.demangled_ += '&';
        break;
      case 'O':
        self.demangled_ += "&&";
        break;
      default:
        break;
      }
    }
  }

  ranges::StringView get_mangled_qualifiers(this Demangler& self) noexcept {
    char const* begin = self.mangled_;
    while (*self.mangled_ == 'K' || *self.mangled_ == 'V' ||
           *self.mangled_ == 'P' || *self.mangled_ == 'R' ||
           *self.mangled_ == 'O') {
      ++self.mangled_;
    }
    return {begin, self.mangled_};
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_template_args(this Demangler& self) noexcept {
    self.has_templates_ = true;
    ++self.mangled_;
    self.demangled_ += '<';

    if (self.skip('E')) {
      self.demangled_ += '>';
      return;
    }

    ++self.template_nesting_level_;
    for (;;) {
      usize arg_begin = self.demangled_.size();
      if (self.skip('J')) {
        if (!self.skip('E')) {
          for (;;) {
            self.demangle_template_arg();
            if (self.skip('E')) {
              break;
            }
            self.demangled_ += ", ";
          }
        }
      } else {
        self.demangle_template_arg();
      }
      if (self.template_nesting_level_ == 1) {
        self.template_args_.emplace_back(arg_begin, self.demangled_.size());
      }
      if (self.skip('E')) {
        break;
      }
      if (arg_begin != self.demangled_.size()) {
        self.demangled_ += ", ";
      }
    }
    --self.template_nesting_level_;
    self.demangled_ += '>';
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_template_arg(this Demangler& self) noexcept {
    if (*self.mangled_ == 'L') {
      self.demangle_litteral();
    } else if (self.skip('X')) {
      while (*self.mangled_ != 'E') {
        self.demangle_expression();
      }
      ++self.mangled_;
    } else {
      self.demangle_type();
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_litteral(this Demangler& self) noexcept {
    ++self.mangled_;
    switch (*self.mangled_) {
    case 'i':
      ++self.mangled_;
      self.demangle_until_E();
      break;
    case 'j':
      ++self.mangled_;
      self.demangle_until_E();
      self.demangled_ += 'u';
      break;
    case 'l':
      ++self.mangled_;
      self.demangle_until_E();
      self.demangled_ += 'l';
      break;
    case 'm':
      ++self.mangled_;
      self.demangle_until_E();
      self.demangled_ += "ul";
      break;
    case 'x':
      ++self.mangled_;
      self.demangle_until_E();
      self.demangled_ += "ll";
      break;
    case 'y':
      ++self.mangled_;
      self.demangle_until_E();
      self.demangled_ += "ull";
      break;
    default:
      self.demangled_ += '(';
      self.demangle_type();
      self.demangled_ += ')';
      self.demangle_until_E();
      break;
    }
  }

  void demangle_until_E(this Demangler& self) noexcept {
    char const* begin = self.mangled_;
    while (*self.mangled_ != 'E') {
      ++self.mangled_;
    }
    self.demangled_ += ranges::StringView{begin, self.mangled_};
    ++self.mangled_;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_expression(this Demangler& self) noexcept {
    if (self.skip("tl")) {
      self.demangle_type_litteral();
    } else if (self.skip("sP")) {
      self.demangled_ += "sizeof ...(";
      while (*self.mangled_ != 'E') {
        self.demangle_template_arg();
      }
      ++self.mangled_;
      self.demangled_ += ')';
    }
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  void demangle_type_litteral(this Demangler& self) noexcept {
    self.demangle_type();
    self.demangled_ += '{';
    for (;;) {
      if (*self.mangled_ == 'L') {
        self.demangle_litteral();
      }
      self.demangle_expression();
      if (self.skip('E')) {
        break;
      }
      self.demangled_ += ", ";
    }
    self.demangled_ += '}';
  }

  [[nodiscard]] bool is_substitution(this Demangler const& self) noexcept {
    return self.mangled_[0] == 'S' &&
           (self.mangled_[1] == '_' ||
            (self.mangled_[1] >= '0' && self.mangled_[1] <= '9') ||
            (self.mangled_[1] >= 'A' && self.mangled_[1] <= 'Z'));
  }

  [[nodiscard]] ranges::StringView substitution(this Demangler& self) noexcept {
    self.skip('S');

    usize index = 0;
    if (!self.skip('_')) {
      index = 1;
      for (;;) {
        if (*self.mangled_ <= '9') {
          index += static_cast<usize>(*self.mangled_) - '0';
        } else {
          index += static_cast<usize>(*self.mangled_) - 'A' + 10;
        }
        ++self.mangled_;
        if (self.skip('_')) {
          break;
        }
        index *= 36;
      }
    }
    if (index >= self.substitutes_.size()) {
      return {self.demangled_.data(), static_cast<usize>(0)};
    }
    return {&self.demangled_[self.substitutes_[index].begin],
            self.substitutes_[index].end - self.substitutes_[index].begin};
  }

  struct Modules {
    struct Submodule {
      alignas(16) usize end;
      bool is_substitution = false;
    };
    containers::String            module_string;
    containers::Vector<Submodule> submodules;
  };

  // NOLINTNEXTLINE(misc-no-recursion)
  Modules demangle_modules(this Demangler& self) noexcept {
    Modules ret;
    for (;;) {
      ranges::StringView to_add;
      bool const         is_substituted = !self.skip('W');
      if (is_substituted) {
        if (!self.is_substitution()) {
          break;
        }
        char const*              before_sub = self.mangled_;
        ranges::StringView const sub        = self.substitution();
        if (sub.data() == self.demangled_.data() || sub.data()[-1] != '@') {
          self.mangled_ = before_sub;
          break;
        }
        to_add = sub;
      } else {
        char const* end = format::num_end(self.mangled_);
        auto const  len = format::atoi<usize>(self.mangled_, end);
        self.mangled_   = end + len;
        to_add          = ranges::StringView(end, self.mangled_);
      }

      char* bytes = ret.module_string.grow(to_add.size() + 1);

      if (ret.module_string.size() == to_add.size() + 1) {
        bytes[0] = '@';
      } else {
        bytes[0] = '.';
      }

      memory::copy(to_add.slice(), bytes + 1);
      ret.submodules.emplace_back(ret.module_string.size(), is_substituted);
    }
    return ret;
  }

  // NOLINTNEXTLINE(google-readability-function-size)
  void demangle_builtin_type_id(this Demangler& self) noexcept {
    using base::ranges::operator""_sv;
    switch (*self.mangled_) {
    case 'v':
      self.demangled_ += "void";
      break;
    case 'w':
      self.demangled_ += "wchar_t";
      break;
    case 'b':
      self.demangled_ += "bool";
      break;
    case 'c':
      self.demangled_ += "char";
      break;
    case 'a':
      self.demangled_ += "signed char";
      break;
    case 'h':
      self.demangled_ += "unsigned char";
      break;
    case 's':
      self.demangled_ += "short";
      break;
    case 't':
      self.demangled_ += "unsigned short";
      break;
    case 'i':
      self.demangled_ += "int";
      break;
    case 'j':
      self.demangled_ += "unsigned int";
      break;
    case 'l':
      self.demangled_ += "long";
      break;
    case 'm':
      self.demangled_ += "unsigned long";
      break;
    case 'x':
      self.demangled_ += "long long";
      break;
    case 'y':
      self.demangled_ += "unsigned long long";
      break;
    case 'n':
      self.demangled_ += "__int128";
      break;
    case 'o':
      self.demangled_ += "unsigned __int128";
      break;
    case 'f':
      self.demangled_ += "float";
      break;
    case 'd':
      self.demangled_ += "double";
      break;
    case 'e':
      self.demangled_ += "long double";
      break;
    case 'g':
      self.demangled_ += "__float128 (ABI extension)";
      break;
    case 'z':
      self.demangled_ += "...";
      break;
    case 'D':
      switch (self.mangled_[1]) {
      case 'u':
        self.demangled_ += "char8_t";
        ++self.mangled_;
        break;
      case 's':
        self.demangled_ += "char16_t";
        ++self.mangled_;
        break;
      case 'i':
        self.demangled_ += "char32_t";
        ++self.mangled_;
        break;
      case 'h':
        self.demangled_ += "_Float16";
        ++self.mangled_;
        break;
      default:
        if (self.mangled_[3] == '6') {
          self.demangled_ += "_Float16";
          self.mangled_ += "DF16_"_sv.size() - 1;
        } else if (self.mangled_[2] == '3') {
          self.demangled_ += "_Float32";
          self.mangled_ += "DF32_"_sv.size() - 1;
        } else if (self.mangled_[2] == '6') {
          self.demangled_ += "_Float64";
          self.mangled_ += "DF64_"_sv.size() - 1;
        } else if (self.mangled_[2] == '1') {
          self.demangled_ += "_Float128";
          self.mangled_ += "DF128_"_sv.size() - 1;
        }
        if (self.mangled_[4] == 'x') {
          self.demangled_ += 'x';
          ++self.mangled_;
        }
      }
      break;
    default:
      break;
    }
    ++self.mangled_;
  }

  bool skip(this Demangler& self, char chr) noexcept {
    if (*self.mangled_ == chr) {
      ++self.mangled_;
      return true;
    }
    return false;
  }

  bool skip(this Demangler& self, ranges::StringView str) noexcept {
    if (ranges::StringView(self.mangled_, self.mangled_ + str.size()) == str) {
      self.mangled_ += str.size();
      return true;
    }
    return false;
  }

private:
  containers::String demangled_;
  struct Range {
    alignas(16) usize begin;
    usize end;
  };
  containers::Vector<Range> substitutes_;
  containers::Vector<usize> substitute_beginings_;
  containers::Vector<Range> template_args_;
  char const*               mangled_ = nullptr;

  usize template_nesting_level_ = 0;

  bool has_templates_ = false;
};
} // namespace

containers::String demangle(ranges::StringView mangled) {
  Demangler demangler;
  return demangler.demangle(mangled);
}

} // namespace base::debug
