module;
#include <stdint.h>
export module base.os.common;

import base.utilities.type_aliases;
import base.ranges.string_view;
import base.containers.stack_array;

export namespace base {

#ifdef _WIN32
using FileHandle                           = void*;
constexpr FileHandle NIL_FILE_HANDLE       = nullptr;
using ProcessHandle                        = void*;
constexpr ProcessHandle NIL_PROCESS_HANDLE = nullptr;
#else
using FileHandle                           = int;
constexpr FileHandle NIL_FILE_HANDLE       = -1;
using ProcessHandle                        = intmax_t;
constexpr ProcessHandle NIL_PROCESS_HANDLE = -1;
#endif

enum class FileOpenFlags : u8 {
  None            = 0,
  Read            = 1 << 0,
  Write           = 1 << 1,
  Trunc           = 1 << 2,
  Create          = 1 << 3,
  SeekEnd         = 1 << 4,
  Append          = 1 << 5,
  DontCloseOnExec = 1 << 6,
};
constexpr FileOpenFlags operator|(FileOpenFlags lhs, FileOpenFlags rhs) noexcept {
  return FileOpenFlags(static_cast<u8>(static_cast<u8>(lhs) | static_cast<u8>(rhs)));
}
constexpr FileOpenFlags operator&(FileOpenFlags lhs, FileOpenFlags rhs) noexcept {
  return FileOpenFlags(static_cast<u8>(static_cast<u8>(lhs) & static_cast<u8>(rhs)));
}
constexpr FileOpenFlags operator~(FileOpenFlags flags) noexcept {
  return FileOpenFlags(~static_cast<u8>(flags));
}

enum class Signal : u8 {
  SigHup    = 1,  // terminate process    terminal line hangup
  SigInt    = 2,  // terminate process    interrupt program
  SigQuit   = 3,  // create core image    quit program
  SigIll    = 4,  // create core image    illegal instruction
  SigTrap   = 5,  // create core image    trace trap
  SigAbrt   = 6,  // create core image    abort program (formerly SIGIOT)
  SigEmt    = 7,  // create core image    emulate instruction executed
  SigFpe    = 8,  // create core image    floating-point exception
  SigKill   = 9,  // terminate process    kill program
  SigBus    = 10, // create core image    bus error
  SigSegv   = 11, // create core image    segmentatoin violatoin
  SigSys    = 12, // create core image    non-existent system call invoked
  SigPipe   = 13, // terminate process    write on a pipe with no reader
  SigAlrm   = 14, // terminate process    real-time timer expired
  SigTerm   = 15, // terminate process    software terminatoin signal
  SigUrg    = 16, // discard signal       urgent condition present on socket
  SigStop   = 17, // stop process         stop (cannot be caught or ignored)
  SigTstp   = 18, // stop process         stop signal generated from keyboard
  SigCont   = 19, // discard signal       continue after stop
  SigChld   = 20, // discard signal       child status has changed
  SigTtin   = 21, // stop process         background read attempted from control terminal
  SigTtou   = 22, // stop process         background write attempted to control terminal
  SigIo     = 23, // discard signal       I/O is possible on a descriptor (see fcntl(2))
  SigXcpu   = 24, // terminate process    cpu time limit exceeded (see setrlimit(2))
  SigXfsz   = 25, // terminate process    file size limit exceeded (see setrlimit(2))
  SigVtalrm = 26, // terminate process    virtual time alarm (see setitimer(2))
  SigProf   = 27, // terminate process    profiling timer alarm (see setitimer(2))
  SigWinch  = 28, // discard signal       Window size change
  SigInfo   = 29, // discard signal       status request from keyboard
  SigUsr1   = 30, // terminate process    User defined signal 1
  SigUsr2   = 31, // terminate process    User defined signal 2
};

} // namespace base
