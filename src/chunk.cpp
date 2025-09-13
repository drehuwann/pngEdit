#include "chunk.h"
#include "htonntoh.h"
#include "crc32_c.h"
#include <array>
#include <cstring>

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
    if (! crcString.empty()) crcString.clear();
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
    auto buf = reinterpret_cast<const unsigned char*>(crcString.data());
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

std::vector<unsigned char> &Chunk::GetCrcString() {
    return this->crcString;
};

Error Chunk::Init() {
    if (! crcString.empty()) return Error::OTHER; // We shouldn't alloc again crcString !
    if (this->model == nullptr) return Error::MEMORYERROR; //elementary security
    UINT32 uintBuf = 0;
    if (!std::fread(&uintBuf, sizeof(uintBuf), 1, file)) return Error::FAILREAD;
    size = ntoh(uintBuf);
    uintBuf = 0;
    try {
        crcString.resize(size + 4);
    } catch (const std::bad_alloc &) {
        return Error::MEMORYERROR;
    }
    if (!std::fread(crcString.data(), sizeof(unsigned char), size + 4, file)) {
        crcString.clear();
        return Error::FAILREAD;
    }
    if (!std::fread(&uintBuf, sizeof(uintBuf), 1, file)) return Error::FAILREAD;
    readCRC = ntoh(uintBuf);
    if (!TestCRC()) {
        crcString.clear();
        return Error::BADCRC;
    }
    auto it = (int)(ChunkType::Unknown);
    UINT32 chunkTag = 0;
    std::memcpy(&chunkTag, crcString.data(), sizeof(chunkTag));
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
    unsigned char const *buf = owner->GetCrcString().data();
    buf += 4; // jump over 'type' field
    buf += owner->GetDataSize(); // Actually don't read
    if (buf) {// remove set_but_not_used warning
    }
    owner->SetPrevious(headChunk);
    model->SetChunksHead(owner);
    return Error::NONE;
}

static Error ReadIHDRDimensions(const unsigned char *buf, s_imInfo *imInfo) {
    UINT32 width = 0;
    UINT32 height = 0;
    std::memcpy(&width, buf, sizeof(width));
    std::memcpy(&height, buf + 4, sizeof(height));
    width = ntoh(width);
    height = ntoh(height);

    if (width == 0 || height == 0) return Error::BADHEADER;
    imInfo->width = width;
    imInfo->height = height;
    return Error::NONE;
}

static Error ReadIHDRBitDepth(const unsigned char *buf, s_imInfo *imInfo) {
    unsigned char bitDepth = *buf;
    if (bitDepth > 16) return Error::BADHEADER;

    imInfo->bitfield.bitDepth = std::bitset<5>(bitDepth);
    if (imInfo->bitfield.bitDepth.count() != 1) return Error::BADHEADER;

    return Error::NONE;
}

static Error ReadIHDRColorType(const unsigned char *buf, s_imInfo *imInfo) {
    unsigned char colourType = *buf;
    if (colourType > 6 || colourType == 1 || colourType == 5) return Error::BADHEADER;

    auto depth = imInfo->bitfield.bitDepth.to_ulong();
    switch (colourType) {
        case 2: case 4: case 6:
            if (depth < 8) return Error::BADHEADER;
            break;
        case 3:
            if (depth == 16) return Error::BADHEADER;
            break;
        default:
            return Error::BADHEADER;
    }

    imInfo->bitfield.colourType = std::bitset<3>(colourType);
    return Error::NONE;
}

static Error ValidateByte(unsigned char value) {
    return (value == 0) ? Error::NONE : Error::BADHEADER;
}

Error ReadIHDR(UINT8 *data, Chunk *owner) {
    if (!data || !owner) return Error::MEMORYERROR;

    Model *model = owner->GetModel();
    if (!model) return Error::REQUESTEDOBJECTNOTPRESENT;

    auto *imInfo = (s_imInfo *)data;

    std::shared_ptr<PngFile> fp = model->GetAssociatedFile();
    if (!fp) return Error::REQUESTEDOBJECTNOTPRESENT;

    if ((fp->getParseFlag() & ParseFlag::IHDRseen) != ParseFlag::cleared)
        return Error::CHUNKNOTUNIQUE;

    if (!owner->GetInitStatus()) return Error::NOTINITIALIZED;
    if (owner->GetDataSize() != 13) return Error::BADHEADER;
    if (model->GetChunksHead()) return Error::IHDRNOTFIRST;

    const unsigned char *buf = owner->GetCrcString().data() + 4;

    if (Error err = ReadIHDRDimensions(buf, imInfo); err != Error::NONE) return err;
    buf += 8;

    if (Error err = ReadIHDRBitDepth(buf, imInfo); err != Error::NONE) return err;
    ++buf;

    if (Error err = ReadIHDRColorType(buf, imInfo); err != Error::NONE) return err;
    ++buf;

    if (Error err = ValidateByte(*buf++); err != Error::NONE) return err; // compression
    if (Error err = ValidateByte(*buf++); err != Error::NONE) return err; // filter
    if (*buf > 1) return Error::BADHEADER;

    imInfo->interlace = (*buf == 1);
    owner->SetPrevious(nullptr);
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
    std::shared_ptr<PngFile> fp = model->GetAssociatedFile();
    if(! fp) return Error::REQUESTEDOBJECTNOTPRESENT;    
    if (ParseFlag pf = fp->getParseFlag() & ParseFlag::PLTEseen; pf != ParseFlag::cleared)
        return Error::CHUNKNOTUNIQUE;
    if (!(owner->GetInitStatus())) return Error::NOTINITIALIZED;
    UINT32 size = owner->GetDataSize();
    UINT32 paletteSize = size / 3;
    auto p_inf = model->GetInfo();
    if (p_inf == nullptr) return Error::MEMORYERROR;
    if (p_inf->bitfield.colourType.to_ulong() % 4 == 0 ) //coltype 0 or 4 forbidden
        return Error::BADPALETTE;
    auto *palEntry = (s_paletteEntry *)data;
    s_paletteEntry entryRead;
    unsigned char *buf = owner->GetCrcString().data();
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
