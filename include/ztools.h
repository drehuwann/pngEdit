#pragma once
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <zlib.h>

/// @brief the first 2 bytes of z-inflated data.
/// See [RFC1950](https://www.rfc-editor.org/rfc/rfc1950)
typedef UINT16 Zheader;

#ifdef __cplusplus
}
#endif
