#pragma once

#include "chunk.h"
#include "utf4win.h"
#include "defs.h"
#include "model.h"
class Model; //fwd decl
#include <cstdio>
#include <filesystem>
#include <vector>

enum class ChunkType : int;

struct serializedData {
    ChunkType type;
    void *data;
};

const unsigned char png_signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a,
        0x1a, 0x0a};

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
};
