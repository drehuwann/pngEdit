#include "crc32_c.h"

uint32_t crc32_compute(const unsigned char* buf, size_t len) {
    /* Table of CRCs of all 8-bit messages. */
    static unsigned long crc_table[256];

    /* Flag: has the table been computed? Initially false. */
    static int crc_table_computed = 0;
        
    /* Make the table for a fast CRC. */
    if (!crc_table_computed) {
        for (int n = 0; n < 256; ++n) {
            unsigned long c = (unsigned long) n;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? 0xedb88320L ^ (c >> 1) : c >> 1;
            crc_table[n] = c;
        }
        crc_table_computed = 1;
    }
    
    /* Update a running CRC with the bytes buf[0..len-1]--the CRC
    should be initialized to all 1's, and the transmitted value
    is the 1's complement of the final running CRC. */        
    unsigned long c = 0xffffffffL;
    for (size_t n = 0; n < len; ++n)
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    return (c ^ 0xffffffffUL) & 0xffffffff; // returns crc forced to 32 bits
}
