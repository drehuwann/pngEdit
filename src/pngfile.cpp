#include "pngfile.h"
#include "htonntoh.h"
#ifdef WIN32
#include "utf4win.h"
#endif  // WIN32
#include <array>

using std::array;

constexpr array<const UINT8, 8> png_signature = {0x89, 0x50, 0x4e, 0x47,
                                                 0x0d, 0x0a, 0x1a, 0x0a};

ParseFlag operator&(const ParseFlag pf1, const ParseFlag pf2) {
  return (ParseFlag)((UINT32)(pf1) & (UINT32)pf2);
}

PngFile::PngFile() : filepath(), fileBuffer(nullptr), model(nullptr),
        parseflag(ParseFlag::cleared) {
}

PngFile::~PngFile() = default;

Error PngFile::SetModel(Model *mod) {
    if (!mod) return Error::MEMORYERROR;
    this->model = mod;
    return Error::NONE;
}

SSIZE_T PngFile::Pick(const char *filename) {
    if (*filename == 0) return static_cast<SSIZE_T>(Error::NOFILENAME);
    filepath = std::filesystem::path(filename);
    Error errCode = Load();
    return (errCode != Error::NONE) ? static_cast<SSIZE_T>(errCode) :
        static_cast<SSIZE_T>(std::filesystem::file_size(filepath));
}

Error PngFile::isPng() {
    array<UINT8, 8> signature;
    if (! std::fread(&signature, 8 , 1, fileBuffer)) return Error::FAILREAD;
    int it = 0;
    while (it < 8) {
        if (signature[it] != png_signature[it]) return Error::BADSIGNATURE;
        ++ it;
    }

    // should be 13 to be a valid IHDR
    UINT32 buf = 0;
    if (! std::fread(&buf, sizeof(buf) , 1, fileBuffer)) return Error::FAILREAD;
    if (ntoh(buf) != 13) return Error::BADHEADER;
    array<UINT8, 4> tag = {0x00, 0x00, 0x00, 0x00};
    if (! std::fread(&tag, sizeof(tag) , 1, fileBuffer)) return Error::FAILREAD;
    if ((tag[0] != 'I') || (tag[1] != 'H') || (tag[2] != 'D') || (tag[3] != 'R')) return Error::BADHEADER;

    // Sets file position indicator on first chunk's first byte
    if (std::fseek(fileBuffer, 0, SEEK_SET) != 0) return Error::FAILSEEK;
    if (! std::fread(&signature, 8 , 1, fileBuffer)) return Error::FAILREAD;

    //Tests succesful !
    return Error::NONE;
}

ParseFlag PngFile::getParseFlag() const {
    return parseflag;
}

void PngFile::setParseFlag(ParseFlag pf) {
    parseflag = parseflag & pf;
}

Error PngFile::Load() {
    // 1) Open the file (Windows vs POSIX)
    if (auto err = openFile(); err != Error::NONE) 
        return err;

    // 2) Validate PNG signature
    if (auto err = validatePngHeader(); err != Error::NONE) {
        closeFile();
        return err;
    }

    // 3) Read and process all chunks
    auto err = processChunks();

    // 4) Close the file and return final status
    if (auto cerr = closeFile(); err == Error::NONE)
        err = cerr;

    return err;
}

// --- Private helpers implementations ---
Error PngFile::openFile() {
#ifdef WIN32
    // Convert UTF-16 path to narrow string for fopen_s
    const char *cStr = ToCstr(filepath.c_str());
    if (fopen_s(&fileBuffer, cStr, "rb")) {
        delete[] cStr;
        return Error::FAILOPEN;
    }
    delete[] cStr;
#elif defined(POSIX)
    // POSIX fopen
    fileBuffer = fopen(filepath.c_str(), "rb");
    if (!fileBuffer)
        return Error::FAILOPEN;
#else
#  error "Platform not supported"
#endif
    return Error::NONE;
}

Error PngFile::validatePngHeader() {
    // Check PNG signature bytes
    if (auto err = isPng(); err != Error::NONE) {
        fclose(fileBuffer);
        fileBuffer = nullptr;
        return err;
    }
    return Error::NONE;
}

