#ifndef _WIN32

#include "unix-utils.hpp"
#include "base/containers/result.hpp"
#include "base/logging/panic.hpp"
#include "base/os/common.hpp"
#include "base/os/file.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace base {

int unix_flags(FileOpenFlags flags) noexcept {
  unsigned int ret = 0;

  if (strip_file_open_flag(&flags, FileOpenFlags::Trunc)) {
    ret |= O_TRUNC;
  }
  if (strip_file_open_flag(&flags, FileOpenFlags::Create)) {
    ret |= O_CREAT;
  }
  if (!strip_file_open_flag(&flags, FileOpenFlags::DontCloseOnExec)) {
    ret |= O_CLOEXEC;
  }
  if (flags == FileOpenFlags::Read) {
    ret |= O_RDONLY;
  } else if (flags == FileOpenFlags::Write) {
    ret |= O_WRONLY;
  } else {
    ret |= O_RDWR;
  }
  return static_cast<int>(ret);
}

Result<OpenedFile, File::OpenError> open_file(const char* path, FileOpenFlags flags) noexcept {
  const bool seek_end = strip_file_open_flag(&flags, FileOpenFlags::SeekEnd) &&
                        (flags & FileOpenFlags::Trunc) == FileOpenFlags::None;
  const int  handle   = ::open(path, unix_flags(flags), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (handle == -1) [[unlikely]] {
    switch (errno) {
    case ENOENT:
      return File::OpenError::FileNotFound;
    default:
      panic("::open() failed: {}", errno_str());
    }
  }
  usize cursor = 0;
  if (seek_end) {
    const isize result = ::lseek(handle, 0, SEEK_END);
    panic_if(result == -1, "::lseek() failed: {}", errno_str());
    cursor = static_cast<usize>(result);
  }
  return OpenedFile{.handle = handle, .cursor_pos = cursor};
}

// NOLINTNEXTLINE(google-readability-function-size, readability-function-cognitive-complexity)
StringView errno_str(int err) noexcept {
  // clang-format off
  if (err == 0) { return "Success"; }	
#ifdef EPERM
  if (err == EPERM) { return "EPERM Operation not permitted"; }
#endif
#ifdef ENOENT
  if (err == ENOENT) { return "ENOENT No such file or directory"; }
#endif
#ifdef ESRCH
  if (err == ESRCH) { return "ESRCH No such process"; }
#endif
#ifdef EINTR
  if (err == EINTR) { return "EINTR Interrupted system call"; }
#endif
#ifdef EIO
  if (err == EIO) { return "EIO Input/output error"; }
#endif
#ifdef ENXIO
  if (err == ENXIO) { return "ENXIO No such device or address"; }
#endif
#ifdef E2BIG
  if (err == E2BIG) { return "E2BIG Argument list too long"; }
#endif
#ifdef ENOEXEC
  if (err == ENOEXEC) { return "ENOEXEC Exec format error"; }
#endif
#ifdef EBADF
  if (err == EBADF) { return "EBADF Bad file descriptor"; }
#endif
#ifdef ECHILD
  if (err == ECHILD) { return "ECHILD No child processes"; }
#endif
#ifdef EAGAIN
  if (err == EAGAIN) { return "EAGAIN Resource temporarily unavailable"; }
#endif
#ifdef ENOMEM
  if (err == ENOMEM) { return "ENOMEM Cannot allocate memory"; }
#endif
#ifdef EACCES
  if (err == EACCES) { return "EACCES Permission denied"; }
#endif
#ifdef EFAULT
  if (err == EFAULT) { return "EFAULT Bad address"; }
#endif
#ifdef ENOTBLK
  if (err == ENOTBLK) { return "ENOTBLK Block device required"; }
#endif
#ifdef EBUSY
  if (err == EBUSY) { return "EBUSY Device or resource busy"; }
#endif
#ifdef EEXIST
  if (err == EEXIST) { return "EEXIST File exists"; }
#endif
#ifdef EXDEV
  if (err == EXDEV) { return "EXDEV Invalid cross-device link"; }
#endif
#ifdef ENODEV
  if (err == ENODEV) { return "ENODEV No such device"; }
#endif
#ifdef ENOTDIR
  if (err == ENOTDIR) { return "ENOTDIR Not a directory"; }
#endif
#ifdef EISDIR
  if (err == EISDIR) { return "EISDIR Is a directory"; }
#endif
#ifdef EINVAL
  if (err == EINVAL) { return "EINVAL Invalid argument"; }
#endif
#ifdef ENFILE
  if (err == ENFILE) { return "ENFILE Too many open files in system"; }
#endif
#ifdef EMFILE
  if (err == EMFILE) { return "EMFILE Too many open files"; }
#endif
#ifdef ENOTTY
  if (err == ENOTTY) { return "ENOTTY Inappropriate ioctl for device"; }
#endif
#ifdef ETXTBSY
  if (err == ETXTBSY) { return "ETXTBSY Text file busy"; }
#endif
#ifdef EFBIG
  if (err == EFBIG) { return "EFBIG File too large"; }
#endif
#ifdef ENOSPC
  if (err == ENOSPC) { return "ENOSPC No space left on device"; }
#endif
#ifdef ESPIPE
  if (err == ESPIPE) { return "ESPIPE Illegal seek"; }
#endif
#ifdef EROFS
  if (err == EROFS) { return "EROFS Read-only file system"; }
#endif
#ifdef EMLINK
  if (err == EMLINK) { return "EMLINK Too many links"; }
#endif
#ifdef EPIPE
  if (err == EPIPE) { return "EPIPE Broken pipe"; }
#endif
#ifdef EDOM
  if (err == EDOM) { return "EDOM Numerical argument out of domain"; }
#endif
#ifdef ERANGE
  if (err == ERANGE) { return "ERANGE Numerical result out of range"; }
#endif
#ifdef EDEADLK
  if (err == EDEADLK) { return "EDEADLK Resource deadlock avoided"; }
#endif
#ifdef ENAMETOOLONG
  if (err == ENAMETOOLONG) { return "ENAMETOOLONG File name too long"; }
#endif
#ifdef ENOLCK
  if (err == ENOLCK) { return "ENOLCK No locks available"; }
#endif
#ifdef ENOSYS
  if (err == ENOSYS) { return "ENOSYS Function not implemented"; }
#endif
#ifdef ENOTEMPTY
  if (err == ENOTEMPTY) { return "ENOTEMPTY Directory not empty"; }
#endif
#ifdef ELOOP
  if (err == ELOOP) { return "ELOOP Too many levels of symbolic links"; }
#endif
#ifdef ENOMSG
  if (err == ENOMSG) { return "ENOMSG No message of desired type"; }
#endif
#ifdef EIDRM
  if (err == EIDRM) { return "EIDRM Identifier removed"; }
#endif
#ifdef ECHRNG
  if (err == ECHRNG) { return "ECHRNG Channel number out of range"; }
#endif
#ifdef EL2NSYNC
  if (err == EL2NSYNC) { return "EL2NSYNC Level 2 not synchronized"; }
#endif
#ifdef EL3HLT
  if (err == EL3HLT) { return "EL3HLT Level 3 halted"; }
#endif
#ifdef EL3RST
  if (err == EL3RST) { return "EL3RST Level 3 reset"; }
#endif
#ifdef ELNRNG
  if (err == ELNRNG) { return "ELNRNG Link number out of range"; }
#endif
#ifdef EUNATCH
  if (err == EUNATCH) { return "EUNATCH Protocol driver not attached"; }
#endif
#ifdef ENOCSI
  if (err == ENOCSI) { return "ENOCSI No CSI structure available"; }
#endif
#ifdef EL2HLT
  if (err == EL2HLT) { return "EL2HLT Level 2 halted"; }
#endif
#ifdef EBADE
  if (err == EBADE) { return "EBADE Invalid exchange"; }
#endif
#ifdef EBADR
  if (err == EBADR) { return "EBADR Invalid request descriptor"; }
#endif
#ifdef EXFULL
  if (err == EXFULL) { return "EXFULL Exchange full"; }
#endif
#ifdef ENOANO
  if (err == ENOANO) { return "ENOANO No anode"; }
#endif
#ifdef EBADRQC
  if (err == EBADRQC) { return "EBADRQC Invalid request code"; }
#endif
#ifdef EBADSLT
  if (err == EBADSLT) { return "EBADSLT Invalid slot"; }
#endif
#ifdef EBFONT
  if (err == EBFONT) { return "EBFONT Bad font file format"; }
#endif
#ifdef ENOSTR
  if (err == ENOSTR) { return "ENOSTR Device not a stream"; }
#endif
#ifdef ENODATA
  if (err == ENODATA) { return "ENODATA No data available"; }
#endif
#ifdef ETIME
  if (err == ETIME) { return "ETIME Timer expired"; }
#endif
#ifdef ENOSR
  if (err == ENOSR) { return "ENOSR Out of streams resources"; }
#endif
#ifdef ENONET
  if (err == ENONET) { return "ENONET Machine is not on the network"; }
#endif
#ifdef ENOPKG
  if (err == ENOPKG) { return "ENOPKG Package not installed"; }
#endif
#ifdef EREMOTE
  if (err == EREMOTE) { return "EREMOTE Object is remote"; }
#endif
#ifdef ENOLINK
  if (err == ENOLINK) { return "ENOLINK Link has been severed"; }
#endif
#ifdef EADV
  if (err == EADV) { return "EADV Advertise error"; }
#endif
#ifdef ESRMNT
  if (err == ESRMNT) { return "ESRMNT Srmount error"; }
#endif
#ifdef ECOMM
  if (err == ECOMM) { return "ECOMM Communication error on send"; }
#endif
#ifdef EPROTO
  if (err == EPROTO) { return "EPROTO Protocol error"; }
#endif
#ifdef EMULTIHOP
  if (err == EMULTIHOP) { return "EMULTIHOP Multihop attempted"; }
#endif
#ifdef EDOTDOT
  if (err == EDOTDOT) { return "EDOTDOT RFS specific error"; }
#endif
#ifdef EBADMSG
  if (err == EBADMSG) { return "EBADMSG Bad message"; }
#endif
#ifdef EOVERFLOW
  if (err == EOVERFLOW) { return "EOVERFLOW Value too large for defined data type"; }
#endif
#ifdef ENOTUNIQ
  if (err == ENOTUNIQ) { return "ENOTUNIQ Name not unique on network"; }
#endif
#ifdef EBADFD
  if (err == EBADFD) { return "EBADFD File descriptor in bad state"; }
#endif
#ifdef EREMCHG
  if (err == EREMCHG) { return "EREMCHG Remote address changed"; }
#endif
#ifdef ELIBACC
  if (err == ELIBACC) { return "ELIBACC Can not access a needed shared library"; }
#endif
#ifdef ELIBBAD
  if (err == ELIBBAD) { return "ELIBBAD Accessing a corrupted shared library"; }
#endif
#ifdef ELIBSCN
  if (err == ELIBSCN) { return "ELIBSCN .lib section in a.out corrupted"; }
#endif
#ifdef ELIBMAX
  if (err == ELIBMAX) { return "ELIBMAX Attempting to link in too many shared libraries"; }
#endif
#ifdef ELIBEXEC
  if (err == ELIBEXEC) { return "ELIBEXEC Cannot exec a shared library directly"; }
#endif
#ifdef EILSEQ
  if (err == EILSEQ) { return "EILSEQ Invalid or incomplete multibyte or wide character"; }
#endif
#ifdef ERESTART
  if (err == ERESTART) { return "ERESTART Interrupted system call should be restarted"; }
#endif
#ifdef ESTRPIPE
  if (err == ESTRPIPE) { return "ESTRPIPE Streams pipe error"; }
#endif
#ifdef EUSERS
  if (err == EUSERS) { return "EUSERS Too many users"; }
#endif
#ifdef ENOTSOCK
  if (err == ENOTSOCK) { return "ENOTSOCK Socket operation on non-socket"; }
#endif
#ifdef EDESTADDRREQ
  if (err == EDESTADDRREQ) { return "EDESTADDRREQ Destination address required"; }
#endif
#ifdef EMSGSIZE
  if (err == EMSGSIZE) { return "EMSGSIZE Message too long"; }
#endif
#ifdef EPROTOTYPE
  if (err == EPROTOTYPE) { return "EPROTOTYPE Protocol wrong type for socket"; }
#endif
#ifdef ENOPROTOOPT
  if (err == ENOPROTOOPT) { return "ENOPROTOOPT Protocol not available"; }
#endif
#ifdef EPROTONOSUPPORT
  if (err == EPROTONOSUPPORT) { return "EPROTONOSUPPORT Protocol not supported"; }
#endif
#ifdef ESOCKTNOSUPPORT
  if (err == ESOCKTNOSUPPORT) { return "ESOCKTNOSUPPORT Socket type not supported"; }
#endif
#ifdef EOPNOTSUPP
  if (err == EOPNOTSUPP) { return "EOPNOTSUPP Operation not supported"; }
#endif
#ifdef EPFNOSUPPORT
  if (err == EPFNOSUPPORT) { return "EPFNOSUPPORT Protocol family not supported"; }
#endif
#ifdef EAFNOSUPPORT
  if (err == EAFNOSUPPORT) { return "EAFNOSUPPORT Address family not supported by protocol"; }
#endif
#ifdef EADDRINUSE
  if (err == EADDRINUSE) { return "EADDRINUSE Address already in use"; }
#endif
#ifdef EADDRNOTAVAIL
  if (err == EADDRNOTAVAIL) { return "EADDRNOTAVAIL Cannot assign requested address"; }
#endif
#ifdef ENETDOWN
  if (err == ENETDOWN) { return "ENETDOWN Network is down"; }
#endif
#ifdef ENETUNREACH
  if (err == ENETUNREACH) { return "ENETUNREACH Network is unreachable"; }
#endif
#ifdef ENETRESET
  if (err == ENETRESET) { return "ENETRESET Network dropped connection on reset"; }
#endif
#ifdef ECONNABORTED
  if (err == ECONNABORTED) { return "ECONNABORTED Software caused connection abort"; }
#endif
#ifdef ECONNRESET
  if (err == ECONNRESET) { return "ECONNRESET Connection reset by peer"; }
#endif
#ifdef ENOBUFS
  if (err == ENOBUFS) { return "ENOBUFS No buffer space available"; }
#endif
#ifdef EISCONN
  if (err == EISCONN) { return "EISCONN Transport endpoint is already connected"; }
#endif
#ifdef ENOTCONN
  if (err == ENOTCONN) { return "ENOTCONN Transport endpoint is not connected"; }
#endif
#ifdef ESHUTDOWN
  if (err == ESHUTDOWN) { return "ESHUTDOWN Cannot send after transport endpoint shutdown"; }
#endif
#ifdef ETOOMANYREFS
  if (err == ETOOMANYREFS) { return "ETOOMANYREFS Too many references: cannot splice"; }
#endif
#ifdef ETIMEDOUT
  if (err == ETIMEDOUT) { return "ETIMEDOUT Connection timed out"; }
#endif
#ifdef ECONNREFUSED
  if (err == ECONNREFUSED) { return "ECONNREFUSED Connection refused"; }
#endif
#ifdef EHOSTDOWN
  if (err == EHOSTDOWN) { return "EHOSTDOWN Host is down"; }
#endif
#ifdef EHOSTUNREACH
  if (err == EHOSTUNREACH) { return "EHOSTUNREACH No route to host"; }
#endif
#ifdef EALREADY
  if (err == EALREADY) { return "EALREADY Operation already in progress"; }
#endif
#ifdef EINPROGRESS
  if (err == EINPROGRESS) { return "EINPROGRESS Operation now in progress"; }
#endif
#ifdef ESTALE
  if (err == ESTALE) { return "ESTALE Stale file handle"; }
#endif
#ifdef EUCLEAN
  if (err == EUCLEAN) { return "EUCLEAN Structure needs cleaning"; }
#endif
#ifdef ENOTNAM
  if (err == ENOTNAM) { return "ENOTNAM Not a Xenix named type file"; }
#endif
#ifdef ENAVAIL
  if (err == ENAVAIL) { return "ENAVAIL No Xenix semaphores available"; }
#endif
#ifdef EISNAM
  if (err == EISNAM) { return "EISNAM Is a named type file"; }
#endif
#ifdef EREMOTEIO
  if (err == EREMOTEIO) { return "EREMOTEIO Remote I/O error"; }
#endif
#ifdef EDQUOT
  if (err == EDQUOT) { return "EDQUOT Disk quota exceeded"; }
#endif
#ifdef ENOMEDIUM
  if (err == ENOMEDIUM) { return "ENOMEDIUM No medium found"; }
#endif
#ifdef EMEDIUMTYPE
  if (err == EMEDIUMTYPE) { return "EMEDIUMTYPE Wrong medium type"; }
#endif
#ifdef ECANCELED
  if (err == ECANCELED) { return "ECANCELED Operation canceled"; }
#endif
#ifdef ENOKEY
  if (err == ENOKEY) { return "ENOKEY Required key not available"; }
#endif
#ifdef EKEYEXPIRED
  if (err == EKEYEXPIRED) { return "EKEYEXPIRED Key has expired"; }
#endif
#ifdef EKEYREVOKED
  if (err == EKEYREVOKED) { return "EKEYREVOKED Key has been revoked"; }
#endif
#ifdef EKEYREJECTED
  if (err == EKEYREJECTED) { return "EKEYREJECTED Key was rejected by service"; }
#endif
#ifdef EOWNERDEAD
  if (err == EOWNERDEAD) { return "EOWNERDEAD Owner died"; }
#endif
#ifdef ENOTRECOVERABLE
  if (err == ENOTRECOVERABLE) { return "ENOTRECOVERABLE State not recoverable"; }
#endif
#ifdef ERFKILL
  if (err == ERFKILL) { return "ERFKILL Operation not possible due to RF-kill"; }
#endif
#ifdef EHWPOISON
  if (err == EHWPOISON) { return "EHWPOISON Memory page has hardware error"; }
#endif
#ifdef ENOTSUP
  if (err == ENOTSUP) { return "ENOTSUP Not supported parameter or option"; }
#endif
#ifdef ENOMEDIUM
  if (err == ENOMEDIUM) { return "ENOMEDIUM Missing media"; }
#endif
#ifdef EILSEQ
  if (err == EILSEQ) { return "EILSEQ Invalid multibyte sequence"; }
#endif
#ifdef EOVERFLOW
  if (err == EOVERFLOW) { return "EOVERFLOW Value too large"; }
#endif
#ifdef ECANCELED
  if (err == ECANCELED) { return "ECANCELED Asynchronous operation stopped before normal completion"; }
#endif
#ifdef ENOTRECOVERABLE
  if (err == ENOTRECOVERABLE) { return "ENOTRECOVERABLE State not recoverable"; }
#endif
#ifdef EOWNERDEAD
  if (err == EOWNERDEAD) { return "EOWNERDEAD Previous owner died"; }
#endif
#ifdef ESTRPIPE
  if (err == ESTRPIPE) { return "ESTRPIPE Streams pipe error"; }
#endif
  // clang-format on
  return "Unkown error";
}

} // namespace base

#endif
