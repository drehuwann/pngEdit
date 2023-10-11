#pragma once

#ifdef WIN32
#include <basetsd.h>
#endif  // WIN32
#include <cstdio>
#include <bitset>
//#include "model.h"
#include "engine.h"
#include "defs.h"

enum class ChunkType : int {
    Unknown, // unrecognized chunk type.

    //Critical chunks (must appear in this order, except PLTE is optional):
    //Name      Multiple    Ordering constraints
    IHDR, //    Unique      Must be first
    PLTE, //    Unique      Before IDAT
    IDAT, //    Multiple    Multiple IDATs must be consecutive
    IEND, //    Unique      Must be last
    
    // Ancillary chunks (need not appear in this order):
    cHRM, //    Unique      Before PLTE and IDAT
    gAMA, //    Unique      Before PLTE and IDAT
    iCCP, //    Unique      Before PLTE and IDAT
    sBIT, //    Unique      Before PLTE and IDAT
    sRGB, //    Unique      Before PLTE and IDAT
    bKGD, //    Unique      After PLTE; before IDAT
    hIST, //    Unique      After PLTE; before IDAT
    tRNS, //    Unique      After PLTE; before IDAT
    pHYs, //    Unique      Before IDAT
    sPLT, //    Multiple    Before IDAT
    tIME, //    Unique      None
    iTXt, //    Multiple    None
    tEXt, //    Multiple    None
    zTXt, //    Multiple    None

    // Extension special purpose chunks :
    oFFs, //    Unique      Before IDAT
    pCAL, //    Unique      Before IDAT
    sCAL, //    Unique      Before IDAT
    gIFg, //    Multiple    None
    gIFt, //    Multiple    None (this chunk is deprecated)
    gIFx, //    Multiple    None
    sTER, //    Unique      Before IDAT
    dSIG, //    Multiple    In pairs, immediately after IHDR and before IEND
    eXIf, //    Unique      None
    fRAc  //    Multiple    None
};

#define TAGS_ARRAY_SIZE 29

const char typeTag[TAGS_ARRAY_SIZE][4] = {"\0\0\0", {'I','H','D','R'}, {'P','L','T','E'}, {'I','D','A','T'}, {'I','E','N','D'}, {'c','H','R','M'},
                            {'g','A','M','A'}, {'i','C','C','P'}, {'s','B','I','T'}, {'s','R','G','B'}, {'b','K','G','D'},
                            {'h','I','S','T'}, {'t','R','N','S'}, {'p','H','Y','s'}, {'s','P','L','T'}, {'t','I','M','E'}, {'i','T','X','t'},
                            {'t','E','X','t'}, {'z','T','X','t'}, {'o','F','F','s'}, {'p','C','A','L'}, {'s','C','A','L'}, {'g','I','F','g'},
                            {'g','I','F','t'}, {'g','I','F','x'}, {'s','T','E','R'}, {'d','S','I','G'}, {'e','X','I','f'}, {'f','R','A','c'}};


