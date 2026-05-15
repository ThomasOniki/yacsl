#pragma once

namespace base {

template <typename... Args> void fail_consteval(Args&&... /*args*/) {}

} // namespace base
