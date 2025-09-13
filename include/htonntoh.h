/**
 * @file htonntoh.h
 * @author drehuwann@gmail.com)
 * @brief Generic hton / ntoh. Include only in *.cpp !
 * @version 0.1
 * @date 2022-10-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#ifdef WIN32
#include <winsock2.h>
#else  // WIN32
#if __has_include(<unistd.h>)
#include<arpa/inet.h>
#include "defs.h"
#else  // unistd.h
#error non Win or non POSIX systems not implemented.
#endif  //unistd.h
#endif  // WIN32

template <typename T> T ntoh(T integral);
template <typename T> T hton(T integral);

#ifdef WIN32
#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
    inline UINT16 ntoh(UINT16 integral) {return ntohs(integral);}
    inline UINT16 hton(UINT16 integral) {return htons(integral);}
    inline UINT32 ntoh(UINT32 integral) {return ntohl(integral);}
    inline UINT32 hton(UINT32 integral) {return htonl(integral);}
    inline UINT64 ntoh(UINT64 integral) {return ntohll(integral);}
    inline UINT64 hton(UINT64 integral) {return htonll(integral);}
#else  // REG_DWORD == REG_DWORD_LITTLE_ENDIAN
    template <typename T> inline T ntoh(T integral) {return integral;}
    template <typename T> inline T hton(T integral) {return integral;}
#endif  // REG_DWORD == REG_DWORD_LITTLE_ENDIAN

#else  // WIN32
#if __has_include(<unistd.h>)
inline UINT16 hton(UINT16 integral) {return htons(integral);}
inline UINT32 ntoh(UINT32 integral) {return ntohl(integral);}
inline UINT32 hton(UINT32 integral) {return htonl(integral);}
inline UINT64 ntoh(UINT64 integral) {
    if (1 == ntohl(1)) return integral;
    else return ((((UINT64)ntohl(integral & 0xffffffffUL)) << 32) |
        ntohl((UINT32)(integral >> 32)));
}
inline UINT64 hton(UINT64 integral) {
    if (1 == htonl(1)) return integral;
    else return  ((((UINT64)htonl(integral & 0xffffffffUL)) << 32) |
        htonl((UINT32)(integral >> 32)));
}
#else  // __has_include(<unistd.h>)
#error non Win or non POSIX systems not implemented.
#endif  // __has_include(<unistd.h>)

#endif  // WIN32
