#pragma once
#include <thread>
#include <atomic>
#include <jni.h>
#include <AL/alc.h>
#include "CSoundChannel.h"

#define NCHANNELS           48

class CSoundPlayer {
public:
    bool multipleSounds = true;

    CSoundPlayer();
    ~CSoundPlayer();

    void setVolume(float);
    float getVolume() const { return mainVolume; }
    void setPan(float);
    float getPan() const { return mainPan; }

    void play(JNIEnv*, jobject jniSound, CSound*, int nLoops, int channel, bool prio, int volume, int pan, int freq, bool focus);

    void stopAllSounds();
    void pauseAllChannels();
    void resumeAllChannels();
    void pauseApp();
    void resumeApp();

    bool isSoundPlaying() const;
    bool isSamplePlaying(short) const;
    bool isSamplePaused(short) const;
    bool isChannelPlaying(int) const;
    bool isChannelPaused(int) const;

    void stop(short handle);
    void pause(short handle);
    void resume(short handle);
    void stopChannel(int);
    void pauseChannel(int);
    void resumeChannel(int);

    int getSampleChannel(const std::string& name) const;
    std::string getChannelSampleName(int) const;

    int getDurationSample(const std::string&) const;
    void setVolumeSample(short handle, float volume);
    float getVolumeSample(const std::string&) const;
    void setPanSample(short handle, float pan);
    float getPanSample(const std::string&) const;
    void setFrequencySample(short handle, int freq);
    int getFrequencySample(const std::string&) const;
    void setPositionSample(short handle, int position);
    int getPositionSample(const std::string&) const;

    int getDurationChannel(int) const;
    void setVolumeChannel(int channel, float volume);
    float getVolumeChannel(int) const;
    void setPanChannel(int channel, float pan);
    float getPanChannel(int) const;
    void setFrequencyChannel(int channel, int freq);
    int getFrequencyChannel(int) const;
    void setPositionChannel(int channel, int position);
    int getPositionChannel(int) const;

    void lockChannel(int);
    void unlockChannel(int);
private:
    ALCdevice* device = nullptr;
    ALCcontext* context = nullptr;

    std::thread streamThread;
    std::atomic<bool> streamRun = true;

    CSoundChannel channels[NCHANNELS];
    float mainVolume = 1.0f, mainPan = 0.0f;
};