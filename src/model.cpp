#include "model.h"
#include <memory>

/** @brief utility function used in scanline size calculations
 *  @param num(erator)
 *  @param den(ominator) 
 *  @return num / den, rounded up.*/
static inline size_t Ceil(const size_t num, const size_t den) {
    size_t res = num / den;
    if (num % den) res ++;
    return res;
}

Model::Model(Engine *engine) : eng(engine), m_file(nullptr), m_info(nullptr),
        inflateBuffer(nullptr), pal(nullptr), numIDAT(0), palSize(0),
        pixelBinarySize(0) {
}

Engine *Model::GetEngine() {
    return this->eng;
}

std::shared_ptr<s_imInfo> Model::GetInfo() const {
    return this->m_info;
}

void Model::SetInfo(std::shared_ptr<s_imInfo> infoPtr) {
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

Palette Model::GetPalette() const {
    return this->pal;
}

void Model::SetPalette(const Palette &palette) {
    this->pal = palette;
}

int Model::GetNumIDAT() const {
    return numIDAT;
}

void Model::SetNumIDAT(int num) {
    numIDAT = num;
}

UINT8 Model::GetPaletteSize() const {
    return this->palSize;
}

void Model::SetPaletteSize(UINT8 size) {
    this->palSize = size;
}

struct Snapshot {
    std::vector<std::unique_ptr<Chunk>> chunks; 
    std::shared_ptr<PngFile> m_file;
    std::shared_ptr<s_imInfo> m_info;
    ImBuffer inflateBuffer;
    Palette pal;
    int numIDAT;
    UINT8 palSize;
    UINT8 pixelBinarySize;
};

void Model::PickFile(const char* path) {
    if (m_file) {
        // TODO: if (Model::isDirty() prompt user to save;
        // if user say yes, call Save()…
        // But don’t destroy the old Model until the new one is known‐good
    }
    Snapshot backup {
        std::move(chunks), 
        std::move(m_file),
        std::move(m_info),
        std::move(inflateBuffer),
        std::move(pal),
        numIDAT,
        palSize,
        pixelBinarySize
    };
    Reset();
    // Lazily create our PngFile object if we didn’t have one already
    if (!m_file) {
        m_file = std::make_unique<PngFile>();
        m_file->SetModel(this);
    }
    
    if (SSIZE_T err = m_file->Pick(path); err < 0) {
        // on failure, restore the backup
        chunks = std::move(backup.chunks);
        m_file = std::move(backup.m_file);
        m_info = std::move(backup.m_info);
        inflateBuffer = std::move(backup.inflateBuffer);
        pal = std::move(backup.pal);
        numIDAT = backup.numIDAT;
        palSize = backup.palSize;
        pixelBinarySize = backup.pixelBinarySize;
        throw err;
    }

}

std::shared_ptr<PngFile> Model::GetAssociatedFile() const {
    return m_file;
}

std::vector<std::unique_ptr<Chunk>> &Model::GetChunks() noexcept {
    return chunks;
}

void Model::Reset() {
    chunks.clear();
    m_file.reset();
    m_info.reset();
    inflateBuffer.reset();
    pal.reset();
    numIDAT = 0;
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
    // Allocate a brand-new vector<Byte> of the exact requested size
    inflateBuffer = std::make_unique<std::vector<Byte>>(bufferSize);
    return Error::NONE;
}
