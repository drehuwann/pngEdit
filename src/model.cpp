#include "model.h"

/** @brief utility function used in scanline size calculations
/// @param num(erator)
/// @param den(ominator) 
/// @return num / den, rounded up.*/
static inline size_t Ceil(const size_t num, const size_t den) {
    size_t res = num / den;
    if (num % den) res ++;
    return res;
}

Model::Model(Engine *engine) : eng(engine), headChunk(nullptr),
        m_file(nullptr), m_info(nullptr), inflateBuffer(nullptr), pal(nullptr), 
        palSize(0), pixelBinarySize(0) {
}

Model::~Model() {
    while(headChunk) delete(headChunk);
    if (m_file) delete m_file;
    m_file = nullptr;
    if (m_info) delete m_info;
    m_info = nullptr;
    if (inflateBuffer) delete inflateBuffer;
    inflateBuffer = nullptr;
    if (pal) delete pal;
    pal = nullptr;
}

Engine *Model::GetEngine() {
    return this->eng;
}

s_imInfo *Model::GetInfo() {
    return this->m_info;
}

void Model::SetInfo(s_imInfo *infoPtr) {
    this->m_info = infoPtr;
    UINT8 samplesByPixel = 0;
    switch ((this->m_info->bitfield.colourType).to_ulong()) {
        case 0:
        case 3: samplesByPixel = 1; break;
        case 2: samplesByPixel = 3; break;
        case 4: samplesByPixel = 2; break;
        case 6: samplesByPixel = 4; break;
        default: break;
    }
    this->pixelBinarySize = samplesByPixel *
        (UINT8)((this->m_info->bitfield.bitDepth).to_ulong());
}

Chunk *Model::GetChunksHead() {
    return this->headChunk;
}

void Model::SetChunksHead(Chunk *head) {
    this->headChunk = head;
}

Palette Model::GetPalette() {
    return this->pal;
}

void Model::SetPalette(Palette palette) {
    this->pal = palette;
}

UINT8 Model::GetPaletteSize() {
    return this->palSize;
}

void Model::SetPaletteSize(UINT8 size) {
    this->palSize = size;
}

void Model::PickFile(const char *path) {
    if (m_file) {
        //TODO : ask for saving, saves eventually
        delete m_file;
        m_file = nullptr;
        this->Reset();
    }
    m_file = new PngFile();
    m_file->SetModel(this);
    SSIZE_T err = (m_file->Pick(path));
    if (err < 0) throw err;
}

PngFile *Model::GetAssociatedFile() {
    return m_file;
}

void Model::Reset() {
    m_file = nullptr;
    while(headChunk) delete headChunk;
    m_info = nullptr;
    //TODO free buffer and palette IMPORTANT
    inflateBuffer = nullptr;
    pal = nullptr;
    palSize = 0;
    pixelBinarySize = 0;
}

Error Model::ReserveInflateBuffer() {
    if (!(this->m_info)) return Error::NOIMGINFO;
    size_t scanlineSize = 0;
    size_t bufferSize = 0;
    if (this->m_info->interlace) {
        const size_t w = this->m_info->width;
        const size_t h = this->m_info->height;
        //pass 1
        scanlineSize = Ceil(Ceil(w, 8) * this->pixelBinarySize, 8);
        scanlineSize ++; // +1 byte for filter type
        bufferSize += Ceil(h, 8) * scanlineSize;
        if (w > 4) { //pass 2
            scanlineSize = Ceil(Ceil(w - 4, 8) * this->pixelBinarySize, 8);
            scanlineSize ++; // +1 byte for filter type
            bufferSize += Ceil(h, 8) * scanlineSize;
        }
        if (h > 4) { //pass 3
            scanlineSize = Ceil(Ceil(w, 4) * this->pixelBinarySize, 8);
            scanlineSize ++; // +1 byte for filter type
            bufferSize += Ceil(h - 4, 8) * scanlineSize;
        }
        if (w > 2) { //pass 4
            scanlineSize = Ceil(Ceil(w - 2, 4) * this->pixelBinarySize, 8);
            scanlineSize ++; // +1 byte for filter type
            bufferSize += Ceil(h, 4) * scanlineSize;
        }
        if (h > 2) { //pass 5
            scanlineSize = Ceil(Ceil(w, 2) * this->pixelBinarySize, 8);
            scanlineSize ++; // +1 byte for filter type
            bufferSize += Ceil(h - 2, 4) * scanlineSize;
        }
        if (w > 1) { //pass 6
            scanlineSize = Ceil(Ceil(w - 1, 2) * this->pixelBinarySize, 8);
            scanlineSize ++; // +1 byte for filter type
            bufferSize += Ceil(h, 2) * scanlineSize;
        }
        if (h > 1) { //pass 7
            scanlineSize = w;
            scanlineSize ++; // +1 byte for filter type
            bufferSize += Ceil(h - 1, 2) * scanlineSize;
        }
    } else {
        scanlineSize = Ceil(this->m_info->width * this->pixelBinarySize, 8);
        scanlineSize ++; // +1 byte for filter type
        bufferSize = scanlineSize * this->m_info->height;
    }
    //TODO Allocate buffer.
    return Error::NONE;
}
