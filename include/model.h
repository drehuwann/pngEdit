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
    s_imInfo() : width(0), height(0), colourTypeDepthFlag(0),
        interlace(false) {};
    UINT32 width;
    UINT32 height;
    union {
        struct {
            std::bitset<5> bitDepth;
            std::bitset<3> colourType;
        } bitfield;
        UINT8 colourTypeDepthFlag;
    };
    bool interlace;
};

struct s_paletteEntry {
    s_paletteEntry() : red(0), green(0), blue(0) {};
    UINT8 red;
    UINT8 green;
    UINT8 blue;
} /*__packed*/;

typedef s_paletteEntry* Palette;

class Model {
public:
    Model(Engine *eng);
    ~Model();

    Engine *GetEngine();
    s_imInfo *GetInfo();
    void SetInfo(s_imInfo *infoPtr);

    /// @brief 
    /// @return the last read chunk.
    /// Use it to free chunk linked list : while(headChunk) delete headChunk;
    /// (See ~Chunk() ...)
    Chunk *GetChunksHead();
    void SetChunksHead(Chunk *head);
    Palette GetPalette();
    void SetPalette(Palette palette);
    int GetNumIDAT();
    void SetNumIDAT(int num);
    UINT8 GetPaletteSize();
    void SetPaletteSize(UINT8 size);
    void PickFile(const char *path);
    PngFile *GetAssociatedFile();
    /// @brief force @ref m_file to nullptr

private:
    Error ReserveInflateBuffer();
    void Reset();

    Engine *eng;

    /// @brief keep track of the last read chunk.
    /// Use it to free chunk linked list : while( @ref headChunk ) delete @ref headChunk;
    /// (See ~Chunk() ...)
    Chunk *headChunk; 

    PngFile *m_file;
    s_imInfo *m_info;
    Byte *inflateBuffer;
    Palette pal;
    int numIDAT;
    UINT8 palSize;
    UINT8 pixelBinarySize;
};
