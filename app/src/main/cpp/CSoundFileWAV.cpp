#include "CSoundFileWAV.h"
#include <unistd.h>
#include <android/log.h>
#include <AL/alext.h>
#include "CSoundPlayer.h"
#include "Macros.h"

#define WAVE_FORMAT_PCM         1
#define WAVE_FORMAT_IEEE_FLOAT  3

struct RIFFChunkHeader {
    char tag[4];
    uint32_t size;
};
struct WAVEFmtChunk {
    uint16_t format;
    uint16_t nChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

CSoundFileWAV::CSoundFileWAV(int fd, int64_t startOffset, int64_t length) : CSoundFile(fd, startOffset, length) {
    if (length == -1) {
        endOffset = lseek64(fd, 0, SEEK_END);
    } else {
        endOffset = startOffset + length;
    }
}
CSoundFileWAV::~CSoundFileWAV() { CSoundFileWAV::close(); }

bool CSoundFileWAV::load() {
    lseek64(fd, startOffset + 12, SEEK_SET);

    RIFFChunkHeader chunk;
    ssize_t result;
    while ((result = ::read(fd, &chunk, sizeof(chunk))) > 0) {
        if (strncmp(chunk.tag, "fmt ", sizeof(chunk.tag)) == 0) {
            WAVEFmtChunk fmtChunk;
            if (::read(fd, &fmtChunk, sizeof(fmtChunk)) == -1) {
                __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read fmt chunk (error code %d)", errno);
                return false;
            }

            if (fmtChunk.format == WAVE_FORMAT_PCM) {
                switch (fmtChunk.bitsPerSample) {
                case 8:     format = fmtChunk.nChannels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8; break;
                case 16:    format = fmtChunk.nChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16; break;
                default:
                    __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Unsupported format (format: %d, bits per sample: %d)", fmtChunk.format, fmtChunk.bitsPerSample);
                    return false;
                }
            } else if (fmtChunk.format == WAVE_FORMAT_IEEE_FLOAT && fmtChunk.bitsPerSample == 32) {
                format = fmtChunk.nChannels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            } else {
                __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Unsupported format (format: %d, bits per sample: %d)", fmtChunk.format, fmtChunk.bitsPerSample);
                return false;
            }
            sampleRate = (int)fmtChunk.sampleRate;
            bytesPerFrame = fmtChunk.nChannels * (fmtChunk.bitsPerSample / 8);
        } else if (strncmp(chunk.tag, "data", sizeof(chunk.tag)) == 0) {
            dataOffset = lseek64(fd, 0, SEEK_CUR);
            frameLength = chunk.size / bytesPerFrame;
            lseek64(fd, chunk.size, SEEK_CUR);
        } else {
            lseek64(fd, chunk.size, SEEK_CUR);
        }
    }

    if (result == -1) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Error reading RIFF chunk header (error code %d)", errno);
        return false;
    }
    lseek64(fd, dataOffset, SEEK_SET);
    return true;
}
int64_t CSoundFileWAV::read(void* dest, int64_t numFrames) {
    size_t byteCount = (size_t)(numFrames * bytesPerFrame);
    off64_t cursor = lseek64(fd, 0, SEEK_CUR);
    if (cursor + byteCount > endOffset) {
        byteCount = endOffset - cursor;
    }
    
    int64_t result = ::read(fd, dest, byteCount);
    if (result <= 0) return result;
    return result / bytesPerFrame;
}
void CSoundFileWAV::seek(int64_t frame) {
    if (frame < 0 || frame > frameLength) return;
    lseek64(fd, dataOffset + frame * bytesPerFrame, SEEK_SET);
}
int64_t CSoundFileWAV::tell() {
    return (lseek64(fd, 0, SEEK_CUR) - dataOffset) / bytesPerFrame;
}
void CSoundFileWAV::close() {
    cleanUp();
    ::close(fd);
}

bool CSoundFileWAV::verify(int fd, int64_t startOffset) {
    lseek64(fd, startOffset, SEEK_SET);

    RIFFChunkHeader riffChunk;
    if (::read(fd, &riffChunk, sizeof(riffChunk)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read RIFF header (error code %d)", errno);
        return false;
    }
    if (strncmp(riffChunk.tag, "RIFF", sizeof(riffChunk.tag)) != 0) return false;

    char waveTag[4];
    if (::read(fd, waveTag, sizeof(waveTag)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read WAVE tag (error code %d)", errno);
        return false;
    }
    if (strncmp(waveTag, "WAVE", sizeof(waveTag)) != 0) return false;

    return true;
}