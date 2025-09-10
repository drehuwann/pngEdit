#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

uint32_t crc32_compute(const unsigned char* buf, size_t len);

#ifdef __cplusplus
}
#endif
