#include "chunk.h"
#include "htonntoh.h"

Chunk::~Chunk() {
    if (crcString) free(crcString);
    crcString = nullptr;
    if (this->isInitialized) {
        this->model->SetChunksHead(this->previous);
    } else {
        // TODO Free there what must be, then cleanly resume:
        // TODO valgrind me!
        // TODO make this house of in-memory cards clean & robust #!
    }
}

bool Chunk::TestCRC() {
    ComputeCRC();
    return (calcCRC == readCRC);
};

void Chunk::ComputeCRC()  {
    /* Table of CRCs of all 8-bit messages. */
    static unsigned long crc_table[256];

    /* Flag: has the table been computed? Initially false. */
    static int crc_table_computed = 0;
        
    /* Make the table for a fast CRC. */
    if (!crc_table_computed) {
        unsigned long c;
        int n, k;
        for (n = 0; n < 256; n++) {
            c = (unsigned long) n;
            for (k = 0; k < 8; k++) {
                if (c & 1) {
                    c = 0xedb88320L ^ (c >> 1);
                } else {
                    c = c >> 1;
                }
            }
            crc_table[n] = c;
        }
        crc_table_computed = 1;
    }
    
    /* Update a running CRC with the bytes buf[0..len-1]--the CRC
    should be initialized to all 1's, and the transmitted value
    is the 1's complement of the final running CRC. */
        
    unsigned long c = 0xffffffffL;
    size_t n;
    unsigned char *buf = (unsigned char *)crcString;
    if (! buf) throw;
    size_t len = (size_t)(size + 4);
    
    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    calcCRC = (UINT32)(c ^ 0xffffffffL);
}

Chunk *Chunk::GetPrevious() {
    return this->previous;
}

void Chunk::SetPrevious(Chunk *prev) {
    this->previous = prev;
};

Error Chunk::Init() {
    if (crcString) return Error::OTHER; // We shouldn't alloc again crcString !
    if (this->model == nullptr) return Error::MEMORYERROR; //elementary security
    UINT32 uintBuf = 0;
    if (!std::fread(&uintBuf, sizeof(uintBuf), 1, file)) return Error::FAILREAD;
    size = ntoh(uintBuf);
    uintBuf = 0;
    crcString = (unsigned char *)malloc(size + 4);
    if (crcString == nullptr) return Error::MEMORYERROR;
    if (!std::fread(&crcString[0], sizeof(crcString[0]), size + 4, file)) {
        free(crcString);
        return Error::FAILREAD;
    }
    if (!std::fread(&uintBuf, sizeof(uintBuf), 1, file)) return Error::FAILREAD;
    readCRC = ntoh(uintBuf);
    if (!TestCRC()) {
        free(crcString);
        return Error::BADCRC;
    }
    int it = (int)(ChunkType::Unknown);
    UINT32 chunkTag = *((UINT32 *)crcString);
    UINT32 *p_testTag = (UINT32 *)(&typeTag);
    while ((int)it < TAGS_ARRAY_SIZE) {
        ++ p_testTag; // get rid of typeTag[0] (Unknown)
        ++ it;
        if (p_testTag && *p_testTag == chunkTag) {
            m_type = (ChunkType)it;
            isInitialized = true;
            return Error::NONE;
        }
    }
    isInitialized = true;
    return Error::NONE;
};

Error ChunkUnknown::Read(void *data) {
    Chunk *headChunk = this->model->GetChunksHead();
    if (!data) return Error::MEMORYERROR;
    if (!headChunk) return Error::BADHEADER;
    
    if (!isInitialized) return Error::NOTINITIALIZED;

    //TODO : popup modal to Ask Load or Ignore this chunk
    this->model->SetChunksHead(this);
    unsigned char *buf = (unsigned char *)crcString;
    buf += 4; // jump over 'type' field
    buf += this->size; // Actually don't read
    this->SetPrevious(headChunk);
    this->model->SetChunksHead(this);
    return Error::NONE;
}

