#pragma once
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** short portable implementation of zlib, according to [this article]
(https://www.euccas.me/zlib/)
*/

#include <stdlib.h>
#include <string.h>

/// @brief the first 2 bytes of z-inflated data.
/// See [RFC1950](https://www.rfc-editor.org/rfc/rfc1950)
typedef UINT16 Zheader;
typedef UINT8 Byte;

/// @brief temporary function for testing ztools.
/// @return 0 on success.
int TestZtools();

#ifdef __cplusplus
}
#endif
