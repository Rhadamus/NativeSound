#pragma once
#include "CSoundFile.h"

class CSoundFileWAV : public CSoundFile {
public:
    CSoundFileWAV(int fd, int64_t startOffset, int64_t length);
    ~CSoundFileWAV() override;

    bool load() override;
    int64_t read(void* dest, int64_t numFrames) override;
    void seek(int64_t frame) override;
    int64_t tell() override;
    void close() override;

    static bool verify(int fd, int64_t startOffset);
private:
    int64_t dataOffset;
};