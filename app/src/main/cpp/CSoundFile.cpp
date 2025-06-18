#include "CSoundFile.h"
#include <assert.h>
#include <android/log.h>
#include "Macros.h"
#include "CSoundPlayer.h"
#include "CSoundFileWAV.h"
#include "CSoundFileOGG.h"
#include "CSoundFileMP3.h"

CSoundFile* CSoundFile::load(int fd, int64_t startOffset, int64_t length) {
    CSoundFile* result;
    if (CSoundFileWAV::verify(fd, startOffset, length)) {
        result = new CSoundFileWAV(fd, startOffset, length);
    } else if (CSoundFileOGG::verify(fd, startOffset, length)) {
        result = new CSoundFileOGG(fd, startOffset, length);
    } else if (CSoundFileMP3::verify(fd, startOffset, length)) {
        result = new CSoundFileMP3(fd, startOffset, length);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Unsupported file format");
        return nullptr;
    }

    if (!result->load()) {
        delete result;
        return nullptr;
    }
    return result;
}
CSoundFile::CSoundFile(int fd, int64_t startOffset, int64_t length) : fd(fd), startOffset(startOffset), length(length) {}