#include "CSoundFileOGG.h"
#include <unistd.h>
#include <android/log.h>
#include "CSoundPlayer.h"
#include "Macros.h"

CSoundFileOGG::CSoundFileOGG(int fd, int64_t startOffset, int64_t length) : CSoundFile(fd, startOffset, length) {
    int64_t endOffset;
    if (length == -1) {
        endOffset = lseek64(fd, 0, SEEK_END);
    } else {
        endOffset = startOffset + length;
    }
    datasource = { fd, startOffset, endOffset };
}
CSoundFileOGG::~CSoundFileOGG() {
    close();
}

static size_t readOgg(void* dest, size_t size, size_t count, void* datasource) {
    CSoundFileOGG::DataSource* src = (CSoundFileOGG::DataSource*)datasource;

    size_t byteCount = count * size;
    off64_t cursor = lseek64(src->fd, 0, SEEK_CUR);
    if (cursor + byteCount > src->endOffset) {
        byteCount = src->endOffset - cursor;
        byteCount -= byteCount % size;
    }

    ssize_t result = read(src->fd, dest, byteCount);
    if (result <= 0) return 0;
    return result / size;
}
static int seekOgg(void* datasource, ogg_int64_t offset, int whence) {
    CSoundFileOGG::DataSource* src = (CSoundFileOGG::DataSource*)datasource;

    switch (whence) {
    case SEEK_SET:
        offset += src->startOffset;
        break;
    case SEEK_END:
        offset = src->endOffset - offset;
        whence = SEEK_SET;
        break;
    }

    off64_t result = lseek64(src->fd, offset, whence);
    if (result < 0) return -1;
    return 0;
}
static int closeOgg(void* datasource) {
    CSoundFileOGG::DataSource* src = (CSoundFileOGG::DataSource*)datasource;
    return close(src->fd);
}
static long tellOgg(void* datasource) {
    CSoundFileOGG::DataSource* src = (CSoundFileOGG::DataSource*)datasource;
    return lseek64(src->fd, 0, SEEK_CUR) - src->startOffset;
}

bool CSoundFileOGG::load() {
    lseek64(fd, startOffset, SEEK_SET);

    ov_callbacks oggCallbacks;
    oggCallbacks.read_func = readOgg;
    oggCallbacks.seek_func = seekOgg;
    oggCallbacks.close_func = closeOgg;
    oggCallbacks.tell_func = tellOgg;

    int result;
    if ((result = ov_open_callbacks(&datasource, &file, nullptr, 0, oggCallbacks)) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to open Ogg Vorbis file (error code %d)", result);
        return false;
    }

    vorbis_info* soundInfo = ov_info(&file, -1);
    if (soundInfo == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to retrieve Ogg info");
        return false;
    }

    format = soundInfo->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    sampleRate = soundInfo->rate;
    frameLength = ov_pcm_total(&file, -1);
    bytesPerFrame = soundInfo->channels * sizeof(short);
    return true;
}
int64_t CSoundFileOGG::read(void* dest, int64_t numFrames) {
    int64_t bytesRead = 0;
    while (bytesRead < numFrames * bytesPerFrame) {
        long result = ov_read(&file, (char*)dest + bytesRead, numFrames * bytesPerFrame - bytesRead, 0, 2, 1, &curBitstream);
        if (result < 0) return result;
        else if (result == 0) break;
        bytesRead += result;
    }
    return bytesRead / bytesPerFrame;
}
void CSoundFileOGG::seek(int64_t frame) {
    ov_pcm_seek(&file, frame);
}
int64_t CSoundFileOGG::tell() {
    return ov_pcm_tell(&file);
}
void CSoundFileOGG::close() {
    ov_clear(&file);
}

bool CSoundFileOGG::verify(int fd, int64_t startOffset, int64_t length) {
    lseek64(fd, startOffset, SEEK_SET);

    char magic[4];
    if (::read(fd, magic, sizeof(magic)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read Ogg magic number (error code %d)", errno);
        return false;
    }
    if (strncmp(magic, "OggS", sizeof(magic)) != 0) return false;

    return true;
}