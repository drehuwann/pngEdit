#include "pngfile.h"
#include "htonntoh.h"

PngFile::PngFile() : filepath(), fileBuffer(nullptr) {
}

PngFile::~PngFile() {
}

Error PngFile::SetModel(Model *mod) {
    if (!mod) return Error::MEMORYERROR;
    this->model = mod;
    return Error::NONE;
}

SSIZE_T PngFile::Pick(const char *filename) {
    if (*filename == 0) return Error::NOFILENAME;
    filepath = std::filesystem::path(filename);
    Error errCode = Load();
    return (errCode != Error::NONE) ? (SSIZE_T)errCode : std::filesystem::file_size(filepath);
}

Error PngFile::isPng() {
    unsigned char signature[8];
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
    char tag[4] = "\0\0\0";
    if (! std::fread(&tag, sizeof(tag) , 1, fileBuffer)) return Error::FAILREAD;
    if ((tag[0] != 'I') || (tag[1] != 'H') || (tag[2] != 'D') || (tag[3] != 'R')) return Error::BADHEADER;

    // Sets file position indicator on first chunk's first byte
    if (std::fseek(fileBuffer, 0, SEEK_SET) != 0) return Error::FAILSEEK;
    if (! std::fread(&signature, 8 , 1, fileBuffer)) return Error::FAILREAD;

    //Tests succesful !
    return Error::NONE;
}

Error PngFile::Load() {
#ifdef WIN32
    LPCTSTR inStr = (LPCTSTR)(filepath.c_str());
    const char *cStr = ToCstr(inStr);
    if (!(fileBuffer = fopen(cStr, "rb"))) {
        return Error::FAILOPEN;
    }
    if (cStr && cStr != (const char *)(inStr)) {
        delete cStr;
    }
#else
#ifdef POSIX
    if (!(fileBuffer = fopen(filepath.c_str(), "rb"))) {
        return Error::FAILOPEN;
    }
#else  // POSIX
#error nonWin or nonPosix system not implemented yet.
#endif  // POSIX
#endif  // WIN32

    Error errCode;
    if ((errCode = isPng()) != Error::NONE) {
        if (fclose(fileBuffer) == EOF) return Error::FAILCLOSE;
        return errCode;
    }

    // Now parse chunks : 
    //TODO make a vector
    while (errCode == Error::NONE) {
        Chunk *chunk = new Chunk(fileBuffer, this->model);
        if (!chunk) return Error::MEMORYERROR;
        errCode = chunk->Init();
        if (errCode != Error::NONE) {
            delete chunk;
            if (fclose(fileBuffer) == EOF) return Error::FAILCLOSE;
            return errCode;
        }
        ChunkType type = chunk->GetType();
        serializedData serData; // TODO make this dynamic allocated.
        serData.type = type;
        switch (type) {
            case ChunkType::IHDR: {
                serData.data = malloc(sizeof(s_imInfo));
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
                if (size % 3) {
                    errCode = Error::BADHEADER;
                } else {
                    serData.data = malloc(sizeof(s_paletteEntry) * paletteSize);
                    errCode = chunk->Read(serData.data);
                    //TODO moves to model
                }
                break;
            }
            case ChunkType::IDAT: {
                static size_t previousSize = 0;
                static void *data = nullptr;
                UINT32 size = chunk->GetDataSize();
                if (size == 0) {
                    errCode = Error::IDATEMPTY;
                } else {
                    data = realloc(data, previousSize + size);
                    errCode = chunk->Read((char *)data + size);
                    if (errCode != Error::NONE) serData.data = data;
                }
                break;
            }
            case ChunkType::IEND: {
                if (chunk->GetDataSize()) {
                    errCode = Error::BADFOOTER;
                } else {
                    errCode = Error::IENDREACHED; // Normal termination
                }
                break;
            }
            default: {
                serData.data = malloc(chunk->GetDataSize());
                errCode = chunk->Read(serData.data);
                break;
            }
        }
    }
//TODO on reprends ci-dessus...
    if (errCode == Error::IENDREACHED) {
        if (fgetc(fileBuffer) == EOF) {
            errCode = feof(fileBuffer)? Error::NONE : Error::FAILREAD;
        } else {
            fseek(fileBuffer, -1, SEEK_CUR); //rewind the fgetc
            //TODO Tell the user there is something beyond IEND and ask what to do. 
        }
    }
    if (fclose(fileBuffer) == EOF) errCode = Error::FAILCLOSE;
    return errCode;
}