/*void *cb_IHDR(FileError &errCode, Chunk *parent);
void *cb_PLTE(FileError &errCode, Chunk *parent);
void *cb_IDAT(FileError &errCode, Chunk *parent);
void *cb_IEND(FileError &errCode, Chunk *parent);
void *cb_cHRM(FileError &errCode, Chunk *parent);
void *cb_gAMA(FileError &errCode, Chunk *parent);
void *cb_iCCP(FileError &errCode, Chunk *parent);
void *cb_sBIT(FileError &errCode, Chunk *parent);
void *cb_sRGB(FileError &errCode, Chunk *parent);
void *cb_bKGD(FileError &errCode, Chunk *parent);
void *cb_hIST(FileError &errCode, Chunk *parent);
void *cb_tRNS(FileError &errCode, Chunk *parent);
void *cb_pHYs(FileError &errCode, Chunk *parent);
void *cb_sPLT(FileError &errCode, Chunk *parent);
void *cb_tIME(FileError &errCode, Chunk *parent);
void *cb_iTXt(FileError &errCode, Chunk *parent);
void *cb_tEXt(FileError &errCode, Chunk *parent);
void *cb_zTXt(FileError &errCode, Chunk *parent);
void *cb_oFFs(FileError &errCode, Chunk *parent);
void *cb_pCAL(FileError &errCode, Chunk *parent);
void *cb_sCAL(FileError &errCode, Chunk *parent);
void *cb_gIFg(FileError &errCode, Chunk *parent);
void *cb_gIFt(FileError &errCode, Chunk *parent);
void *cb_gIFx(FileError &errCode, Chunk *parent);
void *cb_sTER(FileError &errCode, Chunk *parent);
void *cb_dSIG(FileError &errCode, Chunk *parent);
void *cb_eXIf(FileError &errCode, Chunk *parent);
void *cb_fRAc(FileError &errCode, Chunk *parent);


const ReadFn fnArray[TAGS_ARRAY_SIZE] = {nullptr, cb_IHDR, cb_PLTE, cb_IDAT, cb_IEND, cb_cHRM, cb_gAMA, cb_iCCP, cb_sBIT, cb_sRGB, cb_bKGD,
                                        cb_hIST, cb_tRNS, cb_pHYs, cb_sPLT, cb_tIME, cb_iTXt, cb_tEXt, cb_zTXt, cb_oFFs, cb_pCAL, cb_sCAL, cb_gIFg,
                                        cb_gIFt, cb_gIFx, cb_sTER, cb_dSIG, cb_eXIf, cb_fRAc};
*/

class Chunk {
public:
    Chunk(FILE *fileSeek) : crcString(nullptr),
                            size(0),
                            readCRC(0),
                            calcCRC(0),
                            m_type(ChunkType::Unknown),
                            file(fileSeek),
                            isInitialized(false) {};

    Chunk(const Chunk &c) : crcString(c.crcString),
                            size(c.size),
                            readCRC(c.readCRC),
                            calcCRC(c.calcCRC),
                            m_type(c.m_type),
                            file(c.file),
                            isInitialized(c.isInitialized) {};

    virtual ~Chunk() {
        if (crcString) free(crcString);
        crcString = nullptr;
    };
    
    ChunkType GetType() {
        return m_type;
    };
    
    UINT32 GetDataSize() {
        return size;
    };

    /**
     * @brief test CRC validity. If OK, set m_type according to read Tag.
     * Allocates crcString with malloc (freed if error.)
     * crcString contains chunktypeTag+data, and is freed by the destructor.
     * @return FileError 
     */
    enum Error Init();

    virtual Error Read(void *data) = 0;

protected:
    unsigned char *crcString;
    UINT32 size;
    bool isInitialized;
    bool TestCRC();
    void ComputeCRC();

private:
    UINT32 readCRC;
    UINT32 calcCRC;
    ChunkType m_type;
    FILE *file;
};

class ChunkUnknown : public Chunk {
public:
    using Chunk::Chunk;
    ChunkUnknown(const Chunk &c) : Chunk(c) {};
    Error Read(void *data);
};

class ChunkIHDR : public Chunk {
public:
    using Chunk::Chunk;
    ChunkIHDR(const Chunk &c) : Chunk(c) {};
    Error Read(void *data);
};

class ChunkPLTE : public Chunk {
public:
    using Chunk::Chunk;
    ChunkPLTE(const Chunk &c) : Chunk(c) {};
    Error Read(void *data);
};

class ChunkIDAT : public Chunk {
public:
    using Chunk::Chunk;
    ChunkIDAT(const Chunk &c) : Chunk(c) {};
    Error Read(void *data);
};

class ChunkIEND : public Chunk {
public:
    using Chunk::Chunk;
    ChunkIEND(const Chunk &c) : Chunk(c) {};
    Error Read(void *data);
};
