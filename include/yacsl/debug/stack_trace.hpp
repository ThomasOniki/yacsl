#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/export.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/fmt/format_string_parser.hpp"
#include "yacsl/fmt/formatter.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::debug {

struct StackTraceEntry {
  alignas(64) containers::String caller;
  containers::String object;
  void*              base_address = nullptr;
  void*              address      = nullptr;
};

class StackTrace {
public:
  YACSL_NOINLINE static StackTrace current() noexcept YACSL_API;

  // Only call this when you actually need it. Computing the entries is non
  // trivial and can be done at any time as long as this->ptrs_ was gotten at
  // the right time.
  containers::Vector<StackTraceEntry, 16> const& compute_entries(
      this StackTrace const& self YACSL_LIFETIMEBOUND) noexcept YACSL_API;

  [[nodiscard]] constexpr ranges::Slice<const StackTraceEntry>
  entries(this StackTrace const& self) noexcept {
    return self.entries_.slice();
  }

  containers::Vector<StackTraceEntry, 16> const&
  compute_entries(this StackTrace&& self YACSL_LIFETIMEBOUND) noexcept {
    self.compute_entries();
    return memory::move(self).entries_;
  }

  void format_to(this StackTrace const& self,
                 containers::String&    buf) noexcept {
    self.compute_entries();
    for (usize i = 1; i < self.entries_.size(); ++i) {
      fmt::format_to(buf, "#{} {} in {} ({}+{#x})\n", i,
                     self.entries_[i].address, self.entries_[i].caller,
                     self.entries_[i].object,
                     static_cast<uchar*>(self.entries_[i].address) -
                         static_cast<uchar*>(self.entries_[i].base_address));
    }
  }
  containers::String format(this StackTrace const& self) noexcept {
    containers::String buf;
    self.format_to(buf);
    return buf;
  }

private:
  alignas(128) mutable containers::Vector<StackTraceEntry, 16> entries_;
  containers::Vector<void*, 21> addresses_;
};

} // namespace yacsl::debug

namespace yacsl::fmt {

template <> class Formatter<debug::StackTrace> {
public:
  constexpr char const* parse(this Formatter /*self*/, char const* fmt) {
    fmt::FormatStringParser parser(fmt);
    return parser.end_parse();
  }

  void format(this Formatter /*self*/, containers::String& buffer,
              debug::StackTrace const& stacktrace) {
    stacktrace.format_to(buffer);
  }
};

} // namespace yacsl::fmt
