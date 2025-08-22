#include "CSoundFileMP3.h"
#include <unistd.h>
#include <android/log.h>
#include "CSoundPlayer.h"
#include "Macros.h"

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

CSoundFileMP3::CSoundFileMP3(int fd, int64_t startOffset, int64_t length) : CSoundFile(fd, startOffset, length) {
    int64_t endOffset;
    if (length == -1) {
        endOffset = lseek64(fd, 0, SEEK_END);
    } else {
        endOffset = startOffset + length;
    }
    datasource = { fd, startOffset, endOffset };
}
CSoundFileMP3::~CSoundFileMP3() {
    close();
}

static size_t readMp3(void* userData, void* dest, size_t bytesToRead) {
    CSoundFileMP3::DataSource* src = (CSoundFileMP3::DataSource*)userData;

    off64_t cursor = lseek64(src->fd, 0, SEEK_CUR);
    if (cursor + bytesToRead > src->endOffset) {
        bytesToRead = src->endOffset - cursor;
    }
    return read(src->fd, dest, bytesToRead);
}
static drmp3_bool32 seekMp3(void* userData, int offset, drmp3_seek_origin origin) {
    CSoundFileMP3::DataSource* src = (CSoundFileMP3::DataSource*)userData;

    switch (origin) {
    case DRMP3_SEEK_SET:
        offset += src->startOffset;
        break;
    case DRMP3_SEEK_END:
        offset = src->endOffset - offset;
        origin = DRMP3_SEEK_SET;
        break;
    }
    
    return lseek64(src->fd, offset, origin) >= 0;
}
static drmp3_bool32 tellMp3(void* userData, drmp3_int64* cursor) {
    CSoundFileMP3::DataSource* src = (CSoundFileMP3::DataSource*)userData;
    
    off64_t result = lseek64(src->fd, 0, SEEK_CUR);
    if (result < 0) return DRMP3_FALSE;
    
    *cursor = result;
    return DRMP3_TRUE;
}

bool CSoundFileMP3::load() {
    lseek64(fd, startOffset, SEEK_SET);

    if (!drmp3_init(&file, readMp3, seekMp3, tellMp3, nullptr, &datasource, nullptr)) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to open MP3 file");
        return false;
    }

    format = file.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    sampleRate = file.sampleRate;
    frameLength = (int64_t)drmp3_get_pcm_frame_count(&file);
    bytesPerFrame = file.channels * sizeof(short);
    return true;
}
int64_t CSoundFileMP3::read(void* dest, int64_t numFrames) {
    return (int64_t)drmp3_read_pcm_frames_s16(&file, (drmp3_uint64)numFrames, (drmp3_int16*)dest);
}
void CSoundFileMP3::seek(int64_t frame) {
    drmp3_seek_to_pcm_frame(&file, (drmp3_uint64)frame);
}
int64_t CSoundFileMP3::tell() {
    return (int64_t)file.currentPCMFrame;
}
void CSoundFileMP3::close() {
    drmp3_uninit(&file);
    ::close(fd);
}

bool CSoundFileMP3::verify(int fd, int64_t startOffset, int64_t length) {
    lseek64(fd, startOffset, SEEK_SET);

    int64_t endOffset;
    if (length == -1) {
        endOffset = lseek64(fd, 0, SEEK_END);
    } else {
        endOffset = startOffset + length;
    }
    DataSource tempDatasource = { fd, startOffset, endOffset };
    drmp3 tempFile;
    bool valid = drmp3_init(&tempFile, readMp3, seekMp3, tellMp3, nullptr, &tempDatasource, nullptr);
    drmp3_uninit(&tempFile);

    return valid;
}