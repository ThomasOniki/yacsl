#pragma once

namespace yacsl::type_traits {

template <typename From, typename To>
concept ConvertibleTo = __is_convertible(From, To);

} // namespace yacsl::type_traits
