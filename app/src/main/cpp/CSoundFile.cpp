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
    if (CSoundFileWAV::verify(fd, startOffset)) {
        result = new CSoundFileWAV(fd, startOffset, length);
    } else if (CSoundFileOGG::verify(fd, startOffset)) {
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

void CSoundFile::readFileToMemory() {
    if (loaded) return;

    auto fileData = std::unique_ptr<char[]>(new char[frameLength * bytesPerFrame]);

    int64_t resultRead = read(fileData.get(), frameLength);
    if (resultRead < 0) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read entire sound file (error code %lld)", resultRead);
        return;
    }

    ALenum error;

    alGenBuffers(1, &fileBuffer);
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to generate file buffer (error code %d)", error);
        return;
    }
    alBufferData(fileBuffer, format, fileData.get(), frameLength * bytesPerFrame, sampleRate);
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to upload data to file buffer (error code %d)", error);
        alDeleteBuffers(1, &fileBuffer);
        return;
    }

    loaded = true;
}

CSoundFile::CSoundFile(int fd, int64_t startOffset, int64_t length) : fd(fd), startOffset(startOffset), length(length) {}
void CSoundFile::cleanUp() {
    ALenum error;
    if (fileBuffer != 0) {
        alDeleteBuffers(1, &fileBuffer);
        if ((error = alGetError()) != AL_NO_ERROR) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to delete file buffer (error code %d)", error);
        }
    }
}