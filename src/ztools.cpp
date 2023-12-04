#include "ztools.h"
#ifdef __cplusplus
extern "C" {
#endif

/* persistent variables*/
static UINT16 LZwindowSize = LZwindowMaxSize;

/* private functions */
/// @brief Allocates a Byte array filled with LZ77 encoding of input buffer.
/// @param in address of input buffer. If function succeeds, the input buffer 
/// is freed.
/// @param inSize size of input buffer.
/// @param outSizePtr SIZE_T *, to hold and return allocated Byte array's size.
/// @return Address of allocated Byte array.
/// @return returns NULL on error. in this case, *outSizePtr is set to 0,
/// and *in remains allocated.
static Byte *LZ77Encode(const Byte *in, const SIZE_T inSize,
        SIZE_T *const outSizePtr) {
    if (outSizePtr == NULL) return NULL;
    if (in == NULL || inSize == 0) {
        *outSizePtr = 0;
        return NULL;
    }
    Byte *toRet = NULL;
    SIZE_T outSize  = 0;
    //TODO LZ77Encode implementation
    *outSizePtr = outSize;
    return toRet;
}

/// @brief Allocates a Byte array filled with Huffman encoding of input buffer
/// @param in address of input buffer. If function succeeds, the input buffer 
/// is freed.
/// @param inSize size of input buffer.
/// @param outSizePtr SIZE_T *, to hold and return allocated Byte array's size.
/// @return Address of allocated Byte array.
/// @return returns NULL on error. in this case, *outSizePtr is set to 0,
/// and *in remains allocated.
static Byte *HuffmanEncode(const Byte *in, const SIZE_T inSize,
        SIZE_T *const outSizePtr) {
    if (outSizePtr == NULL) return NULL;
    if (in == NULL || inSize == 0) {
        *outSizePtr = 0;
        return NULL;
    }
    Byte *toRet = NULL;
    SIZE_T outSize  = 0;
    //TODO HuffmanEncode implementation
    *outSizePtr = outSize;
    return toRet;
}

/// @brief Allocates a Byte array filled with LZ77 decoding of input buffer
/// @param in address of input buffer. If function succeeds, the input buffer 
/// is freed.
/// @param inSize size of input buffer.
/// @param outSizePtr SIZE_T *, to hold and return allocated Byte array's size.
/// @return Address of allocated Byte array.
/// @return returns NULL on error. in this case, *outSizePtr is set to 0,
/// and *in remains allocated.
static Byte *LZ77Decode(const Byte *in, const SIZE_T inSize,
        SIZE_T *const outSizePtr) {
    if (outSizePtr == NULL) return NULL;
    if (in == NULL || inSize == 0) {
        *outSizePtr = 0;
        return NULL;
    }
    Byte *toRet = NULL;
    SIZE_T outSize  = 0;
    //TODO LZ77Decode implementation
    *outSizePtr = outSize;
    return toRet;
}

/// @brief Allocates a Byte array filled with Huffman decoding of input buffer
/// @param in address of input buffer. If function succeeds, the input buffer 
/// is freed.
/// @param inSize size of input buffer.
/// @param outSizePtr SIZE_T *, to hold and return allocated Byte array's size.
/// @return Address of allocated Byte array.
/// @return returns NULL on error. in this case, *outSizePtr is set to 0,
/// and *in remains allocated.
static Byte *HuffmanDecode(const Byte *in, const SIZE_T inSize,
        SIZE_T *const outSizePtr) {
    if (outSizePtr == NULL) return NULL;
    if (in == NULL || inSize == 0) {
        *outSizePtr = 0;
        return NULL;
    }
    Byte *toRet = NULL;
    SIZE_T outSize  = 0;
    //TODO HuffmanDecode implementation
    *outSizePtr = outSize;
    return toRet;
}

/* public functions */

// 'tstString.txt' : file masked by .gitignore.
// It contains a song lyrics to be used by 'int TestZtools();'
// /TODO wait for publication authorization.
// Else you can make your own private tstString.txt containing the line:
// const char *tstStr = "<...whatever you want as test string...>";
#include "tstString.txt" //will define tstStr[]

static const SIZE_T inSize = strlen(tstStr) + 1; //+1 for NULL_termination

int TestZtools() {
    int toRet = -1; //error by default
    const Byte *const in = (Byte *)tstStr;

    // used to store last /(LZ77|Huffman)(En|De)code/ function returned pointer
    Byte *out = NULL;

    SIZE_T outSize = 0;
    SIZE_T *const outSizePtr = &outSize;

    if (LZ77Encode(NULL, 0, NULL)) { //should return NULL
        _BP_
    }
    outSize = 10; // explicitly set outSize to a non-null value
    if (LZ77Encode(in, 0, outSizePtr)) {
        //should return NULL and set outSize to 0
        _BP_
    }
    if (outSize) { //outSize not resetted by fonction call
        _BP_
    }
    if ((out = LZ77Encode(in, inSize, outSizePtr))) {
//TODO implement this test
    } else {
        _BP_
    }

    // used to store the size of the last available returned Byte array.
    SIZE_T lastOutSize = *outSizePtr;
    
    if (HuffmanEncode(NULL, 0, NULL)) { //should return NULL
        _BP_
    }
    outSize = 10; // explicitly set outSize to a non-null value
    if (HuffmanEncode(out, 0, outSizePtr)) {
        //should return NULL and set outSize to 0
        _BP_
    }
    if (outSize) { //outSize not resetted by fonction call
        _BP_
    }
    if ((out = HuffmanEncode(out, lastOutSize, outSizePtr))) {
//TODO implement this test
    } else {
        _BP_
    }

    lastOutSize = *outSizePtr;    
    if (HuffmanDecode(NULL, 0, NULL)) { //should return NULL
        _BP_
    }
    outSize = 10; // explicitly set outSize to a non-null value
    if (HuffmanDecode(out, 0, outSizePtr)) {
        //should return NULL and set outSize to 0
        _BP_
    }
    if (outSize) { //outSize not resetted by fonction call
        _BP_
    }
    if ((out = HuffmanEncode(out, lastOutSize, outSizePtr))) {
//TODO implement this test
    } else {
        _BP_
    }
    
    lastOutSize = *outSizePtr;
    if (LZ77Decode(NULL, 0, NULL)) { //should return NULL
        _BP_
    }
    outSize = 10; // explicitly set outSize to a non-null value
    if (LZ77Decode(out, 0, outSizePtr)) {
        //should return NULL and set outSize to 0
        _BP_
    }
    if (outSize) { //outSize not resetted by fonction call
        _BP_
    }
    if ((out = LZ77Decode(out, lastOutSize, outSizePtr))) {
//TODO implement this test
    } else {
        _BP_
    }

    // Now `out` should point on a perfect copy of `in`,
    // and `outSize` should be equal to `inSize`
    if (outSize != inSize) {
        _BP_
    }
    if ((toRet = memcmp((const void *)out, (const void *)in, outSize))) {
        _BP_
    }

    return toRet;
}

#ifdef __cplusplus
}
#endif
