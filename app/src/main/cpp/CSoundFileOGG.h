#pragma once
#include "CSoundFile.h"
#include <vorbis/vorbisfile.h>

class CSoundFileOGG : public CSoundFile {
public:
    struct DataSource {
        int fd;
        int64_t startOffset;
        int64_t endOffset;
    };

    CSoundFileOGG(int fd, int64_t startOffset, int64_t length);
    ~CSoundFileOGG() override;

    bool load() override;
    int64_t read(void* dest, int64_t numFrames) override;
    void seek(int64_t frame) override;
    int64_t tell() override;
    void close() override;

    static bool verify(int fd, int64_t startOffset);
private:
    OggVorbis_File file;
    int curBitstream;
    DataSource datasource;
};