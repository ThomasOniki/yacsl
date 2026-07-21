export module base.utilities.demangle;

import base.containers.string;
import base.ranges.string_view;

export namespace base {

String demangle(StringView mangled);

} // namespace base
