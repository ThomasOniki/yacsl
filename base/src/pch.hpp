#ifndef BASE_LSP_ONLY
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BASE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#endif
