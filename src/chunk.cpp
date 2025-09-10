#include "chunk.h"
#include "htonntoh.h"
#include "crc32_c.h"
#include <array>

using std::array;

const array<cbRead, static_cast<size_t>(ChunkType::TAGS_ARRAY_SIZE)>
    readFn = {ReadUnknown, ReadIHDR, ReadPLTE, ReadIDAT, ReadIEND, ReadcHRM,
              ReadgAMA,    ReadiCCP, ReadsBIT, ReadsRGB, ReadbKGD, ReadhIST,
              ReadtRNS,    ReadpHYs, ReadsPLT, ReadtIME, ReadiTXt, ReadtEXt,
              ReadzTXt,    ReadoFFs, ReadpCAL, ReadsCAL, ReadgIFg, ReadgIFt,
              ReadgIFx,    ReadsTER, ReaddSIG, ReadeXIf, ReadfRAc};

static constexpr array<array<char, 4>, static_cast<std::size_t>(ChunkType::TAGS_ARRAY_SIZE)> typeTag = {{
    {{'\0', '\0', '\0', '\0'}},
    {{'I', 'H', 'D', 'R'}}, {{'P', 'L', 'T', 'E'}}, {{'I', 'D', 'A', 'T'}},
    {{'I', 'E', 'N', 'D'}}, {{'c', 'H', 'R', 'M'}}, {{'g', 'A', 'M', 'A'}},
    {{'i', 'C', 'C', 'P'}}, {{'s', 'B', 'I', 'T'}}, {{'s', 'R', 'G', 'B'}},
    {{'b', 'K', 'G', 'D'}}, {{'h', 'I', 'S', 'T'}}, {{'t', 'R', 'N', 'S'}},
    {{'p', 'H', 'Y', 's'}}, {{'s', 'P', 'L', 'T'}}, {{'t', 'I', 'M', 'E'}},
    {{'i', 'T', 'X', 't'}}, {{'t', 'E', 'X', 't'}}, {{'z', 'T', 'X', 't'}},
    {{'o', 'F', 'F', 's'}}, {{'p', 'C', 'A', 'L'}}, {{'s', 'C', 'A', 'L'}},
    {{'g', 'I', 'F', 'g'}}, {{'g', 'I', 'F', 't'}}, {{'g', 'I', 'F', 'x'}},
    {{'s', 'T', 'E', 'R'}}, {{'d', 'S', 'I', 'G'}}, {{'e', 'X', 'I', 'f'}},
    {{'f', 'R', 'A', 'c'}}
}};

Chunk::~Chunk() {
    if (crcString) free(crcString);
    crcString = nullptr;
    if (this->isInitialized) {
        this->model->SetChunksHead(this->previous);
    } else {
_BP_    // inserted to check if we go there

        // TODO Free there what must be, then cleanly resume:
        // TODO valgrind me!
        // TODO make this house of in-memory cards clean & robust #!
    }
}

bool Chunk::TestCRC() {
    ComputeCRC();
    return (calcCRC == readCRC);
};

