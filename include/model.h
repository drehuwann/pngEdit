#pragma once

#include "defs.h"
#include "chunk.h"
#include "engine.h"
#include "pngfile.h"
#include <bitset>
#include "zconf.h"

// forward declarations
class Chunk;
class Engine;
class PngFile;

struct s_imInfo {
    s_imInfo() = default;
    UINT32 width = 0;
    UINT32 height = 0;
    union {
        struct {
            std::bitset<5> bitDepth;
            std::bitset<3> colourType;
        } bitfield;
        UINT8 colourTypeDepthFlag = 0;
    };
    bool interlace = false;
};

struct s_paletteEntry {
    s_paletteEntry() = default;
    UINT8 red = 0;
    UINT8 green = 0;
    UINT8 blue = 0;
} /*__packed*/;

using Palette = std::shared_ptr<std::vector<s_paletteEntry>>;
using ImBuffer = std::unique_ptr<std::vector<Byte>>;

class Model {
public:
    explicit Model(Engine *eng);
    ~Model();

    Engine *GetEngine();
    std::shared_ptr<s_imInfo> GetInfo();
    void SetInfo(std::shared_ptr<s_imInfo> infoPtr);

    /// @brief 
    /// @return the last read chunk.
    /// Use it to free chunk linked list : while(headChunk) delete headChunk;
    /// (See ~Chunk() ...)
    Chunk *GetChunksHead();
    void SetChunksHead(Chunk *head);
    Palette GetPalette();
    void SetPalette(const Palette &palette);
    int GetNumIDAT() const;
    void SetNumIDAT(int num);
    UINT8 GetPaletteSize() const;
    void SetPaletteSize(UINT8 size);
    void PickFile(const char *path);
    std::shared_ptr<PngFile> GetAssociatedFile();
    // ImBuffer Accessors for zlib or other consumers
    Byte* InflateBufData() noexcept { return inflateBuffer->data(); }
    size_t InflateBufSize() const noexcept { return inflateBuffer->size(); }

private:
    Error ReserveInflateBuffer();
    void Reset();

    Engine *eng;

    /// @brief keep track of the last read chunk.
    /// Use it to free chunk linked list : while( @ref headChunk ) delete @ref headChunk;
    /// (See ~Chunk() ...)
    Chunk *headChunk; 

    std::shared_ptr<PngFile> m_file;
    std::shared_ptr<s_imInfo> m_info;
    ImBuffer inflateBuffer;
    Palette pal;
    int numIDAT;
    UINT8 palSize;
    UINT8 pixelBinarySize;
};
