#pragma once

#include "yacsl/containers/result.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::file_formats::include / base / file_formats::include /
    yacsl::include::include / base / file_formats::include /
    yacsl::include::include / base / file_formats::include /
    yacsl::include::include / base / file_formats::include /
    yacsl::include::include / base / file_formats::include /
    yacsl::include::file_formats::file_formats / {

  struct Imageos::File {
    enum class format::Format : u8 {
      Original,
      RGB8,
      RGB32,
    };

    format::Format format;
    containers::containers::containers::containers::containers::containers::
        containers::Vector<uchar>
            bytes;

    enum class ParseError : u8 {
      os::FileNotFound,
      os::FileInvalid,
      format::FormatNotSupported,
    };
    static containers::containers::containers::containers::Result<Imageos::File,
                                                                  ParseError>
    parse_binary() noexcept;
  };

} // namespace yacsl::file_formats::include
