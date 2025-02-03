#pragma once
#include <string>
#include "CSoundFile.h"

#define SNDF_LOADONCALL             0x0010
#define SNDF_PLAYFROMDISK           0x0020

class CSound {
public:
    CSound(const std::string& name, short handle, int frequency, int flags);
    CSound();
    ~CSound();

    void load(int fd, int64_t startOffset, int64_t length);

    short getHandle() const { return handle; }
    const std::string& getName() const { return soundName; }
    int getFlags() const { return flags; }
    int getDuration() const;
    int getOrigFrequency() const { return origFrequency; }

    CSoundFile* getFile() const { return file; }
private:
    short handle = -1;
    std::string soundName;
    int flags = 0;
    int origFrequency = -1;

    CSoundFile* file;
};