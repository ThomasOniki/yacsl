export module base.utilities.isize;

import base.utilities.type_aliases;
import base.type_traits.sign;

export namespace base {
using isize = MakeSigned<usize>;
} // namespace base
