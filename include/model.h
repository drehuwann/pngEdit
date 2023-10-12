#pragma once

#include "defs.h"
#include "engine.h"
#include "pngfile.h"
class Engine;   //fwd decl
class PngFile;  //fwd decl

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
    Model(const Engine &eng);
    ~Model();

    const Engine &GetEngine();
    void SetInfo(s_imInfo *infoPtr);
    void PickFile(const char *path);

private:
    const Engine &eng;
    PngFile *m_file;
    s_imInfo *m_info;
    Palette pal;
};
