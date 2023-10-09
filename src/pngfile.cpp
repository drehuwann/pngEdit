#include "pngfile.h"
#include "htonntoh.h"

PngFile::PngFile() : filepath(), fileBuffer(nullptr) {
}

PngFile::~PngFile() {
}

SSIZE_T PngFile::Pick(const char *filename) {
    if (*filename == 0) return Error::NOFILENAME;
    filepath = std::filesystem::path(filename);
    return (SSIZE_T)Load();
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

    // Sets file position indicator on first chink's firdt byte
    if (std::fseek(fileBuffer, 0, SEEK_SET) != 0) return Error::FAILSEEK;
    if (! std::fread(&signature, 8 , 1, fileBuffer)) return Error::FAILREAD;

    //Tests succesful !
    return Error::NONE;
}

Error PngFile::Load() {
#ifdef WIN32
    if (!(fileBuffer = fopen(ToCstr((LPCTSTR)(filepath.c_str())), "rb"))) {
        return Error::FAILOPEN;
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
        ChunkUnknown *chunk = new ChunkUnknown(fileBuffer);
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
                ChunkIHDR ch = ChunkIHDR(*chunk);
                serData.data = malloc(sizeof(s_imInfo));
                errCode = ch.Read(serData.data);
                break;
            }
            case ChunkType::PLTE: {
                ChunkPLTE ch = ChunkPLTE(*chunk);
                UINT32 paletteSize = ch.GetDataSize() / 3;
                if (ch.GetDataSize() % 3) {
                    errCode = Error::BADHEADER;
                } else {
                    serData.data = malloc(sizeof(s_paletteEntry) * paletteSize);
                    errCode = ch.Read(serData.data);
                }
                break;
            }
            case ChunkType::IEND: {
                ChunkIEND ch = ChunkIEND(*chunk);
                if (ch.GetDataSize()) {
                    errCode = Error::BADFOOTER;
                } else {
                    errCode = Error::IENDREACHED;
                }
                break;
            }
            default: {
                ChunkUnknown ch = ChunkUnknown(*chunk);
                serData.data = malloc(ch.GetDataSize());
                errCode = ch.Read(serData.data);
                break;
            }
        }
    }
//TODO on reprends ci-dessus...

    return errCode;
}

size_t PngFile::Size() {size_t size = 0; return size;}
