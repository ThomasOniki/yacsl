export module base.utilities.is_constant_evaluated;

export namespace base {

constexpr bool is_constant_evaluated() noexcept { return __builtin_is_constant_evaluated(); }

} // namespace base
