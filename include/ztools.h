#pragma once
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#ifdef WIN32
#define Bytef unsigned char
//TODO implement zlib
#else  // WIN32
#ifdef POSIX
#include <zlib.h>
#else  // POSIX
#error non win or non posix systems not implemented
#endif  // POSIX
#endif  // WIN32

/// @brief the first 2 bytes of z-inflated data.
/// See [RFC1950](https://www.rfc-editor.org/rfc/rfc1950)
typedef UINT16 Zheader;

#ifdef __cplusplus
}
#endif
