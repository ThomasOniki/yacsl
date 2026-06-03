#ifdef BASE_PLATFORM_WINDOWS

#pragma once

#include "base/containers/string_view.hpp"
#include <intsafe.h>
#include <winerror.h>

namespace base {

// NOLINTNEXTLINE(google-runtime-int)
inline StringView win32_err_str(DWORD err) {
  switch (err) {
  case 0:
    return "0 ERROR_SUCCESS The operation completed successfully.";
  case 1:
    return "1 ERROR_INVALID_FUNCTION Incorrect function.";
  case 2:
    return "2 ERROR_FILE_NOT_FOUND The system cannot find the file specified.";
  case 3:
    return "3 ERROR_PATH_NOT_FOUND The system cannot find the path specified.";
  case 4:
    return "4 ERROR_TOO_MANY_OPEN_FILES The system cannot open the file.";
  case 5:
    return "5 ERROR_ACCESS_DENIED Access is denied.";
  case 6:
    return "6 ERROR_INVALID_HANDLE The handle is invalid.";
  case 7:
    return "7 ERROR_ARENA_TRASHED The storage control blocks were destroyed.";
  case 8:
    return "8 ERROR_NOT_ENOUGH_MEMORY Not enough memory resources are available to process this "
           "command.";
  case 9:
    return "9 ERROR_INVALID_BLOCK The storage control block address is invalid.";
  case 10:
    return "10 ERROR_BAD_ENVIRONMENT The environment is incorrect.";
  case 11:
    return "11 ERROR_BAD_FORMAT An attempt was made to load a program with an incorrect format.";
  case 12:
    return "12 ERROR_INVALID_ACCESS The access code is invalid.";
  case 13:
    return "13 ERROR_INVALID_DATA The data is invalid.";
  case 14:
    return "14 ERROR_OUTOFMEMORY Not enough storage is available to complete this operation.";
  case 15:
    return "15 ERROR_INVALID_DRIVE The system cannot find the drive specified.";
  case 16:
    return "16 ERROR_CURRENT_DIRECTORY The directory cannot be removed.";
  case 17:
    return "17 ERROR_NOT_SAME_DEVICE The system cannot move the file to a different disk drive.";
  case 18:
    return "18 ERROR_NO_MORE_FILES There are no more files.";
  case 19:
    return "19 ERROR_WRITE_PROTECT The media is write protected.";
  case 20:
    return "20 ERROR_BAD_UNIT The system cannot find the device specified.";
  case 21:
    return "21 ERROR_NOT_READY The device is not ready.";
  case 22:
    return "22 ERROR_BAD_COMMAND The device does not recognize the command.";
  case 23:
    return "23 ERROR_CRC Data error (cyclic redundancy check).";
  case 24:
    return "24 ERROR_BAD_LENGTH The program issued a command but the command length is incorrect.";
  case 25:
    return "25 ERROR_SEEK The drive cannot locate a specific area or track on the disk.";
  case 26:
    return "26 ERROR_NOT_DOS_DISK The specified disk or diskette cannot be accessed.";
  case 27:
    return "27 ERROR_SECTOR_NOT_FOUND The drive cannot find the sector requested.";
  case 28:
    return "28 ERROR_OUT_OF_PAPER The printer is out of paper.";
  case 29:
    return "29 ERROR_WRITE_FAULT The system cannot write to the specified device.";
  case 30:
    return "30 ERROR_READ_FAULT The system cannot read from the specified device.";
  case 31:
    return "31 ERROR_GEN_FAILURE A device attached to the system is not functioning.";
  case 32:
    return "32 ERROR_SHARING_VIOLATION The process cannot access the file because it is being used "
           "by another process.";
  case 33:
    return "33 ERROR_LOCK_VIOLATION The process cannot access the file because another process has "
           "locked a portion of the file.";
  case 34:
    return "34 ERROR_WRONG_DISK The wrong diskette is in the drive. Insert %2 (Volume Serial "
           "Number: %3) into drive %1.";
  case 36:
    return "36 ERROR_SHARING_BUFFER_EXCEEDED Too many files opened for sharing.";
  case 38:
    return "38 ERROR_HANDLE_EOF Reached the end of the file.";
  case 39:
    return "39 ERROR_HANDLE_DISK_FULL The disk is full.";
  case 50:
    return "50 ERROR_NOT_SUPPORTED The request is not supported.";
  case 51:
    return "51 ERROR_REM_NOT_LIST Windows cannot find the network path. Verify that the network "
           "path is correct and the destination computer is not busy or turned off. If Windows "
           "still cannot find t he network path, contact your network administrator.";
  case 52:
    return "52 ERROR_DUP_NAME You were not connected because a duplicate name exists on the "
           "network. If joining a domain, go to System in Control Panel to change the computer "
           "name and try again. If joining a workgroup, choose another workgroup name.";
  case 53:
    return "53 ERROR_BAD_NETPATH The network path was not found.";
  case 54:
    return "54 ERROR_NETWORK_BUSY The network is busy.";
  case 55:
    return "55 ERROR_DEV_NOT_EXIST The specified network resource or device is no longer "
           "available.";
  case 56:
    return "56 ERROR_TOO_MANY_CMDS The network BIOS command limit has been reached.";
  case 57:
    return "57 ERROR_ADAP_HDW_ERR A network adapter hardware error occurred.";
  case 58:
    return "58 ERROR_BAD_NET_RESP The specified server cannot perform the requested operation.";
  case 59:
    return "59 ERROR_UNEXP_NET_ERR An unexpected network error occurred.";
  case 60:
    return "60 ERROR_BAD_REM_ADAP The remote adapter is not compatible.";
  case 61:
    return "61 ERROR_PRINTQ_FULL The printer queue is full.";
  case 62:
    return "62 ERROR_NO_SPOOL_SPACE Space to store the file waiting to be printed is not available "
           "on the server.";
  case 63:
    return "63 ERROR_PRINT_CANCELLED Your file waiting to be printed was deleted.";
  case 64:
    return "64 ERROR_NETNAME_DELETED The specified network name is no longer available.";
  case 65:
    return "65 ERROR_NETWORK_ACCESS_DENIED Network access is denied.";
  case 66:
    return "66 ERROR_BAD_DEV_TYPE The network resource type is not correct.";
  case 67:
    return "67 ERROR_BAD_NET_NAME The network name cannot be found.";
  case 68:
    return "68 ERROR_TOO_MANY_NAMES The name limit for the local computer network adapter card was "
           "exceeded.";
  case 69:
    return "69 ERROR_TOO_MANY_SESS The network BIOS session limit was exceeded.";
  case 70:
    return "70 ERROR_SHARING_PAUSED The remote server has been paused or is in the process of "
           "being started.";
  case 71:
    return "71 ERROR_REQ_NOT_ACCEP No more connections can be made to this remote computer at this "
           "time because there are already as many connections as the computer can accept.";
  case 72:
    return "72 ERROR_REDIR_PAUSED The specified printer or disk device has been paused.";
  case 80:
    return "80 ERROR_FILE_EXISTS The file exists.";
  case 82:
    return "82 ERROR_CANNOT_MAKE The directory or file cannot be created.";
  case 83:
    return "83 ERROR_FAIL_I24 Fail on INT 24.";
  case 84:
    return "84 ERROR_OUT_OF_STRUCTURES Storage to process this request is not available.";
  case 85:
    return "85 ERROR_ALREADY_ASSIGNED The local device name is already in use.";
  case 86:
    return "86 ERROR_INVALID_PASSWORD The specified network password is not correct.";
  case 87:
    return "87 ERROR_INVALID_PARAMETER The parameter is incorrect.";
  case 88:
    return "88 ERROR_NET_WRITE_FAULT A write fault occurred on the network.";
  case 89:
    return "89 ERROR_NO_PROC_SLOTS The system cannot start another process at this time.";
  case 100:
    return "100 ERROR_TOO_MANY_SEMAPHORES Cannot create another system semaphore.";
  case 101:
    return "101 ERROR_EXCL_SEM_ALREADY_OWNED The exclusive semaphore is owned by another process.";
  case 102:
    return "102 ERROR_SEM_IS_SET The semaphore is set and cannot be closed.";
  case 103:
    return "103 ERROR_TOO_MANY_SEM_REQUESTS The semaphore cannot be set again.";
  case 104:
    return "104 ERROR_INVALID_AT_INTERRUPT_TIME Cannot request exclusive semaphores at interrupt "
           "time.";
  case 105:
    return "105 ERROR_SEM_OWNER_DIED The previous ownership of this semaphore has ended.";
  case 106:
    return "106 ERROR_SEM_USER_LIMIT Insert the diskette for drive %1.";
  case 107:
    return "107 ERROR_DISK_CHANGE The program stopped because an alternate diskette was not "
           "inserted.";
  case 108:
    return "108 ERROR_DRIVE_LOCKED The disk is in use or locked by another process.";
  case 109:
    return "109 ERROR_BROKEN_PIPE The pipe has been ended.";
  case 110:
    return "110 ERROR_OPEN_FAILED The system cannot open the device or file specified.";
  case 111:
    return "111 ERROR_BUFFER_OVERFLOW The file name is too long.";
  case 112:
    return "112 ERROR_DISK_FULL There is not enough space on the disk.";
  case 113:
    return "113 ERROR_NO_MORE_SEARCH_HANDLES No more internal file identifiers available.";
  case 114:
    return "114 ERROR_INVALID_TARGET_HANDLE The target internal file identifier is incorrect.";
  case 117:
    return "117 ERROR_INVALID_CATEGORY The IOCTL call made by the application program is not "
           "correct.";
  case 118:
    return "118 ERROR_INVALID_VERIFY_SWITCH The verify-on-write switch parameter value is not "
           "correct.";
  case 119:
    return "119 ERROR_BAD_DRIVER_LEVEL The system does not support the command requested.";
  case 120:
    return "120 ERROR_CALL_NOT_IMPLEMENTED This function is not supported on this system.";
  case 121:
    return "121 ERROR_SEM_TIMEOUT The semaphore timeout period has expired.";
  case 122:
    return "122 ERROR_INSUFFICIENT_BUFFER The data area passed to a system call is too small.";
  case 123:
    return "123 ERROR_INVALID_NAME The filename, directory name, or volume label syntax is "
           "incorrect.";
  case 124:
    return "124 ERROR_INVALID_LEVEL The system call level is not correct.";
  case 125:
    return "125 ERROR_NO_VOLUME_LABEL The disk has no volume label.";
  case 126:
    return "126 ERROR_MOD_NOT_FOUND The specified module could not be found.";
  case 127:
    return "127 ERROR_PROC_NOT_FOUND The specified procedure could not be found.";
  case 128:
    return "128 ERROR_WAIT_NO_CHILDREN There are no child processes to wait for.";
  case 129:
    return "129 ERROR_CHILD_NOT_COMPLETE The %1 application cannot be run in Win32 mode.";
  case 130:
    return "130 ERROR_DIRECT_ACCESS_HANDLE Attempt to use a file handle to an open disk partition "
           "for an operation other than raw disk I/O.";
  case 131:
    return "131 ERROR_NEGATIVE_SEEK An attempt was made to move the file pointer before the "
           "beginning of the file.";
  case 132:
    return "132 ERROR_SEEK_ON_DEVICE The file pointer cannot be set on the specified device or "
           "file.";
  case 133:
    return "133 ERROR_IS_JOIN_TARGET A JOIN or SUBST command cannot be used for a drive that "
           "contains previously joined drives.";
  case 134:
    return "134 ERROR_IS_JOINED An attempt was made to use a JOIN or SUBST command on a drive that "
           "has already been joined.";
  case 135:
    return "135 ERROR_IS_SUBSTED An attempt was made to use a JOIN or SUBST command on a drive "
           "that has already been substituted.";
  case 136:
    return "136 ERROR_NOT_JOINED The system tried to delete the JOIN of a drive that is not "
           "joined.";
  case 137:
    return "137 ERROR_NOT_SUBSTED The system tried to delete the substitution of a drive that is "
           "not substituted.";
  case 138:
    return "138 ERROR_JOIN_TO_JOIN The system tried to join a drive to a directory on a joined "
           "drive.";
  case 139:
    return "139 ERROR_SUBST_TO_SUBST The system tried to substitute a drive to a directory on a "
           "substituted drive.";
  case 140:
    return "140 ERROR_JOIN_TO_SUBST The system tried to join a drive to a directory on a "
           "substituted drive.";
  case 141:
    return "141 ERROR_SUBST_TO_JOIN The system tried to SUBST a drive to a directory on a joined "
           "drive.";
  case 142:
    return "142 ERROR_BUSY_DRIVE The system cannot perform a JOIN or SUBST at this time.";
  case 143:
    return "143 ERROR_SAME_DRIVE The system cannot join or substitute a drive to or for a "
           "directory on the same drive.";
  case 144:
    return "144 ERROR_DIR_NOT_ROOT The directory is not a subdirectory of the root directory.";
  case 145:
    return "145 ERROR_DIR_NOT_EMPTY The directory is not empty.";
  case 146:
    return "146 ERROR_IS_SUBST_PATH The path specified is being used in a substitute.";
  case 147:
    return "147 ERROR_IS_JOIN_PATH Not enough resources are available to process this command.";
  case 148:
    return "148 ERROR_PATH_BUSY The path specified cannot be used at this time.";
  case 149:
    return "149 ERROR_IS_SUBST_TARGET An attempt was made to join or substitute a drive for which "
           "a directory on the drive is the target of a previous substitute.";
  case 150:
    return "150 ERROR_SYSTEM_TRACE System trace information was not specified in your CONFIG.SYS "
           "file, or tracing is disallowed.";
  case 151:
    return "151 ERROR_INVALID_EVENT_COUNT The number of specified semaphore events for "
           "DosMuxSemWait is not correct.";
  case 152:
    return "152 ERROR_TOO_MANY_MUXWAITERS DosMuxSemWait did not execute; too many semaphores are "
           "already set.";
  case 153:
    return "153 ERROR_INVALID_LIST_FORMAT The DosMuxSemWait list is not correct.";
  case 154:
    return "154 ERROR_LABEL_TOO_LONG The volume label you entered exceeds the label character "
           "limit of the target file system.";
  case 155:
    return "155 ERROR_TOO_MANY_TCBS Cannot create another thread.";
  case 156:
    return "156 ERROR_SIGNAL_REFUSED The recipient process has refused the signal.";
  case 157:
    return "157 ERROR_DISCARDED The segment is already discarded and cannot be locked.";
  case 158:
    return "158 ERROR_NOT_LOCKED The segment is already unlocked.";
  case 159:
    return "159 ERROR_BAD_THREADID_ADDR The address for the thread ID is not correct.";
  case 160:
    return "160 ERROR_BAD_ARGUMENTS One or more arguments are not correct.";
  case 161:
    return "161 ERROR_BAD_PATHNAME The specified path is invalid.";
  case 162:
    return "162 ERROR_SIGNAL_PENDING A signal is already pending.";
  case 164:
    return "164 ERROR_MAX_THRDS_REACHED No more threads can be created in the system.";
  case 167:
    return "167 ERROR_LOCK_FAILED Unable to lock a region of a file.";
  case 170:
    return "170 ERROR_BUSY The requested resource is in use.";
  case 171:
    return "171 ERROR_DEVICE_SUPPORT_IN_PROGRESS Device's command support detection is in "
           "progress.";
  case 173:
    return "173 ERROR_CANCEL_VIOLATION A lock request was not outstanding for the supplied cancel "
           "region.";
  case 174:
    return "174 ERROR_ATOMIC_LOCKS_NOT_SUPPORTED The file system does not support atomic changes "
           "to the lock type.";
  case 180:
    return "180 ERROR_INVALID_SEGMENT_NUMBER The system detected a segment number that was not "
           "correct.";
  case 182:
    return "182 ERROR_INVALID_ORDINAL The operating system cannot run %1.";
  case 183:
    return "183 ERROR_ALREADY_EXISTS Cannot create a file when that file already exists.";
  case 186:
    return "186 ERROR_INVALID_FLAG_NUMBER The flag passed is not correct.";
  case 187:
    return "187 ERROR_SEM_NOT_FOUND The specified system semaphore name was not found.";
  case 188:
    return "188 ERROR_INVALID_STARTING_CODESEG The operating system cannot run %1.";
  case 189:
    return "189 ERROR_INVALID_STACKSEG The operating system cannot run %1.";
  case 190:
    return "190 ERROR_INVALID_MODULETYPE The operating system cannot run %1.";
  case 191:
    return "191 ERROR_INVALID_EXE_SIGNATURE Cannot run %1 in Win32 mode.";
  case 192:
    return "192 ERROR_EXE_MARKED_INVALID The operating system cannot run %1.";
  case 193:
    return "193 ERROR_BAD_EXE_FORMAT %1 is not a valid Win32 application.";
  case 194:
    return "194 ERROR_ ITERATED_DATA_EXCEEDS_64k The operating system cannot run %1.";
  case 195:
    return "195 ERROR_INVALID_MINALLOCSIZE The operating system cannot run %1.";
  case 196:
    return "196 ERROR_DYNLINK_FROM_INVALID_RING The operating system cannot run this application "
           "program.";
  case 197:
    return "197 ERROR_IOPL_NOT_ENABLED The operating system is not presently configured to run "
           "this application.";
  case 198:
    return "198 ERROR_INVALID_SEGDPL The operating system cannot run %1.";
  case 199:
    return "199 ERROR_AUTODA TASEG_EXCEEDS_64k The operating system cannot run this application "
           "program.";
  case 200:
    return "200 ERROR_RING2SEG_MUST_BE_MOVABLE The code segment cannot be greater than or equal to "
           "64K.";
  case 201:
    return "201 ERROR_RELOC_CHAIN_XEEDS_SEGLIM The operating system cannot run %1.";
  case 202:
    return "202 ERROR_INFLOOP_IN_RELOC_CHAIN The operating system cannot run %1.";
  case 203:
    return "203 ERROR_ENVVAR_NOT_FOUND The system could not find the environment option that was "
           "entered.";
  case 205:
    return "205 ERROR_NO_SIGNAL_SENT No process in the command subtree has a signal handler.";
  case 206:
    return "206 ERROR_FILENAME_EXCED_RANGE The filename or extension is too long.";
  case 207:
    return "207 ERROR_RING2_STACK_IN_USE The ring 2 stack is in use.";
  case 208:
    return "208 ERROR_META_EXPANSION_TOO_LONG The global filename characters, * or ?, are entered "
           "incorrectly or too many global filename characters are specified.";
  case 209:
    return "209 ERROR_INVALID_SIGNAL_NUMBER The signal being posted is not correct.";
  case 210:
    return "210 ERROR_THREAD_1_INACTIVE The signal handler cannot be set.";
  case 212:
    return "212 ERROR_LOCKED The segment is locked and cannot be reallocated.";
  case 214:
    return "214 ERROR_TOO_MANY_MODULES Too many dynamic-link modules are attached to this program "
           "or dynamic-link module.";
  case 215:
    return "215 ERROR_NESTING_NOT_ALLOWED Cannot nest calls to LoadModule.";
  case 216:
    return "216 ERROR_EXE_MACHINE_TYPE_MISMATCH This version of %1 is not compatible with the "
           "version of Windows you're running. Check your computer's system information and then "
           "contact the software publisher.";
  case 217:
    return "217 ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY The image file %1 is signed, unable to "
           "modify.";
  case 218:
    return "218 ERROR_EXE_CANNOT_MODIFY_STRONG _SIGNED_BINARY The image file %1 is strong signed, "
           "unable to modify.";
  case 220:
    return "220 ERROR_FILE_CHECKED_OUT This file is checked out or locked for editing by another "
           "user.";
  case 221:
    return "221 ERROR_CHECKOUT_REQUIRED The file must be checked out before saving changes.";
  case 222:
    return "222 ERROR_BAD_FILE_TYPE The file type being saved or retrieved has been blocked.";
  case 223:
    return "223 ERROR_FILE_TOO_LARGE The file size exceeds the limit allowed and cannot be saved.";
  case 224:
    return "224 ERROR_FORMS_AUTH_REQUIRED Access Denied. Before opening files in this location, "
           "you must first add the web site to your trusted sites list, browse to the web site, "
           "and select the option to l ogin automatically.";
  case 225:
    return "225 ERROR_VIRUS_INFECTED Operation did not complete successfully because the file "
           "contains a virus or potentially unwanted software.";
  case 226:
    return "226 ERROR_VIRUS_DELETED This file contains a virus or potentially unwanted software "
           "and cannot be opened. Due to the nature of this virus or potentially unwanted "
           "software, the file has bee n removed from this location.";
  case 229:
    return "229 ERROR_PIPE_LOCAL The pipe is local.";
  case 230:
    return "230 ERROR_BAD_PIPE The pipe state is invalid.";
  case 231:
    return "231 ERROR_PIPE_BUSY All pipe instances are busy.";
  case 232:
    return "232 ERROR_NO_DATA The pipe is being closed.";
  case 233:
    return "233 ERROR_PIPE_NOT_CONNECTED No process is on the other end of the pipe.";
  case 234:
    return "234 ERROR_MORE_DATA More data is available.";
  case 240:
    return "240 ERROR_VC_DISCONNECTED The session was canceled.";
  case 254:
    return "254 ERROR_INVALID_EA_NAME The specified extended attribute name was invalid.";
  case 255:
    return "255 ERROR_EA_LIST_INCONSISTENT The extended attributes are inconsistent.";
  case 258:
    return "258 WAIT_TIMEOUT The wait operation timed out.";
  case 259:
    return "259 ERROR_NO_MORE_ITEMS No more data is available.";
  case 266:
    return "266 ERROR_CANNOT_COPY The copy functions cannot be used.";
  case 267:
    return "267 ERROR_DIRECTORY The directory name is invalid.";
  case 275:
    return "275 ERROR_EAS_DIDNT_FIT The extended attributes did not fit in the buffer.";
  case 276:
    return "276 ERROR_EA_FILE_CORRUPT The extended attribute file on the mounted file system is "
           "corrupt.";
  case 277:
    return "277 ERROR_EA_TABLE_FULL The extended attribute table file is full.";
  case 278:
    return "278 ERROR_INVALID_EA_HANDLE The specified extended attribute handle is invalid.";
  case 282:
    return "282 ERROR_EAS_NOT_SUPPORTED The mounted file system does not support extended "
           "attributes.";
  case 288:
    return "288 ERROR_NOT_OWNER Attempt to release mutex not owned by caller.";
  case 298:
    return "298 ERROR_TOO_MANY_POSTS Too many posts were made to a semaphore.";
  case 299:
    return "299 ERROR_PARTIAL_COPY Only part of a ReadProcessMemory or WriteProcessMemory request "
           "was completed.";
  case 300:
    return "300 ERROR_OPLOCK_NOT_GRANTED The oplock request is denied.";
  case 301:
    return "301 ERROR_INVALID_OPLOCK_PROTOCOL An invalid oplock acknowledgment was received by the "
           "system.";
  case 302:
    return "302 ERROR_DISK_TOO_FRAGMENTED The volume is too fragmented to complete this operation.";
  case 303:
    return "303 ERROR_DELETE_PENDING The file cannot be opened because it is in the process of "
           "being deleted.";
  case 304:
    return "304 ERROR_INCOMPATIBLE_WITH_GLOBAL_SHORT_NAME_REGISTRY_SETTING Short name settings may "
           "not be changed on this volume due to the global registry setting.";
  case 305:
    return "305 ERROR_SHORT_NAMES_NOT_ENABLED_ON_VOLUME< /strong> Short names are not enabled on "
           "this volume.";
  case 306:
    return "306 ERROR_SECURITY_STREAM_IS_INCONSISTENT The security stream for the given volume is "
           "in an inconsistent state. Please run CHKDSK on the volume.";
  case 307:
    return "307 ERROR_INVALID_LOCK_RANGE A requested file lock operation cannot be processed due "
           "to an invalid byte range.";
  case 308:
    return "308 ERROR_IMAGE_SUBSYSTEM_NOT_PRESENT The subsystem needed to support the image type "
           "is not present.";
  case 309:
    return "309 ERROR_NOTIFICATION_GUID_ALREADY_DEFINED< /strong> The specified file already has a "
           "notification GUID associated with it.";
  case 310:
    return "310 ERROR_INVALID_EXCEPTION_HANDLER An invalid exception handler routine has been "
           "detected.";
  case 311:
    return "311 ERROR_DUPLICATE_PRIVILEGES Duplicate privileges were specified for the token.";
  case 312:
    return "312 ERROR_NO_RANGES_PROCESSED No ranges for the specified operation were able to be "
           "processed.";
  case 313:
    return "313 ERROR_NOT_ALLOWED_ON_SYSTEM_FILE Operation is not allowed on a file system "
           "internal file.";
  case 314:
    return "314 ERROR_DISK_RESOURCES_EXHAUSTED The physical resources of this disk have been "
           "exhausted.";
  case 315:
    return "315 ERROR_INVALID_TOKEN The token representing the data is invalid.";
  case 316:
    return "316 ERROR_DEVICE_FEATURE_NOT_SUPPORTED The device does not support the command "
           "feature.";
  case 317:
    return "317 ERROR_MR_MID_NOT_FOUND The system cannot find message text for message number 0x%1 "
           "in the message file for %2.";
  case 318:
    return "318 ERROR_SCOPE_NOT_FOUND The scope specified was not found.";
  case 319:
    return "319 ERROR_UNDEFINED_SCOPE The Central Access Policy specified is not defined on the "
           "target machine.";
  case 320:
    return "320 ERROR_INVALID_CAP The Central Access Policy obtained from Active Directory is "
           "invalid.";
  case 321:
    return "321 ERROR_DEVICE_UNREACHABLE The device is unreachable.";
  case 322:
    return "322 ERROR_DEVICE_NO_RESOURCES The target device has insufficient resources to complete "
           "the operation.";
  case 323:
    return "323 ERROR_DATA_CHECKSUM_ERROR A data integrity checksum error occurred. Data in the "
           "file stream is corrupt.";
  case 324:
    return "324 ERROR_INTERMIXED_KERNEL_EA_OPERATION< /p> An attempt was made to modify both a "
           "KERNEL and normal Extended Attribute (EA) in the same operation.";
  case 326:
    return "326 ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED Device does not support file-level TRIM.";
  case 327:
    return "327 ERROR_OFFSET_ALIGNMENT_VIOLATION The command specified a data offset that does not "
           "align to the device's granularity/alignment.";
  case 328:
    return "328 ERROR_INVALID_FIELD_IN_PARAMETER_LIST The command specified an invalid field in "
           "its parameter list.";
  case 329:
    return "329 ERROR_OPERATION_IN_PROGRESS An operation is currently in progress with the device.";
  case 330:
    return "330 ERROR_BAD_DEVICE_PATH An attempt was made to send down the command via an invalid "
           "path to the target device.";
  case 331:
    return "331 ERROR_TOO_MANY_DESCRIPTORS The command specified a number of descriptors that "
           "exceeded the maximum supported by the device.";
  case 332:
    return "332 ERROR_SCRUB_DATA_DISABLED Scrub is disabled on the specified file.";
  case 333:
    return "333 ERROR_NOT_REDUNDANT_STORAGE The storage device does not provide redundancy.";
  case 334:
    return "334 ERROR_RESIDENT_FILE_NOT_SUPPORTED An operation is not supported on a resident "
           "file.";
  case 335:
    return "335 ERROR_COMPRESSED_FILE_NOT_SUPPORTED An operation is not supported on a compressed "
           "file.";
  case 336:
    return "336 ERROR_DIRECTORY_NOT_SUPPORTED An operation is not supported on a directory.";
  case 337:
    return "337 ERROR_NOT_READ_FROM_COPY The specified copy of the requested data could not be "
           "read.";
  case 350:
    return "350 ERROR_FAIL_NOACTION_REBOOT No action was taken as a system reboot is required.";
  case 351:
    return "351 ERROR_FAIL_SHUTDOWN The shutdown operation failed.";
  case 352:
    return "352 ERROR_FAIL_RESTART The restart operation failed.";
  case 353:
    return "353 ERROR_MAX_SESSIONS_REACHED The maximum number of sessions has been reached.";
  case 400:
    return "400 ERROR_THREAD_MODE_ALREADY_BACKGROUND< /p> The thread is already in background "
           "processing mode.";
  case 401:
    return "401 ERROR_THREAD_MODE_NOT_BACKGROUND The thread is not in background processing mode.";
  case 402:
    return "402 ERROR_PROCESS_MODE_ALREADY_BACKGROUND The process is already in background "
           "processing mode.";
  case 403:
    return "403 ERROR_PROCESS_MODE_NOT_BACKGROUND The process is not in background processing "
           "mode.";
  case 487:
    return "487 ERROR_INVALID_ADDRESS Attempt to access invalid address.";
  default:
    return "Unknown error.";
  }
}

inline StringView win32_err_str() { return win32_err_str(GetLastError()); }

} // namespace base

#endif
