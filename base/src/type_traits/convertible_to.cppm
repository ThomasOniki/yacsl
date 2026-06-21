export module base.type_traits.convertible_to;

export namespace base {

template <typename From, typename To>
concept ConvertibleTo = __is_convertible(From, To);

} // namespace base
