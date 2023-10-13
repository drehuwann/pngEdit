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

class MyFrame; //Fwd declaration
#define HWND MyFrame *

#else //POSIX
#error nonWIn or nonPosix not implemented yet.
#endif //POSIX
#endif //WIN32

//Common constants
#define ID_Save 1
#define ID_Load 2
#define ID_Info 3
#define ID_Layo 4
#define ID_Exit 5
#define ID_Undo 6
#define ID_Redo 7
#define ID_Abou 8

#define SIZE_X 500
#define SIZE_Y 300
#define aboutStr "\tpng Editor was initially a quick home made tool to work on \
small 16*16 icons defined in png files.\r\nIt growed becoming a tool to check \
eventual steganography embedded in .png files.\r\nCopyright drehuwann@gmail.com\
\r\nPublished under the terms of the General Public License.\r\n\
(See https://gnu.org/licenses/gpl.html)"
#define infoStr "Dimensions(WxH) : %ux%u\r\nColourType/BitDepth : %s/%lu bit(s)\
\r\nInterlace : %s"

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
