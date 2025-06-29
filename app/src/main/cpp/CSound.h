#pragma once
#include <string>
#include <jni.h>
#include "CSoundFile.h"

#define SNDF_LOADONCALL             0x0010
#define SNDF_PLAYFROMDISK           0x0020

class CSound {
public:
    CSound(const std::string& name, short handle, int frequency, int flags);
    CSound();
    ~CSound();

    void load(JNIEnv*, jobject jniSound);
    CSoundFile* openFile(JNIEnv*, jobject jniSound) const;
    void loadBuffer(JNIEnv*, jobject jniSound);

    short getHandle() const { return handle; }
    const std::string& getName() const { return soundName; }
    int getFlags() const { return flags; }
    int64_t getFrameLength() const { return frameLength; }
    int getDuration() const { return (int)(frameLength * 1000 / origFrequency); };
    int getOrigFrequency() const { return origFrequency; }
    
    ALuint getBuffer() const { return soundBuffer; }
    bool hasBuffer() const { return soundBuffer > 0; }
private:
    short handle = -1;
    std::string soundName;
    int64_t frameLength = 0;
    int flags = 0;
    int origFrequency = -1;

    ALuint soundBuffer = 0;
};