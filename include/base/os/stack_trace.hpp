#pragma once

#include "base/attributes.hpp"
#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/log/logger.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

struct StackTraceEntry {
  alignas(64) String caller;
  String object;
  void*  base_address = nullptr;
  void*  address      = nullptr;
};

struct StackTrace {

  BASE_NOINLINE static StackTrace current() noexcept;

  // Only call this when you actually need it. Computing the entries is non
  // trivial and can be done at any time as long as this->ptrs_ was gotten at
  // the right time.
  Vector<StackTraceEntry, 16> const&
  compute_entries(this StackTrace& self BASE_LIFETIMEBOUND) noexcept;

  Vector<StackTraceEntry, 16> const&
  compute_entries(this StackTrace&& self BASE_LIFETIMEBOUND) noexcept {
    self.compute_entries();
    return base::move(self).entries_;
  }

  template <typename Self>
  void print(this Self&& self, Logger::Level lvl) noexcept {
    base::forward<Self>(self).compute_entries();
    for (usize i = 0; i < self.entries_.size(); ++i) {
      base::log(lvl, "#{} {} in {} ({}+{#x})", i, self.entries_[i].address,
                self.entries_[i].caller, self.entries_[i].object,
                static_cast<uchar*>(self.entries_[i].address) -
                    static_cast<uchar*>(self.entries_[i].base_address));
    }
  }

  alignas(128) Vector<StackTraceEntry, 16> entries_;
  Vector<void*, 21> addresses_;
};

} // namespace base
