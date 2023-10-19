#pragma once

#ifdef WIN32
#include <basetsd.h>
#endif  // WIN32
#include "ztools.h"
#include <cstdio>
#include <bitset>
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

const char typeTag[(int)(ChunkType::TAGS_ARRAY_SIZE)][4] = {"\0\0\0",
    {'I','H','D','R'}, {'P','L','T','E'}, {'I','D','A','T'}, {'I','E','N','D'},
    {'c','H','R','M'}, {'g','A','M','A'}, {'i','C','C','P'}, {'s','B','I','T'},
    {'s','R','G','B'}, {'b','K','G','D'}, {'h','I','S','T'}, {'t','R','N','S'},
    {'p','H','Y','s'}, {'s','P','L','T'}, {'t','I','M','E'}, {'i','T','X','t'},
    {'t','E','X','t'}, {'z','T','X','t'}, {'o','F','F','s'}, {'p','C','A','L'},
    {'s','C','A','L'}, {'g','I','F','g'}, {'g','I','F','t'}, {'g','I','F','x'},
    {'s','T','E','R'}, {'d','S','I','G'}, {'e','X','I','f'}, {'f','R','A','c'}
};

class Chunk;
typedef Error(*cbRead)(void *data, Chunk *owner);

Error ReadUnknown(void *data, Chunk *owner);
Error ReadIHDR(void *data, Chunk *owner);
Error ReadPLTE(void *data, Chunk *owner);
Error ReadIDAT(void *data, Chunk *owner);
Error ReadIEND(void *data, Chunk *owner);
Error ReadcHRM(void *data, Chunk *owner);
Error ReadgAMA(void *data, Chunk *owner);
Error ReadiCCP(void *data, Chunk *owner);
Error ReadsBIT(void *data, Chunk *owner);
Error ReadsRGB(void *data, Chunk *owner);
Error ReadbKGD(void *data, Chunk *owner);
Error ReadhIST(void *data, Chunk *owner);
Error ReadtRNS(void *data, Chunk *owner);
Error ReadpHYs(void *data, Chunk *owner);
Error ReadsPLT(void *data, Chunk *owner);
Error ReadtIME(void *data, Chunk *owner);
Error ReadiTXt(void *data, Chunk *owner);
Error ReadtEXt(void *data, Chunk *owner);
Error ReadzTXt(void *data, Chunk *owner);
Error ReadoFFs(void *data, Chunk *owner);
Error ReadpCAL(void *data, Chunk *owner);
Error ReadsCAL(void *data, Chunk *owner);
Error ReadgIFg(void *data, Chunk *owner);
Error ReadgIFt(void *data, Chunk *owner);
Error ReadgIFx(void *data, Chunk *owner);
Error ReadsTER(void *data, Chunk *owner);
Error ReaddSIG(void *data, Chunk *owner);
Error ReadeXIf(void *data, Chunk *owner);
Error ReadfRAc(void *data, Chunk *owner);

class Chunk {
public:
    Chunk(FILE *fileSeek, Model *mod) :
                            model(mod),
                            crcString(nullptr),
                            previous(nullptr),
                            file(fileSeek),
                            readCRC(0),
                            calcCRC(0),
                            size(0),
                            m_type(ChunkType::Unknown),
                            isInitialized(false) {};

/*    Chunk(const Chunk &c) : 
                            model(c.model),
                            crcString(c.crcString),
                            previous(c.previous),
                            file(c.file),
                            readCRC(c.readCRC),
                            calcCRC(c.calcCRC),
                            size(c.size),
                            m_type(c.m_type),
                            isInitialized(c.isInitialized) {};
*/

    /// @brief checks this->isInitialized, and free resources accordingly
    virtual ~Chunk();
    
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

    Error Read(void *data);

    bool TestCRC();
    void ComputeCRC();
    Chunk *GetPrevious();
    void SetPrevious(Chunk *prev);
    Model *GetModel();
    bool GetInitStatus();
    unsigned char *GetCrcString();

private:
    Model *model;
    unsigned char *crcString;
    Chunk *previous;
    FILE *file;
    UINT32 readCRC;
    UINT32 calcCRC;
    UINT32 size;
    ChunkType m_type;
    bool isInitialized;
};
