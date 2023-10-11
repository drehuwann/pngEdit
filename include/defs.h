#pragma once

#ifdef WIN32
#include <basetsd.h>
#else  //WIN32
#ifdef POSIX
#include <sys/types.h>
#include <cstdint>
#define SSIZE_T ssize_t
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
//TODO test this on linux
class MyFrame; //Fwd declaration
#define HWND MyFrame *

#else //POSIX
#error nonWIn or nonPosix not implemented yet.
#endif //POSIX
#endif //WIN32

enum Error : SSIZE_T {
    NONE = 0,
    NOFILENAME = -1,
    FAILOPEN = -2,
    FAILREAD = -3,
    FAILWRITE = -4,
    BADHEADER = -5,
    BADFOOTER = -6,
    BADSIGNATURE = -7,
    UNEXPECTEDEOF = -8,
    MEMORYERROR = -9,
    BADCRC = -10,
    FILECLOSED = -11,
    FAILSEEK = -12,
    FAILCLOSE = -13,
    NOTINITIALIZED = -14,
    IENDREACHED = -15,
    OTHER = -16
};

#define BREAKPOINT __asm { int 3; }
