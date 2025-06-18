#include "CSound.h"
#include <android/log.h>
#include "Macros.h"

CSound::CSound(const std::string& name, short handle, int frequency, int flags) :
    soundName(name), handle(handle), origFrequency(frequency), flags(flags) {}
CSound::CSound() :
    flags(SNDF_PLAYFROMDISK) {}
CSound::~CSound() {
    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Unloading sound %d", (int)handle);
    if (hasBuffer()) {
        alDeleteBuffers(1, &soundBuffer);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to delete sound buffer (error code %d)", error);
        }
    }
    delete file;
}

void CSound::load(int fd, int64_t startOffset, int64_t length) {
    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Loading sound with handle %d and flags 0x%04X", (int)handle, flags);
    file = CSoundFile::load(fd, startOffset, length);
    if (file == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not load sound %d", (int)handle);
        return;
    }

    if ((flags & SNDF_LOADONCALL) == 0 && (flags & SNDF_PLAYFROMDISK) == 0) {
        loadBuffer();
    }

    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Success");
}

int CSound::getDuration() const {
    return (int)(file->getFrameLength() * 1000 / origFrequency);
}

void CSound::loadBuffer() {
    if (hasBuffer()) return;

    auto fileData = std::unique_ptr<char[]>(new char[file->getFrameLength() * file->getBytesPerFrame()]);

    int64_t resultRead = file->read(fileData.get(), file->getFrameLength());
    if (resultRead < 0) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read entire sound file (error code %lld)", (long long)resultRead);
        return;
    }

    ALenum error;

    alGenBuffers(1, &soundBuffer);
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to generate sound buffer (error code %d)", error);
        soundBuffer = 0;
        return;
    }
    alBufferData(soundBuffer, file->getFormat(), fileData.get(), file->getFrameLength() * file->getBytesPerFrame(), file->getSampleRate());
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to upload data to sound buffer (error code %d)", error);
        alDeleteBuffers(1, &soundBuffer);
        soundBuffer = 0;
        return;
    }
}