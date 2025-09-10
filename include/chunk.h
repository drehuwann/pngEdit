#pragma once

#ifdef WIN32
#include <basetsd.h>
#endif  // WIN32
#include <cstdio>
#include "engine.h"
#include "defs.h"
#include "model.h"
class Model;

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
    fRAc, //    Multiple    None

    /// @brief should be casted to int to reflect the number of ChunkTypes.
    TAGS_ARRAY_SIZE
};

class Chunk;
using cbRead = Error (*)(UINT8 *data, Chunk *owner);

Error ReadUnknown(UINT8 *data, Chunk *owner);
Error ReadIHDR(UINT8 *data, Chunk *owner);
Error ReadPLTE(UINT8 *data, Chunk *owner);
Error ReadIDAT(UINT8 *data, Chunk *owner);
Error ReadIEND(UINT8 *data, Chunk *owner);
Error ReadcHRM(UINT8 *data, Chunk *owner);
Error ReadgAMA(UINT8 *data, Chunk *owner);
Error ReadiCCP(UINT8 *data, Chunk *owner);
Error ReadsBIT(UINT8 *data, Chunk *owner);
Error ReadsRGB(UINT8 *data, Chunk *owner);
Error ReadbKGD(UINT8 *data, Chunk *owner);
Error ReadhIST(UINT8 *data, Chunk *owner);
Error ReadtRNS(UINT8 *data, Chunk *owner);
Error ReadpHYs(UINT8 *data, Chunk *owner);
Error ReadsPLT(UINT8 *data, Chunk *owner);
Error ReadtIME(UINT8 *data, Chunk *owner);
Error ReadiTXt(UINT8 *data, Chunk *owner);
Error ReadtEXt(UINT8 *data, Chunk *owner);
Error ReadzTXt(UINT8 *data, Chunk *owner);
Error ReadoFFs(UINT8 *data, Chunk *owner);
Error ReadpCAL(UINT8 *data, Chunk *owner);
Error ReadsCAL(UINT8 *data, Chunk *owner);
Error ReadgIFg(UINT8 *data, Chunk *owner);
Error ReadgIFt(UINT8 *data, Chunk *owner);
Error ReadgIFx(UINT8 *data, Chunk *owner);
Error ReadsTER(UINT8 *data, Chunk *owner);
Error ReaddSIG(UINT8 *data, Chunk *owner);
Error ReadeXIf(UINT8 *data, Chunk *owner);
Error ReadfRAc(UINT8 *data, Chunk *owner);

class Chunk {
public:
    Chunk(FILE *fileSeek, Model *mod) : model(mod), file(fileSeek) {};

    /// @brief checks this->isInitialized, and free resources accordingly
    virtual ~Chunk();
    
    ChunkType GetType() const {
        return m_type;
    };
    
    UINT32 GetTypeTag() const {
        return ttag;
    };

    UINT32 GetDataSize() const {
        return size;
    };

    /**
     * @brief test CRC validity. If OK, set m_type according to read Tag.
     * Allocates crcString with malloc (freed if error.)
     * crcString contains chunktypeTag+data, and is freed by the destructor.
     * @return FileError 
     */
    enum Error Init();

    Error Read(UINT8 *data);

    bool TestCRC();
    void ComputeCRC();
    Chunk *GetPrevious();
    void SetPrevious(Chunk *prev);
    Model *GetModel();
    bool GetInitStatus() const;
    unsigned char *GetCrcString();

private:
    Model *model;
    unsigned char *crcString = nullptr;
    Chunk *previous = nullptr;
    FILE *file;
    UINT32 readCRC = 0;
    UINT32 calcCRC = 0;
    UINT32 size = 0;
    UINT32 ttag = 0; //type tag seen as uint32_t
    ChunkType m_type = ChunkType::Unknown;
    bool isInitialized = false;
};