Error PngFile::processChunks() {
    Error err = Error::NONE;
    while (err == Error::NONE) {
        auto chunk = std::make_unique<Chunk>(fileBuffer, model);
        if (!chunk)
            return Error::MEMORYERROR;

        serializedData sd{};
        sd.type = chunk->GetType();
        err = chunk->Init();
        // 2) if Init succeeded, dispatch it
        if (err == Error::NONE)
            err = dispatchChunk(*chunk, sd);
        // 3) single break when anything fails
        if (err != Error::NONE)
            break;
    }
    // post-IEND logic
    if (err == Error::IENDREACHED)
        return finalizeAfterIEND();
    return err;
}

Error PngFile::dispatchChunk(Chunk &chunk, serializedData &sd) {
    // Route chunk processing based on its type
    switch (chunk.GetType()) {
        case ChunkType::IHDR:   return handleIHDR(chunk, sd);
        case ChunkType::PLTE:   return handlePLTE(chunk, sd);
        case ChunkType::IDAT:   return handleIDAT(chunk, sd);
        case ChunkType::IEND:   return handleIEND(chunk, sd);
        default:                return handleGeneric(chunk, sd);
    }
}

Error PngFile::handleIHDR(Chunk &chunk, serializedData &sd) {
    // Allocate and read PNG header info
    auto info = std::make_shared<s_imInfo>();
    if (auto err = chunk.Read(reinterpret_cast<UINT8*>(info.get())); err != Error::NONE)
        return err;
    sd.data = std::make_unique<std::any>(info);
    model->SetInfo(info);
    return Error::NONE;
}
Error PngFile::handlePLTE(Chunk &chunk, serializedData &sd) {
    // Validate PLTE chunk length and read palette entries
    auto size    = chunk.GetDataSize();
    auto palSz   = size/3;

    if (auto maxPal  = (UINT16)(1 << model->GetInfo()->bitfield.bitDepth.to_ulong()); size%3 || palSz > maxPal)
        return Error::BADPALETTE;

    auto palette = std::make_shared<std::vector<s_paletteEntry>>(palSz);
    if (auto err = chunk.Read(reinterpret_cast<UINT8*>(palette->data())); err != Error::NONE)
        return err;

    sd.data = std::make_unique<std::any>(palette);

    model->SetPalette(palette);
    model->SetPaletteSize(static_cast<UINT8>(palSz));
    return Error::NONE;
}

Error PngFile::handleIDAT(Chunk &chunk, serializedData &sd) {
    // Read image data for IDAT chunk
    auto size = chunk.GetDataSize();
    if (size == 0)
        return Error::IDATEMPTY;
    auto buffer = std::make_shared<std::vector<UINT8>>(size);
    if (auto err = chunk.Read(buffer->data()); err != Error::NONE)
        return err;
    sd.data = std::make_unique<std::any>(buffer);

    // TODO: Integrate with zlib decompression
    // 4) optionally hand it off to model
    // model->AppendIDAT(buffer->data(), size);

    return Error::NONE;
}

Error PngFile::handleIEND(Chunk &chunk, serializedData &sd) {
    // Ensure no extra data in IEND
    if (chunk.GetDataSize())
        return Error::BADFOOTER;
    sd.data = std::make_unique<std::any>(nullptr);
    return chunk.Read(nullptr);
}

Error PngFile::handleGeneric(Chunk &chunk, serializedData &sd) {
    // Default handler for unknown chunk types: read raw bytes into a vector
    auto size = chunk.GetDataSize();

    // Allocate exactly 'size' bytes on the heap
    auto buffer = std::make_shared<std::vector<UINT8>>(size);

    // Read chunk data directly into our vector
    if (auto err = chunk.Read(buffer->data()); err != Error::NONE)
        return err;

    // Store the shared_ptr<vector<UINT8>> in sd.data for later retrieval
    sd.data = std::make_unique<std::any>(buffer);

    return Error::NONE;
}


Error PngFile::finalizeAfterIEND() {
    // Check for extra bytes after IEND marker
    if (int c = fgetc(fileBuffer); c == EOF)
        return feof(fileBuffer) ? Error::NONE : Error::FAILREAD;

    // Rewind one byte if not EOF
    fseek(fileBuffer, -1, SEEK_CUR);
    // TODO: Notify user about extra data after IEND
    return Error::NONE;
}

