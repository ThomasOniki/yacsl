#pragma once

namespace base::type_traits {

template <typename From, typename To>
concept ConvertibleTo = __is_convertible(From, To);

} // namespace base::type_traits
