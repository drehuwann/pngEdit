#pragma once

#ifdef WIN32
#include <basetsd.h>
#include <intrin.h>
#define BREAKPOINT __debugbreak();
enum class MenuID {
    ID_Save = 1, ID_Load, ID_Info, ID_Layo, ID_Exit, ID_Undo, ID_Redo, ID_Abou
};
#else  //WIN32
#ifdef POSIX
#include <sys/types.h>
#include <cstdint>
#define SIZE_T size_t
#define SSIZE_T ssize_t
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define BREAKPOINT __asm__("int $3\n");
class MyFrame; //Fwd declaration
#define HWND MyFrame *
#else //POSIX
#error nonWIn or nonPosix not implemented yet.
#endif //POSIX
#endif //WIN32

#ifdef _DEBUG
    #define _BP_ BREAKPOINT
#else  // _DEBUG
    #undef BREAKPOINT
    #define _BP_  //defines _BP_ to NULL
#endif  // _DEBUG

enum class Error : SSIZE_T {
    NONE = 0,
    NOFILENAME = -1,
    FAILOPEN = -2,
    FAILREAD = -3,
    FAILWRITE = -4,
    BADHEADER = -5,
    IHDRNOTFIRST = -6,
    BADFOOTER = -7,
    BADSIGNATURE = -8,
    UNEXPECTEDEOF = -9,
    MEMORYERROR = -10,
    BADCRC = -11,
    BADPALETTE = -12,
    FAILSEEK = -13,
    FAILCLOSE = -14,
    NOTINITIALIZED = -15,
    CHUNKNOTUNIQUE = -16,
    IDATNOTCONSECUTIVE = -17,
    IDATEMPTY = -18,
    IENDREACHED = -19,
    NOIMGINFO = -20,
    REQUESTEDOBJECTNOTPRESENT = -21,
    CHUNKSHOULDNOTAPPEARTHERE = -22,
    OTHER = -23
};
