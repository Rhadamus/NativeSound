#include "CSoundChannel.h"
#include <android/log.h>
#include "Macros.h"
#include "CSoundPlayer.h"

// Hidden extension AL_SOFT_source_panning, found at https://github.com/kcat/openal-soft/blob/master/alc/inprogext.h
#define AL_PANNING_ENABLED_SOFT 0x19EC
#define AL_PAN_SOFT             0x19ED

void CSoundChannel::init(CSoundPlayer* initPlayer) {
    player = initPlayer;

    ALenum error;

    alGenSources(1, &sourceID);
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to generate OpenAL source (error code %d)", error);
        return;
    }
    alSourcei(sourceID, AL_PANNING_ENABLED_SOFT, AL_TRUE);
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to enable panning in OpenAL source (error code %d)", error);
    }
}
void CSoundChannel::deinit() {
    if (sourceID != 0) alDeleteSources(1, &sourceID);
    if (streamBuffers[0] != 0 || streamBuffers[1] != 0) alDeleteBuffers(2, streamBuffers);
}

void CSoundChannel::start(CSound* sound, bool newUninterruptible) {
    if (sourceID == 0) return;

    alSourcei(sourceID, AL_BUFFER, 0);
    if ((sound->getFlags() & SNDF_PLAYFROMDISK) == 0) {
        if ((sound->getFlags() & SNDF_LOADONCALL) != 0 && !sound->getFile()->isLoadedInMemory()) {
            sound->getFile()->readFileToMemory();
        }
        if (!sound->getFile()->isLoadedInMemory()) return;
        currentSound = sound;

        if (numLoops > 0) {
            ALuint buffer = sound->getFile()->getFileBuffer();
            auto bufferQueue = std::unique_ptr<ALuint[]>(new ALuint[numLoops]);
            for (int i = 0; i < numLoops; i++) {
                bufferQueue[i] = buffer;
            }

            alSourceQueueBuffers(sourceID, numLoops, bufferQueue.get());
            alSourcei(sourceID, AL_LOOPING, AL_FALSE);
        } else {
            alSourcei(sourceID, AL_BUFFER, sound->getFile()->getFileBuffer());
            alSourcei(sourceID, AL_LOOPING, AL_TRUE);
        }
    } else {
        if (streamBuffers[0] == 0 || streamBuffers[1] == 0) {
            alGenBuffers(2, streamBuffers);
        }

        currentSound = sound;
        streamFileCursor = 0;
        streamCursor = 0;

        alSourcei(sourceID, AL_LOOPING, AL_FALSE);
        int bufNum = 0;
        for (ALuint buffer : streamBuffers) {
            int64_t result = fillStreamBuffer(buffer, streamBufferLength);
            if (result > 0) {
                bufNum++;
            } else if (result < 0) {
                __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not load stream sound with handle %d, error reading file (error code %lld)", sound->getHandle(), result);
                break;
            }
        }
        alSourceQueueBuffers(sourceID, bufNum, streamBuffers);
    }

    uninterruptible = newUninterruptible;
    // Pitch for new sound needs to be recalculated, in case it has a different origFrequency
    updateFrequency();

    paused = false;
    alSourceRewind(sourceID);
    alSourcePlay(sourceID);
}
bool CSoundChannel::stop(bool force) {
    if (sourceID == 0) return true;
    if (!isPlaying()) return true;
    if (uninterruptible && !force) return false;

    alSourceStop(sourceID);
    streamLock.lock();
    currentSound = nullptr;
    streamLock.unlock();
    return true;
}
void CSoundChannel::pause() {
    if (sourceID == 0 || !isPlaying()) return;

    alSourcePause(sourceID);
    paused = true;
}
void CSoundChannel::resume() {
    if (sourceID == 0 || !isPlaying()) return;

    if (!pausedApp) alSourcePlay(sourceID);
    paused = false;
}
void CSoundChannel::pauseApp() {
    if (sourceID == 0 || !isPlaying()) return;

    alSourcePause(sourceID);
    pausedApp = true;
}
void CSoundChannel::resumeApp() {
    if (sourceID == 0 || !isPlaying()) return;

    if (!paused) alSourcePlay(sourceID);
    pausedApp = false;
}

bool CSoundChannel::isPlaying() const {
    if (sourceID == 0) return false;

    ALint state;
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING || state == AL_PAUSED;
}
bool CSoundChannel::isPaused() const {
    return paused;
}

