#pragma once

#include "base/attributes.hpp"
#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/logging/logger.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

struct StackTraceEntry {
  alignas(64) String caller;
  StringView object;
  void*      base_address = nullptr;
  void*      address      = nullptr;
};

struct StackTrace {

  BASE_NOINLINE static StackTrace current() noexcept;

  // Only call this when you actually need it. Computing the entries is non trivial and can be done
  // at any time as long as this->ptrs_ was gotten at the right time.
  const Vector<StackTraceEntry, 16>& compute_entries() noexcept;

  void print(Logger::Level lvl) noexcept {
    compute_entries();
    for (usize i = 0; i < entries_.size(); ++i) {
      base::log(lvl, "#{} {} in {} ({}+{#x})", i, entries_[i].address, entries_[i].caller,
                entries_[i].object,
                static_cast<uchar*>(entries_[i].address) -
                    static_cast<uchar*>(entries_[i].base_address));
    }
  }

  alignas(128) Vector<StackTraceEntry, 16> entries_;
  Vector<void*, 21> addresses_;
};

} // namespace base
