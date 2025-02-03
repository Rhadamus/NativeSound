#pragma once
#include <stdint.h>
#include <AL/al.h>

class CSoundFile {
public:
    static CSoundFile* load(int fd, int64_t startOffset, int64_t length);
    virtual ~CSoundFile() = default;

    virtual bool load() = 0;
    // Returns number of frames read, or 0 if EOF, or < 0 on error
    virtual int64_t read(void* dest, int64_t numFrames) = 0;
    virtual void seek(int64_t frame) = 0;
    virtual int64_t tell() = 0;
    virtual void close() = 0;

    void readFileToMemory();
    bool isLoadedInMemory() const { return loaded; }
    ALuint getFileBuffer() const { return fileBuffer; }

    ALenum getFormat() const { return format; }
    int getSampleRate() const { return sampleRate; }
    int64_t getFrameLength() const { return frameLength; }
    int getBytesPerFrame() const { return bytesPerFrame; }
protected:
    int fd;
    int64_t startOffset;
    int64_t length;

    ALenum format;
    int sampleRate;
    int64_t frameLength;
    int bytesPerFrame;

    CSoundFile(int fd, int64_t startOffset, int64_t length);
    void cleanUp();
private:
    ALuint fileBuffer = 0;
    bool loaded = false;
};