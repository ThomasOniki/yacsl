#pragma once

namespace base {

template <typename From, typename To>
concept ConvertibleTo = __is_convertible(From, To);

} // namespace base