Error ChunkIHDR::Read(void *data) {
    if (!data) return Error::MEMORYERROR;
    s_imInfo *imInfo = (s_imInfo *)data;
    static int numHeaders = 0;
    if (numHeaders != 0) {
        return Error::BADHEADER; // not unique
    }
    if (!isInitialized) {
        return Error::NOTINITIALIZED;
    }
    if (size != 13) {
        return Error::BADHEADER;
    }
    Chunk *headChunk = this->model->GetChunksHead();
    if (headChunk) return Error::IHDRNOTFIRST;
    unsigned char *buf = (unsigned char *)crcString;
    buf += 4; // jump over 'type' field
    UINT32 intRead = ntoh(*((UINT32 *)buf));  // width
    buf += 4;
    if (intRead != 0) {
        imInfo->width = intRead;
        intRead = 0;
    } else {
        return Error::BADHEADER;
    }
    intRead = ntoh(*((UINT32 *)buf));  // height
    buf += 4;
    if (intRead != 0) {
        imInfo->height = intRead;
        intRead = 0;
    } else {
        return Error::BADHEADER;
    }
    unsigned char byteRead = 0;
    byteRead = *buf;  // bitdepth
    ++buf;
    if (byteRead <= 16) {
        imInfo->bitfield.bitDepth = (std::bitset<5>)byteRead;
        byteRead = 0;
        if (imInfo->bitfield.bitDepth.count() != 1) {
            return Error::BADHEADER;
        }
    } else {
        return Error::BADHEADER;
    }
    byteRead = *buf;  // colourType
    ++buf;
    if ((byteRead <= 6) && (byteRead != 1) && (byteRead != 5)) {
        switch (byteRead) {
            case 2:
            case 4:
            case 6:
                if (imInfo->bitfield.bitDepth.to_ulong() < 8UL) {
                    return Error::BADHEADER;
                }
                break;
            case 3:
                if (imInfo->bitfield.bitDepth.to_ulong() == 16UL) {
                    return Error::BADHEADER;
                }
                break;
            default:
                break;
        }
        imInfo->bitfield.colourType = (std::bitset<3>)byteRead;
        byteRead = 0;
    } else {
        return Error::BADHEADER;
    }
    byteRead = *buf;  // compression type
    ++buf;
    if (byteRead != 0) {
        return Error::BADHEADER;
    }
    byteRead = *buf;  // filter method
    ++buf;
    if (byteRead != 0) {
        return Error::BADHEADER;
    }
    byteRead = *buf;  // interlace
    if (byteRead > 1) {
        return Error::BADHEADER;
    }
    imInfo->interlace = (byteRead == 1);
    ++ numHeaders;
    this->SetPrevious(nullptr);
    this->model->SetChunksHead(this);
    return Error::NONE;
}

Error ChunkPLTE::Read(void *data) {
    if (!data) return Error::MEMORYERROR;
    Chunk *headChunk = this->model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER;
    s_paletteEntry *palEntry = (s_paletteEntry *)data;
    static int numPalettes = 0;
    if (numPalettes != 0) return Error::CHUNKNOTUNIQUE;
    if (!isInitialized) return Error::NOTINITIALIZED;
    UINT32 paletteSize = GetDataSize() / 3;
    if (GetDataSize() % 3) return Error::BADHEADER;
    /*TODO access to imInfo from here
    test paletteSize compatible with colour type and bitdepth
    */
    s_paletteEntry entryRead;
    unsigned char *buf = (unsigned char *)crcString;
    buf += 4; // jump over 'type' field
    while (paletteSize) {
        entryRead = *((s_paletteEntry *)buf);
        *palEntry = entryRead;
        buf += 3;
        ++palEntry;
        --paletteSize;
    }
    ++ numPalettes;
    this->model->SetChunksHead(this);
    this->SetPrevious(headChunk);
    return Error::NONE;
}

Error ChunkIDAT::Read(void *data) {
    if (!data) return Error::MEMORYERROR;
    static int numIDAT = 0;
    Chunk *headChunk = this->model->GetChunksHead();
    if (numIDAT != 0) {
        Chunk *prev = headChunk;
        if (!prev) return Error::OTHER; //It should not happen
        if (prev->GetType() != ChunkType::IDAT)
            return Error::IDATNOTCONSECUTIVE;
    }
    if (!isInitialized) return Error::NOTINITIALIZED;
    //TODO zlib integration | implementation
    ++ numIDAT;
    this->SetPrevious(headChunk);
    this->model->SetChunksHead(this);
    return Error::NONE;
}

Error ChunkIEND::Read(void *data) {
    if (!data) return Error::MEMORYERROR;
    if (!isInitialized) return Error::NOTINITIALIZED;
    Chunk *headChunk = this->model->GetChunksHead();
    this->SetPrevious(headChunk);
    headChunk = this;
    return Error::IENDREACHED;
}

/* Default template for Read() methods

Error ChunkAAAA::Read(void *data) {
    if (!data) return Error::MEMORYERROR;
    if (!isInitialized) return Error::NOTINITIALIZED;
// Do tests here
    ...
// on tests success, do allocations, reading, interpretations...    
    T *buf = (T *)crcString;
    buf += 4; // jump over 'type' field
    ...
// finally update linked list and return 0.
    this->SetPrevious(headChunk);
    headChunk = this;
    return Error::NONE;
}

*/

/*void* cb_cHRM(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_gAMA(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_iCCP(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_sBIT(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_sRGB(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_bKGD(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_hIST(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_tRNS(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_pHYs(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_sPLT(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_tIME(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_iTXt(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_tEXt(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_zTXt(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_oFFs(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_pCAL(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_sCAL(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_gIFg(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_gIFt(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_gIFx(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_sTER(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_dSIG(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_eXIf(Error &errCode, Chunk *parent) {return nullptr;}
void* cb_fRAc(Error &errCode, Chunk *parent) {return nullptr;}
*/
