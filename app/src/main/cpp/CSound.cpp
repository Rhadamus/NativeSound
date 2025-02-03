#include "CSound.h"
#include <android/log.h>
#include "Macros.h"

CSound::CSound(const std::string& name, short handle, int frequency, int flags) :
    soundName(name), handle(handle), origFrequency(frequency), flags(flags) {}
CSound::CSound() :
    flags(SNDF_PLAYFROMDISK) {}
CSound::~CSound() {
    delete file;
}

void CSound::load(int fd, int64_t startOffset, int64_t length) {
    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Loading sound with handle %d and flags 0x%04X", handle, flags);
    file = CSoundFile::load(fd, startOffset, length);
    if (file == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not load sound %d", handle);
        return;
    }

    if ((flags & SNDF_LOADONCALL) == 0 && (flags & SNDF_PLAYFROMDISK) == 0) {
        file->readFileToMemory();
    }

    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Success");
}

int CSound::getDuration() const {
    return (int)(file->getFrameLength() * 1000 / origFrequency);
}