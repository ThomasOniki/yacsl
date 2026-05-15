#include "base/sys/errno.hpp"
#include "base/containers/string_view.hpp"
#include <errno.h>

namespace base {

StringView errno_str() noexcept { return errno_str(errno); }

// NOLINTNEXTLINE(google-readability-function-size, readability-function-cognitive-complexity)
StringView errno_str(int err) noexcept {
  // clang-format off
if (err == 0) { return "Success"; }	
#ifdef EPERM
if (err == EPERM) { return "Operation not permitted"; }
#endif
#ifdef ENOENT
if (err == ENOENT) { return "No such file or directory"; }
#endif
#ifdef ESRCH
if (err == ESRCH) { return "No such process"; }
#endif
#ifdef EINTR
if (err == EINTR) { return "Interrupted system call"; }
#endif
#ifdef EIO
if (err == EIO) { return "Input/output error"; }
#endif
#ifdef ENXIO
if (err == ENXIO) { return "No such device or address"; }
#endif
#ifdef E2BIG
if (err == E2BIG) { return "Argument list too long"; }
#endif
#ifdef ENOEXEC
if (err == ENOEXEC) { return "Exec format error"; }
#endif
#ifdef EBADF
if (err == EBADF) { return "Bad file descriptor"; }
#endif
#ifdef ECHILD
if (err == ECHILD) { return "No child processes"; }
#endif
#ifdef EAGAIN
if (err == EAGAIN) { return "Resource temporarily unavailable"; }
#endif
#ifdef ENOMEM
if (err == ENOMEM) { return "Cannot allocate memory"; }
#endif
#ifdef EACCES
if (err == EACCES) { return "Permission denied"; }
#endif
#ifdef EFAULT
if (err == EFAULT) { return "Bad address"; }
#endif
#ifdef ENOTBLK
if (err == ENOTBLK) { return "Block device required"; }
#endif
#ifdef EBUSY
if (err == EBUSY) { return "Device or resource busy"; }
#endif
#ifdef EEXIST
if (err == EEXIST) { return "File exists"; }
#endif
#ifdef EXDEV
if (err == EXDEV) { return "Invalid cross-device link"; }
#endif
#ifdef ENODEV
if (err == ENODEV) { return "No such device"; }
#endif
#ifdef ENOTDIR
if (err == ENOTDIR) { return "Not a directory"; }
#endif
#ifdef EISDIR
if (err == EISDIR) { return "Is a directory"; }
#endif
#ifdef EINVAL
if (err == EINVAL) { return "Invalid argument"; }
#endif
#ifdef ENFILE
if (err == ENFILE) { return "Too many open files in system"; }
#endif
#ifdef EMFILE
if (err == EMFILE) { return "Too many open files"; }
#endif
#ifdef ENOTTY
if (err == ENOTTY) { return "Inappropriate ioctl for device"; }
#endif
#ifdef ETXTBSY
if (err == ETXTBSY) { return "Text file busy"; }
#endif
#ifdef EFBIG
if (err == EFBIG) { return "File too large"; }
#endif
#ifdef ENOSPC
if (err == ENOSPC) { return "No space left on device"; }
#endif
#ifdef ESPIPE
if (err == ESPIPE) { return "Illegal seek"; }
#endif
#ifdef EROFS
if (err == EROFS) { return "Read-only file system"; }
#endif
#ifdef EMLINK
if (err == EMLINK) { return "Too many links"; }
#endif
#ifdef EPIPE
if (err == EPIPE) { return "Broken pipe"; }
#endif
#ifdef EDOM
if (err == EDOM) { return "Numerical argument out of domain"; }
#endif
#ifdef ERANGE
if (err == ERANGE) { return "Numerical result out of range"; }
#endif
#ifdef EDEADLK
if (err == EDEADLK) { return "Resource deadlock avoided"; }
#endif
#ifdef ENAMETOOLONG
if (err == ENAMETOOLONG) { return "File name too long"; }
#endif
#ifdef ENOLCK
if (err == ENOLCK) { return "No locks available"; }
#endif
#ifdef ENOSYS
if (err == ENOSYS) { return "Function not implemented"; }
#endif
#ifdef ENOTEMPTY
if (err == ENOTEMPTY) { return "Directory not empty"; }
#endif
#ifdef ELOOP
if (err == ELOOP) { return "Too many levels of symbolic links"; }
#endif
#ifdef ENOMSG
if (err == ENOMSG) { return "No message of desired type"; }
#endif
#ifdef EIDRM
if (err == EIDRM) { return "Identifier removed"; }
#endif
#ifdef ECHRNG
if (err == ECHRNG) { return "Channel number out of range"; }
#endif
#ifdef EL2NSYNC
if (err == EL2NSYNC) { return "Level 2 not synchronized"; }
#endif
#ifdef EL
if (err == EL) { return "HLT 	46 	Level 3 halted"; }
#endif
#ifdef EL
if (err == EL) { return "RST 	47 	Level 3 reset"; }
#endif
#ifdef ELNRNG
if (err == ELNRNG) { return "Link number out of range"; }
#endif
#ifdef EUNATCH
if (err == EUNATCH) { return "Protocol driver not attached"; }
#endif
#ifdef ENOCSI
if (err == ENOCSI) { return "No CSI structure available"; }
#endif
#ifdef EL2HLT
if (err == EL2HLT) { return "Level 2 halted"; }
#endif
#ifdef EBADE
if (err == EBADE) { return "Invalid exchange"; }
#endif
#ifdef EBADR
if (err == EBADR) { return "Invalid request descriptor"; }
#endif
#ifdef EXFULL
if (err == EXFULL) { return "Exchange full"; }
#endif
#ifdef ENOANO
if (err == ENOANO) { return "No anode"; }
#endif
#ifdef EBADRQC
if (err == EBADRQC) { return "Invalid request code"; }
#endif
#ifdef EBADSLT
if (err == EBADSLT) { return "Invalid slot"; }
#endif
#ifdef EBFONT
if (err == EBFONT) { return "Bad font file format"; }
#endif
#ifdef ENOSTR
if (err == ENOSTR) { return "Device not a stream"; }
#endif
#ifdef ENODATA
if (err == ENODATA) { return "No data available"; }
#endif
#ifdef ETIME
if (err == ETIME) { return "Timer expired"; }
#endif
#ifdef ENOSR
if (err == ENOSR) { return "Out of streams resources"; }
#endif
#ifdef ENONET
if (err == ENONET) { return "Machine is not on the network"; }
#endif
#ifdef ENOPKG
if (err == ENOPKG) { return "Package not installed"; }
#endif
#ifdef EREMOTE
if (err == EREMOTE) { return "Object is remote"; }
#endif
#ifdef ENOLINK
if (err == ENOLINK) { return "Link has been severed"; }
#endif
#ifdef EADV
if (err == EADV) { return "Advertise error"; }
#endif
#ifdef ESRMNT
if (err == ESRMNT) { return "Srmount error"; }
#endif
#ifdef ECOMM
if (err == ECOMM) { return "Communication error on send"; }
#endif
#ifdef EPROTO
if (err == EPROTO) { return "Protocol error"; }
#endif
#ifdef EMULTIHOP
if (err == EMULTIHOP) { return "Multihop attempted"; }
#endif
#ifdef EDOTDOT
if (err == EDOTDOT) { return "RFS specific error"; }
#endif
#ifdef EBADMSG
if (err == EBADMSG) { return "Bad message"; }
#endif
#ifdef EOVERFLOW
if (err == EOVERFLOW) { return "Value too large for defined data type"; }
#endif
#ifdef ENOTUNIQ
if (err == ENOTUNIQ) { return "Name not unique on network"; }
#endif
#ifdef EBADFD
if (err == EBADFD) { return "File descriptor in bad state"; }
#endif
#ifdef EREMCHG
if (err == EREMCHG) { return "Remote address changed"; }
#endif
#ifdef ELIBACC
if (err == ELIBACC) { return "Can not access a needed shared library"; }
#endif
#ifdef ELIBBAD
if (err == ELIBBAD) { return "Accessing a corrupted shared library"; }
#endif
#ifdef ELIBSCN
if (err == ELIBSCN) { return ".lib section in a.out corrupted"; }
#endif
#ifdef ELIBMAX
if (err == ELIBMAX) { return "Attempting to link in too many shared libraries"; }
#endif
#ifdef ELIBEXEC
if (err == ELIBEXEC) { return "Cannot exec a shared library directly"; }
#endif
#ifdef EILSEQ
if (err == EILSEQ) { return "Invalid or incomplete multibyte or wide character"; }
#endif
#ifdef ERESTART
if (err == ERESTART) { return "Interrupted system call should be restarted"; }
#endif
#ifdef ESTRPIPE
if (err == ESTRPIPE) { return "Streams pipe error"; }
#endif
#ifdef EUSERS
if (err == EUSERS) { return "Too many users"; }
#endif
#ifdef ENOTSOCK
if (err == ENOTSOCK) { return "Socket operation on non-socket"; }
#endif
#ifdef EDESTADDRREQ
if (err == EDESTADDRREQ) { return "Destination address required"; }
#endif
#ifdef EMSGSIZE
if (err == EMSGSIZE) { return "Message too long"; }
#endif
#ifdef EPROTOTYPE
if (err == EPROTOTYPE) { return "Protocol wrong type for socket"; }
#endif
#ifdef ENOPROTOOPT
if (err == ENOPROTOOPT) { return "Protocol not available"; }
#endif
#ifdef EPROTONOSUPPORT
if (err == EPROTONOSUPPORT) { return "Protocol not supported"; }
#endif
#ifdef ESOCKTNOSUPPORT
if (err == ESOCKTNOSUPPORT) { return "Socket type not supported"; }
#endif
#ifdef EOPNOTSUPP
if (err == EOPNOTSUPP) { return "Operation not supported"; }
#endif
#ifdef EPFNOSUPPORT
if (err == EPFNOSUPPORT) { return "Protocol family not supported"; }
#endif
#ifdef EAFNOSUPPORT
if (err == EAFNOSUPPORT) { return "Address family not supported by protocol"; }
#endif
#ifdef EADDRINUSE
if (err == EADDRINUSE) { return "Address already in use"; }
#endif
#ifdef EADDRNOTAVAIL
if (err == EADDRNOTAVAIL) { return "Cannot assign requested address"; }
#endif
#ifdef ENETDOWN
if (err == ENETDOWN) { return "Network is down"; }
#endif
#ifdef ENETUNREACH
if (err == ENETUNREACH) { return "Network is unreachable"; }
#endif
#ifdef ENETRESET
if (err == ENETRESET) { return "Network dropped connection on reset"; }
#endif
#ifdef ECONNABORTED
if (err == ECONNABORTED) { return "Software caused connection abort"; }
#endif
#ifdef ECONNRESET
if (err == ECONNRESET) { return "Connection reset by peer"; }
#endif
#ifdef ENOBUFS
if (err == ENOBUFS) { return "No buffer space available"; }
#endif
#ifdef EISCONN
if (err == EISCONN) { return "Transport endpoint is already connected"; }
#endif
#ifdef ENOTCONN
if (err == ENOTCONN) { return "Transport endpoint is not connected"; }
#endif
#ifdef ESHUTDOWN
if (err == ESHUTDOWN) { return "Cannot send after transport endpoint shutdown"; }
#endif
#ifdef ETOOMANYREFS
if (err == ETOOMANYREFS) { return "Too many references: cannot splice"; }
#endif
#ifdef ETIMEDOUT
if (err == ETIMEDOUT) { return "Connection timed out"; }
#endif
#ifdef ECONNREFUSED
if (err == ECONNREFUSED) { return "Connection refused"; }
#endif
#ifdef EHOSTDOWN
if (err == EHOSTDOWN) { return "Host is down"; }
#endif
#ifdef EHOSTUNREACH
if (err == EHOSTUNREACH) { return "No route to host"; }
#endif
#ifdef EALREADY
if (err == EALREADY) { return "Operation already in progress"; }
#endif
#ifdef EINPROGRESS
if (err == EINPROGRESS) { return "Operation now in progress"; }
#endif
#ifdef ESTALE
if (err == ESTALE) { return "Stale file handle"; }
#endif
#ifdef EUCLEAN
if (err == EUCLEAN) { return "Structure needs cleaning"; }
#endif
#ifdef ENOTNAM
if (err == ENOTNAM) { return "Not a Xenix named type file"; }
#endif
#ifdef ENAVAIL
if (err == ENAVAIL) { return "No Xenix semaphores available"; }
#endif
#ifdef EISNAM
if (err == EISNAM) { return "Is a named type file"; }
#endif
#ifdef EREMOTEIO
if (err == EREMOTEIO) { return "Remote I/O error"; }
#endif
#ifdef EDQUOT
if (err == EDQUOT) { return "Disk quota exceeded"; }
#endif
#ifdef ENOMEDIUM
if (err == ENOMEDIUM) { return "No medium found"; }
#endif
#ifdef EMEDIUMTYPE
if (err == EMEDIUMTYPE) { return "Wrong medium type"; }
#endif
#ifdef ECANCELED
if (err == ECANCELED) { return "Operation canceled"; }
#endif
#ifdef ENOKEY
if (err == ENOKEY) { return "Required key not available"; }
#endif
#ifdef EKEYEXPIRED
if (err == EKEYEXPIRED) { return "Key has expired"; }
#endif
#ifdef EKEYREVOKED
if (err == EKEYREVOKED) { return "Key has been revoked"; }
#endif
#ifdef EKEYREJECTED
if (err == EKEYREJECTED) { return "Key was rejected by service"; }
#endif
#ifdef EOWNERDEAD
if (err == EOWNERDEAD) { return "Owner died"; }
#endif
#ifdef ENOTRECOVERABLE
if (err == ENOTRECOVERABLE) { return "State not recoverable"; }
#endif
#ifdef ERFKILL
if (err == ERFKILL) { return "Operation not possible due to RF-kill"; }
#endif
#ifdef EHWPOISON
if (err == EHWPOISON) { return "Memory page has hardware error"; }
#endif
#ifdef ENOTSUP
if (err == ENOTSUP) { return "Not supported parameter or option"; }
#endif
#ifdef ENOMEDIUM
if (err == ENOMEDIUM) { return "Missing media"; }
#endif
#ifdef EILSEQ
if (err == EILSEQ) { return "Invalid multibyte sequence"; }
#endif
#ifdef EOVERFLOW
if (err == EOVERFLOW) { return "Value too large"; }
#endif
#ifdef ECANCELED
if (err == ECANCELED) { return "Asynchronous operation stopped before normal completion"; }
#endif
#ifdef ENOTRECOVERABLE
if (err == ENOTRECOVERABLE) { return "State not recoverable"; }
#endif
#ifdef EOWNERDEAD
if (err == EOWNERDEAD) { return "Previous owner died"; }
#endif
#ifdef ESTRPIPE
if (err == ESTRPIPE) { return "Streams pipe error"; }
#endif
  // clang-format on
  return "Unkown error";
}

} // namespace base
