#pragma once
#include <mutex>
#include <AL/al.h>
#include "CSound.h"

class CSoundPlayer;

class CSoundChannel {
public:
    bool locked = false;

    void init(CSoundPlayer*);
    void deinit();

    void start(CSound*, bool uninterruptible);
    bool stop(bool force);
    void pause();
    void resume();
    void pauseApp();
    void resumeApp();

    bool isPlaying() const;
    bool isPaused() const;

    void setVolume(float);
    float getVolume() const { return volume; }
    void setPan(float);
    float getPan() const { return pan; }
    void setFrequency(int);
    int getFrequency() const;
    void setLoopCount(int);
    int getLoopCount() const { return numLoops; }
    void setPosition(int);
    int getPosition() const;

    void updateVolume();
    void updatePan();
    void updateFrequency();
    void updateStream();

    CSound* getSound() const;
private:
    CSoundPlayer* player = nullptr;
    ALuint sourceID = 0;

    CSound* currentSound = nullptr;

    static constexpr int64_t streamBufferLength = 16 * 1024;    // In frames
    ALuint streamBuffers[2] = {};
    int64_t streamFileCursor = 0;
    int64_t streamCursor = 0;
    std::mutex streamLock;

    float volume = 1.0f, pan = 0.0f;
    int frequency = -1;
    int numLoops = 1;
    int loopsLeft = 0;

    bool paused = true, pausedApp = false;
    bool uninterruptible = false;

    // Returns number of frames actually filled.
    int64_t fillStreamBuffer(ALuint bufferID, int64_t numFrames);
};