export module base.utilities.extended_types;

export namespace base {

#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
using i128                  = __int128_t;
using u128                  = __uint128_t;
constexpr bool int128_found = true;
#else
import base.containers.stack_array;
struct i128 {
  StackArray<char, 16> dummmy;
};
using u128                  = i128;
constexpr bool int128_found = false;
#endif

} // namespace base
