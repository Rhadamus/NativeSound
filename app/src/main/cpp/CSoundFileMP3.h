#pragma once
#include "CSoundFile.h"
#define DR_MP3_NO_STDIO
#include <dr_mp3.h>

class CSoundFileMP3 : public CSoundFile {
public:
    struct DataSource {
        int fd;
        int64_t startOffset;
        int64_t endOffset;
    };

    CSoundFileMP3(int fd, int64_t startOffset, int64_t length);
    ~CSoundFileMP3() override;

    bool load() override;
    int64_t read(void* dest, int64_t numFrames) override;
    void seek(int64_t frame) override;
    int64_t tell() override;
    void close() override;

    static bool verify(int fd, int64_t startOffset, int64_t length);
private:
    drmp3 file;
    DataSource datasource;
};