void CSoundChannel::setVolume(float newVolume) {
    volume = newVolume;
    updateVolume();
}
void CSoundChannel::setPan(float newPan) {
    pan = newPan;
    updatePan();
}
void CSoundChannel::setFrequency(int newFrequency) {
    frequency = newFrequency;
    updateFrequency();
}
int CSoundChannel::getFrequency() const {
    if (frequency > 0) {
        return frequency;
    } else if (isPlaying()) {
        return currentSound->getOrigFrequency();
    }
    return 0;
}
void CSoundChannel::setLoopCount(int newLoops) {
    numLoops = newLoops;
    loopsLeft = newLoops;
}
void CSoundChannel::setPosition(int newPosition) {
    if (!isPlaying()) return;

    newPosition = std::max(0, std::min(currentSound->getDuration(), newPosition));
    if ((currentSound->getFlags() & SNDF_PLAYFROMDISK) == 0) {
        alSourcei(sourceID, AL_SAMPLE_OFFSET, (int64_t)newPosition * (int64_t)currentSound->getOrigFrequency() / 1000);
    } else {
        std::lock_guard lock(streamLock);

        streamCursor = (int64_t)newPosition * (int64_t)currentSound->getOrigFrequency() / 1000;
        streamFileCursor = streamCursor;
        alSourceStop(sourceID);

        alSourcei(sourceID, AL_BUFFER, 0);
        int bufNum = 0;
        for (ALuint buffer : streamBuffers) {
            int64_t result = fillStreamBuffer(buffer, streamBufferLength);
            if (result > 0) {
                bufNum++;
            } else if (result < 0) {
                __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not set sound position of handle %d, error reading file (error code %lld)", currentSound->getHandle(), result);
                break;
            }
        }
        alSourceQueueBuffers(sourceID, bufNum, streamBuffers);

        alSourcePlay(sourceID);
    }
}
int CSoundChannel::getPosition() const {
    if (!isPlaying()) return 0;

    int frameOffset;
    alGetSourcei(sourceID, AL_SAMPLE_OFFSET, &frameOffset);
    if ((currentSound->getFlags() & SNDF_PLAYFROMDISK) == 0) {
        return (int)((int64_t)(frameOffset % currentSound->getFile()->getFrameLength()) * 1000 / currentSound->getOrigFrequency());
    } else {
        return (int)((int64_t)(streamCursor + frameOffset) * 1000 / currentSound->getOrigFrequency());
    }
}

void CSoundChannel::updateVolume() {
    if (sourceID == 0) return;
    alSourcef(sourceID, AL_GAIN, volume * player->getVolume());
}
void CSoundChannel::updatePan() {
    if (sourceID == 0) return;
    alSourcef(sourceID, AL_PAN_SOFT, pan + player->getPan());
}
void CSoundChannel::updateFrequency() {
    if (sourceID == 0) return;
    if (frequency > 0 && isPlaying()) {
        alSourcef(sourceID, AL_PITCH, (float)frequency / (float)currentSound->getOrigFrequency());
    } else {
        alSourcef(sourceID, AL_PITCH, 1.0f);
    }
}
void CSoundChannel::updateStream() {
    std::lock_guard lock(streamLock);

    if (!isPlaying() || (currentSound->getFlags() & SNDF_PLAYFROMDISK) == 0) return;
    CSoundFile* file = currentSound->getFile();
    if (file == nullptr) return;

    int processedBuffers;
    alGetSourcei(sourceID, AL_BUFFERS_PROCESSED, &processedBuffers);

    while (processedBuffers > 0) {
        ALuint removedBufferID;
        alSourceUnqueueBuffers(sourceID, 1, &removedBufferID);

        int bufferSize;
        alGetBufferi(removedBufferID, AL_SIZE, &bufferSize);
        streamCursor += bufferSize / file->getBytesPerFrame();
        streamCursor %= file->getFrameLength();

        int64_t result = fillStreamBuffer(removedBufferID, streamBufferLength);
        if (result >= 0 && result < streamBufferLength) {
            if (loopsLeft > 0) {
                loopsLeft--;
                streamFileCursor = 0;
                if (loopsLeft == 0) {
                    break;
                }
            } else if (numLoops == 0) {
                streamFileCursor = 0;
            }
        }
        if (result < 0) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not update stream with handle %d, error reading file (error code %lld)", currentSound->getHandle(), result);
            break;
        }

        alSourceQueueBuffers(sourceID, 1, &removedBufferID);
        processedBuffers--;
    }
}

CSound* CSoundChannel::getSound() const {
    if (!isPlaying()) return nullptr;
    return currentSound;
}

int64_t CSoundChannel::fillStreamBuffer(ALuint bufferID, int64_t numFrames) {
    CSoundFile* file = currentSound->getFile();
    if (file == nullptr) return 0;

    auto streamData = std::unique_ptr<char[]>(new char[streamBufferLength * file->getBytesPerFrame()]);
    numFrames = std::min(numFrames, streamBufferLength);

    file->seek(streamFileCursor);
    int64_t readResult = file->read(streamData.get(), numFrames);
    if (readResult <= 0) {
        return readResult;
    }

    streamFileCursor += readResult;
    alBufferData(bufferID, file->getFormat(), streamData.get(), readResult * file->getBytesPerFrame(), file->getSampleRate());
    return readResult;
}