Error PngFile::closeFile() {
    if (!fileBuffer)
        return Error::NONE;

    // Close file handle and clear buffer
    if (fclose(fileBuffer) == EOF)
        return Error::FAILCLOSE;

    fileBuffer = nullptr;
    return Error::NONE;
}

/*
Error PngFile::Load() {
#ifdef WIN32
    auto inStr = filepath.c_str();
    const char *cStr = ToCstr(inStr);
    if (fopen_s(&fileBuffer, cStr, "rb")) {
        return Error::FAILOPEN;
    }
    if (cStr && cStr != (const char *)(inStr)) {
        delete cStr;
    }
#else
#ifdef POSIX
    fileBuffer = fopen(filepath.c_str(), "rb");
    if (fileBuffer == nullptr) {
        return Error::FAILOPEN;
    }
#else  // POSIX
#error nonWin or nonPosix system not implemented yet.
#endif  // POSIX
#endif  // WIN32

    Error errCode;
    if ((errCode = isPng()) != Error::NONE) {
        if (fclose(fileBuffer) == EOF) return Error::FAILCLOSE;
        fileBuffer = nullptr;
        return errCode;
    }

    // Now parse chunks : 
    while (errCode == Error::NONE) {
        auto *chunk = new Chunk(fileBuffer, this->model);
        if (!chunk) return Error::MEMORYERROR;
        errCode = chunk->Init();
        if (errCode != Error::NONE) {
            delete chunk;
            if (fclose(fileBuffer) == EOF) return Error::FAILCLOSE;
            fileBuffer = nullptr;
            return errCode;
        }
        ChunkType type = chunk->GetType();
        serializedData serData; // TODO make this dynamic allocated.
        serData.type = type;
        switch (type) {
            case ChunkType::IHDR: {
                serData.data = (UINT8 *)malloc(sizeof(s_imInfo));
                errCode = chunk->Read(serData.data);
                if (errCode == Error::NONE) {
                    //moves the data into model
                    model->SetInfo((s_imInfo *)(serData.data));
                }
                break;
            }
            case ChunkType::PLTE: {
                UINT32 size = chunk->GetDataSize();
                UINT32 paletteSize = size / 3;
                /// MaxPaletteSize = computed palSize from bitdepth. 
                if (auto MaxPaletteSize = (UINT16)(1 << ((model->GetInfo()->bitfield.bitDepth).to_ulong()));
                    size % 3 || (size / 3) > (UINT32)MaxPaletteSize) {
                        errCode = Error::BADPALETTE;
                        break;
                }
                serData.data = (UINT8 *)malloc(sizeof(s_paletteEntry) * paletteSize);
                errCode = chunk->Read(serData.data);
                if (errCode != Error::NONE) break;
                //moves the data into model
                model->SetPalette((Palette)(serData.data));
                model->SetPaletteSize((UINT8)(0xff & (size / 3)));
                break;
            }
            case ChunkType::IDAT: {
                UINT32 size;
                if ((size = chunk->GetDataSize() == 0)) {
                    errCode = Error::IDATEMPTY;
                    break;
                }
                serData.data = (UINT8 *)malloc(size);
                errCode = chunk->Read(serData.data);
                if (errCode != Error::NONE) break;
                //TODO send the data to zlib
                errCode = Error::NONE; //asserting we managed to hook zlib
                break;
            }
            case ChunkType::IEND: {
                if (chunk->GetDataSize()) {
                    errCode = Error::BADFOOTER;
                } else {
                    //there is nothing to read and NULL will be catched by Chunk::ReadIEND()
                    errCode = chunk->Read(nullptr);
                }
                break;
            }
            default: {
                serData.data = (UINT8 *)malloc(chunk->GetDataSize());
                errCode = chunk->Read(serData.data);
                break;
            }
        }
    }
    if (errCode == Error::IENDREACHED) {
        if (fgetc(fileBuffer) == EOF) {
            errCode = feof(fileBuffer)? Error::NONE : Error::FAILREAD;
        } else {
            fseek(fileBuffer, -1, SEEK_CUR); //rewind the fgetc
            //TODO Tell the user there is something beyond IEND and ask what to do. 
        }
    }
    if (fclose(fileBuffer) == EOF) {
        errCode = Error::FAILCLOSE;
    } else {
        this->fileBuffer = nullptr;
    }
    return errCode;
}
*/
