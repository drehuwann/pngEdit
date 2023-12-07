#include "chunk.h"
#include "htonntoh.h"

cbRead readFn[(int)(ChunkType::TAGS_ARRAY_SIZE)] = { ReadUnknown,
    ReadIHDR, ReadPLTE, ReadIDAT, ReadIEND, ReadcHRM, ReadgAMA, ReadiCCP,
    ReadsBIT, ReadsRGB, ReadbKGD, ReadhIST, ReadtRNS, ReadpHYs, ReadsPLT,
    ReadtIME, ReadiTXt, ReadtEXt, ReadzTXt, ReadoFFs, ReadpCAL, ReadsCAL,
    ReadgIFg, ReadgIFt, ReadgIFx, ReadsTER, ReaddSIG, ReadeXIf, ReadfRAc
};

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
}

Model *Chunk::GetModel() {
    return this->model;
}

bool Chunk::GetInitStatus() {
    return this->isInitialized;
}

unsigned char *Chunk::GetCrcString() {
    return this->crcString;
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
    while (it < (int)ChunkType::TAGS_ARRAY_SIZE) {
        ++ p_testTag; // get rid of typeTag[0] (Unknown)
        ++ it;
        if (p_testTag && *p_testTag == chunkTag) {
            m_type = (ChunkType)it;
            ttag = chunkTag;
            isInitialized = true;
            return Error::NONE;
        }
    }
    isInitialized = true;
    return Error::NONE;
}

Error Chunk::Read(void *data) {
    return readFn[(int)m_type](data, this);
};

Error ReadUnknown(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first   
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;

    //TODO : popup modal to Ask Load or Ignore this chunk
    model->SetChunksHead(owner);
    unsigned char *buf = owner->GetCrcString();
    buf += 4; // jump over 'type' field
    buf += owner->GetDataSize(); // Actually don't read
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadIHDR(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    s_imInfo *imInfo = (s_imInfo *)data;
    static int numHeaders = 0;
    if (numHeaders != 0) return Error::BADHEADER; // not unique
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    if (owner->GetDataSize() != 13) return Error::BADHEADER;
    Chunk *headChunk = model->GetChunksHead();
    if (headChunk) return Error::IHDRNOTFIRST;
    unsigned char *buf = owner->GetCrcString();
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
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadPLTE(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first
    static int numPalettes = 0;
    if (numPalettes != 0) return Error::CHUNKNOTUNIQUE;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    UINT32 size = owner->GetDataSize();
    UINT32 paletteSize = size / 3;
    s_imInfo *p_inf = model->GetInfo();
    if (p_inf == nullptr) return Error::MEMORYERROR;
    if (p_inf->bitfield.colourType.to_ulong() % 4 == 0 ) //coltype 0 or 4 forbidden
        return Error::BADPALETTE;
    s_paletteEntry *palEntry = (s_paletteEntry *)data;
    s_paletteEntry entryRead;
    unsigned char *buf = owner->GetCrcString();
    buf += 4; // jump over 'type' field
    while (paletteSize) {
        entryRead = *((s_paletteEntry *)buf);
        *palEntry = entryRead;
        buf += 3;
        ++palEntry;
        --paletteSize;
    }
    ++ numPalettes;
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadIDAT(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first
    static int numIDAT = 0;
    if (numIDAT != 0) {
        if (headChunk->GetType() != ChunkType::IDAT)
            return Error::IDATNOTCONSECUTIVE;
    }
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    //TODO zlib integration | implementation
    ++ numIDAT;
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadIEND(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::IENDREACHED;
}

Error ReadcHRM(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadgAMA(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadiCCP(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadsBIT(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadsRGB(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadbKGD(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadhIST(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadtRNS(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadpHYs(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadsPLT(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadtIME(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadiTXt(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadtEXt(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadzTXt(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadoFFs(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadpCAL(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadsCAL(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadgIFg(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadgIFt(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadgIFx(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadsTER(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReaddSIG(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadeXIf(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadfRAc(void *data, Chunk *owner) {
   if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

//TODO the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

/* Default template for Read() methods

Error ReadXXXX(void *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first

    <do the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

*/