void Chunk::ComputeCRC() {
    auto buf = reinterpret_cast<const unsigned char*>(crcString);
    calcCRC = crc32_compute(buf, size + 4);
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

bool Chunk::GetInitStatus() const {
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
    auto it = (int)(ChunkType::Unknown);
    UINT32 chunkTag = *((UINT32 *)crcString);
    auto const *p_testTag = (const UINT32 *)(&typeTag);
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

Error Chunk::Read(UINT8 *data) {
    return readFn[(int)m_type](data, this);
};

Error ReadUnknown(UINT8 *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first   
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;

    //TODO : popup modal to Ask Load or Ignore this chunk
    model->SetChunksHead(owner);
    unsigned char const *buf = owner->GetCrcString();
    buf += 4; // jump over 'type' field
    buf += owner->GetDataSize(); // Actually don't read
    if (buf) {// remove set_but_not_used warning
    }
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadIHDR(UINT8 *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    if (!model) return Error::REQUESTEDOBJECTNOTPRESENT;
    auto *imInfo = (s_imInfo *)data;
    PngFile *fp = model->GetAssociatedFile();
    if(! fp) return Error::REQUESTEDOBJECTNOTPRESENT;
    if (ParseFlag pf = fp->getParseFlag() & ParseFlag::IHDRseen; pf != ParseFlag::cleared)
        return Error::CHUNKNOTUNIQUE;
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
    } else {
        return Error::BADHEADER;
    }
    intRead = ntoh(*((UINT32 *)buf));  // height
    buf += 4;
    if (intRead != 0) {
        imInfo->height = intRead;
    } else {
        return Error::BADHEADER;
    }
    unsigned char byteRead = 0;
    byteRead = *buf;  // bitdepth
    ++buf;
    if (byteRead <= 16) {
        imInfo->bitfield.bitDepth = (std::bitset<5>)byteRead;
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
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    fp->setParseFlag(ParseFlag::IHDRseen);
    return Error::NONE;
}

Error ReadPLTE(UINT8 *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    if(! model) return Error::REQUESTEDOBJECTNOTPRESENT;
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first
    if (model->GetNumIDAT()) return Error::CHUNKSHOULDNOTAPPEARTHERE; //there was already IDAT seen
    PngFile *fp = model->GetAssociatedFile();
    if(! fp) return Error::REQUESTEDOBJECTNOTPRESENT;    
    if (ParseFlag pf = fp->getParseFlag() & ParseFlag::PLTEseen; pf != ParseFlag::cleared)
        return Error::CHUNKNOTUNIQUE;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    UINT32 size = owner->GetDataSize();
    UINT32 paletteSize = size / 3;
    s_imInfo const *p_inf = model->GetInfo();
    if (p_inf == nullptr) return Error::MEMORYERROR;
    if (p_inf->bitfield.colourType.to_ulong() % 4 == 0 ) //coltype 0 or 4 forbidden
        return Error::BADPALETTE;
    auto *palEntry = (s_paletteEntry *)data;
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
    fp->setParseFlag(ParseFlag::PLTEseen);
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadIDAT(UINT8 *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    Model *model = owner->GetModel();
    if(! model) return Error::REQUESTEDOBJECTNOTPRESENT;
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first
    int numIDAT = model->GetNumIDAT();
    if (numIDAT != 0 && headChunk->GetType() != ChunkType::IDAT)
        return Error::IDATNOTCONSECUTIVE;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    //TODO zlib integration | implementation
    ++ numIDAT;
    model->SetNumIDAT(numIDAT);
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

Error ReadIEND(UINT8 *data, Chunk *owner) {
    if (data || ! owner) return Error::MEMORYERROR; //data should be NULL !
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; //this chunk can't come first
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::IENDREACHED;
}

Error ReadcHRM(UINT8 *data, Chunk *owner) {
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

Error ReadgAMA(UINT8 *data, Chunk *owner) {
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

Error ReadiCCP(UINT8 *data, Chunk *owner) {
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

Error ReadsBIT(UINT8 *data, Chunk *owner) {
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

Error ReadsRGB(UINT8 *data, Chunk *owner) {
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

Error ReadbKGD(UINT8 *data, Chunk *owner) {
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

Error ReadhIST(UINT8 *data, Chunk *owner) {
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

Error ReadtRNS(UINT8 *data, Chunk *owner) {
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

Error ReadpHYs(UINT8 *data, Chunk *owner) {
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

Error ReadsPLT(UINT8 *data, Chunk *owner) {
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

Error ReadtIME(UINT8 *data, Chunk *owner) {
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

Error ReadiTXt(UINT8 *data, Chunk *owner) {
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

Error ReadtEXt(UINT8 *data, Chunk *owner) {
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

Error ReadzTXt(UINT8 *data, Chunk *owner) {
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

Error ReadoFFs(UINT8 *data, Chunk *owner) {
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

Error ReadpCAL(UINT8 *data, Chunk *owner) {
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

Error ReadsCAL(UINT8 *data, Chunk *owner) {
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

Error ReadgIFg(UINT8 *data, Chunk *owner) {
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

Error ReadgIFt(UINT8 *data, Chunk *owner) {
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

Error ReadgIFx(UINT8 *data, Chunk *owner) {
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

Error ReadsTER(UINT8 *data, Chunk *owner) {
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

Error ReaddSIG(UINT8 *data, Chunk *owner) {
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

Error ReadeXIf(UINT8 *data, Chunk *owner) {
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

Error ReadfRAc(UINT8 *data, Chunk *owner) {
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

/** Default template for Read() methods

Error ReadXXXX(UINT8 *data, Chunk *owner) {
    if (! data || ! owner) return Error::MEMORYERROR;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    Model *model = owner->GetModel();
    Chunk *headChunk = model->GetChunksHead();
    if (!headChunk) return Error::BADHEADER; #this chunk can't come first

    <do the logic there>

    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

*/
