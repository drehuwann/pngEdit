#pragma once

#include "chunk.h"
#include "defs.h"
#include "model.h"
#include <cstdio>
#include <filesystem>

enum class ChunkType : int;

struct serializedData {
    ChunkType type;
    UINT8 *data;
};

enum class ParseFlag : UINT32 {
    cleared = 0, //used to reinit flag
    IHDRseen = 1 << 0,  //Unique      Must be first
    gAMAseen = 1 << 1,  //Unique      Before PLTE and IDAT
    iCCPseen = 1 << 2,  //Unique      Before PLTE and IDAT
    sBITseen = 1 << 3,  //Unique      Before PLTE and IDAT
    sRGBseen = 1 << 4,  //Unique      Before PLTE and IDAT
    cHRMseen = 1 << 5,  //Unique      Before PLTE and IDAT
    pHYsseen = 1 << 6,  //Unique      Before IDAT
    oFFsseen = 1 << 7,  //Unique      Before IDAT
    pCALseen = 1 << 8,  //Unique      Before IDAT
    sCALseen = 1 << 9,  //Unique      Before IDAT
    sTERseen = 1 << 10, //Unique      Before IDAT
    PLTEseen = 1 << 11, //Unique      Before IDAT
    bKGDseen = 1 << 12, //Unique      After PLTE; before IDAT
    hISTseen = 1 << 13, //Unique      After PLTE; before IDAT
    tRNSseen = 1 << 14, //Unique      After PLTE; before IDAT
    tIMEseen = 1 << 15, //Unique      None
    eXIfseen = 1 << 16, //Unique      None
    IENDseen = 1 << 17  //Unique      Must be last
};

ParseFlag operator&(const ParseFlag pf1, const ParseFlag pf2);

// forward declarations
class Model;

class PngFile {
public:
    PngFile();
    ~PngFile();

    Error SetModel(Model *mod);

    /**
     * @brief Opens file. If is is valid .png, loads it.
     *  
     * @return size of file, in bytes. on error returns @ref FileError 
     */
    SSIZE_T Pick(const char *filename);
    ParseFlag getParseFlag() const;
    void setParseFlag(ParseFlag pf);

private:

    /**
     * @brief Test validity of PNG signature and first chunk's conformance. 
     * The file should be prealably opened. This function doesn't check CRC.
     * If test is successful, sets file position indicator on first chunk's first byte.
     * 
     * @return FileError::NONE on success. 
     */
    Error isPng();

    Error Load();

    std::filesystem::path filepath;
    FILE *fileBuffer;
    Model *model;
    ParseFlag parseflag;
};
