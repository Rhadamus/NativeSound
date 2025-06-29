#include "CSoundPlayer.h"
#include <assert.h>
#include <android/log.h>
#include "Macros.h"

CSoundPlayer::CSoundPlayer() {
    device = alcOpenDevice(nullptr);
    if (device == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not open OpenAL device");
        return;
    }

    context = alcCreateContext(device, nullptr);
    if (context == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not create OpenAL context (error code %d)", alcGetError(device));
        return;
    }
    if (alcMakeContextCurrent(context) != AL_TRUE) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not make OpenAL context current (error code %d)", alcGetError(device));
        return;
    }

    for (CSoundChannel& channel : channels) {
        channel.init(this);
    }

    streamThread = std::thread([this]{
        while (streamRun.load(std::memory_order_acquire)) {
            for (CSoundChannel& channel : channels) {
                channel.updateStream();
            }
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(5));
        }
    });
    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Initialized CSoundPlayer");
}
CSoundPlayer::~CSoundPlayer() {
    streamRun.store(false, std::memory_order_release);
    streamThread.join();
    for (CSoundChannel& channel : channels) {
        channel.deinit();
    }
    if (context != nullptr) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
    }
    if (device != nullptr) alcCloseDevice(device);
}

void CSoundPlayer::setVolume(float volume) {
    mainVolume = std::max(0.0f, std::min(1.0f, volume));
    for (CSoundChannel& channel : channels) {
        channel.updateVolume();
    }
}
void CSoundPlayer::setPan(float pan) {
    mainPan = std::max(-1.0f, std::min(1.0f, pan));
    for (CSoundChannel& channel : channels) {
        channel.updatePan();
    }
}

void CSoundPlayer::play(JNIEnv* jniEnv, jobject jniSound, CSound* sound, int nLoops, int channel,
                        bool prio, int volume, int pan, int freq, bool focus) {
    bool withChannel = channel != -1;

    if (!multipleSounds) channel = 0;

    if (channel < 0) {
        int i;
        for (i = 0; i < NCHANNELS; i++) {
            if (!channels[i].isPlaying() && !channels[i].locked) break;
        }
        if (i == NCHANNELS) {
            for (i = 0; i < NCHANNELS; i++) {
                if (!channels[i].locked && channels[i].stop(false)) break;
            }
        }
        channel = i;
    }

    if (channel >= NCHANNELS || !channels[channel].stop(prio)) return;

    channels[channel].setLoopCount(nLoops);
    if (volume != -1) {
        channels[channel].setVolume((float)volume / 100.0f);
        channels[channel].setPan((float)pan / 100.0f);
        channels[channel].setFrequency(freq);
    } else if (!withChannel) {
        channels[channel].setVolume(1.0f);
        channels[channel].setPan(0.0f);
        channels[channel].setFrequency(-1);
    }

    if (focus) channels[channel].start(jniEnv, jniSound, sound, prio);
}

void CSoundPlayer::stopAllSounds() {
    for (CSoundChannel& channel : channels) {
        channel.stop(true);
    }
}
void CSoundPlayer::pauseAllChannels() {
    for (CSoundChannel& channel : channels) {
        channel.pause();
    }
}
void CSoundPlayer::resumeAllChannels() {
    for (CSoundChannel& channel : channels) {
        channel.resume();
    }
}
void CSoundPlayer::pauseApp() {
    for (CSoundChannel& channel : channels) {
        channel.pauseApp();
    }
}
void CSoundPlayer::resumeApp() {
    for (CSoundChannel& channel : channels) {
        channel.resumeApp();
    }
}

bool CSoundPlayer::isSoundPlaying() const {
    for (const CSoundChannel& channel : channels) {
        if (channel.isPlaying()) return true;
    }
    return false;
}
bool CSoundPlayer::isSamplePlaying(short handle) const {
    for (const CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            return true;
        }
    }
    return false;
}
bool CSoundPlayer::isSamplePaused(short handle) const {
    for (const CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            if (channel.isPaused()) return true;
        }
    }
    return false;
}
bool CSoundPlayer::isChannelPlaying(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return false;
    return channels[channel].isPlaying();
}
bool CSoundPlayer::isChannelPaused(int channel) const {
    if (channel < 0 || channel >= NCHANNELS || !channels[channel].isPlaying()) return false;
    return channels[channel].isPaused();
}

void CSoundPlayer::stop(short handle) {
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.stop(true);
        }
    }
}
void CSoundPlayer::pause(short handle) {
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.pause();
        }
    }
}
void CSoundPlayer::resume(short handle) {
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.resume();
        }
    }
}
void CSoundPlayer::stopChannel(int channel) {
    if (channel < 0 || channel >= NCHANNELS) return;
    channels[channel].stop(true);
}
void CSoundPlayer::pauseChannel(int channel) {
    if (channel < 0 || channel >= NCHANNELS || !channels[channel].isPlaying()) return;
    channels[channel].pause();
}
void CSoundPlayer::resumeChannel(int channel) {
    if (channel < 0 || channel >= NCHANNELS || !channels[channel].isPlaying()) return;
    channels[channel].resume();
}

int CSoundPlayer::getSampleChannel(const std::string& name) const {
    for (int i = 0; i < NCHANNELS; i++) {
        if (channels[i].isPlaying() && channels[i].getSound()->getName() == name) {
            return i;
        }
    }
    return -1;
}
std::string CSoundPlayer::getChannelSampleName(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return "";
    if (channels[channel].isPlaying()) {
        return channels[channel].getSound()->getName();
    }
    return "";
}

int CSoundPlayer::getDurationSample(const std::string& name) const {
    int channel = getSampleChannel(name);
    if (channel < 0) return 0;
    return channels[channel].getSound()->getDuration();
}
void CSoundPlayer::setVolumeSample(short handle, float volume) {
    volume = std::max(0.0f, std::min(1.0f, volume));
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.setVolume(volume);
        }
    }
}
float CSoundPlayer::getVolumeSample(const std::string& name) const {
    int channel = getSampleChannel(name);
    if (channel < 0) return 0.0f;
    return channels[channel].getVolume();
}
void CSoundPlayer::setPanSample(short handle, float pan) {
    pan = std::max(-1.0f, std::min(1.0f, pan));
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.setPan(pan);
        }
    }
}
float CSoundPlayer::getPanSample(const std::string& name) const {
    int channel = getSampleChannel(name);
    if (channel < 0) return 0.0f;
    return channels[channel].getPan();
}
void CSoundPlayer::setFrequencySample(short handle, int freq) {
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.setFrequency(freq);
        }
    }
}
int CSoundPlayer::getFrequencySample(const std::string& name) const {
    int channel = getSampleChannel(name);
    if (channel < 0) return 0;
    return channels[channel].getFrequency();
}
void CSoundPlayer::setPositionSample(short handle, int position) {
    for (CSoundChannel& channel : channels) {
        if (channel.isPlaying() && channel.getSound()->getHandle() == handle) {
            channel.setPosition(position);
        }
    }
}
int CSoundPlayer::getPositionSample(const std::string& name) const {
    int channel = getSampleChannel(name);
    if (channel < 0) return 0;
    return channels[channel].getPosition();
}

int CSoundPlayer::getDurationChannel(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return 0;
    if (channels[channel].isPlaying()) {
        return channels[channel].getSound()->getDuration();
    }
    return 0;
}
void CSoundPlayer::setVolumeChannel(int channel, float volume) {
    if (channel < 0 || channel >= NCHANNELS) return;
    volume = std::max(0.0f, std::min(1.0f, volume));
    channels[channel].setVolume(volume);
}
float CSoundPlayer::getVolumeChannel(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return 0.0f;
    return channels[channel].getVolume();
}
void CSoundPlayer::setPanChannel(int channel, float pan) {
    if (channel < 0 || channel >= NCHANNELS) return;
    pan = std::max(-1.0f, std::min(1.0f, pan));
    channels[channel].setPan(pan);
}
float CSoundPlayer::getPanChannel(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return 0.0f;
    return channels[channel].getPan();
}
void CSoundPlayer::setFrequencyChannel(int channel, int freq) {
    if (channel < 0 || channel >= NCHANNELS) return;
    channels[channel].setFrequency(freq);
}
int CSoundPlayer::getFrequencyChannel(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return 0;
    return channels[channel].getFrequency();
}
void CSoundPlayer::setPositionChannel(int channel, int position) {
    if (channel < 0 || channel >= NCHANNELS) return;
    channels[channel].setPosition(position);
}
int CSoundPlayer::getPositionChannel(int channel) const {
    if (channel < 0 || channel >= NCHANNELS) return 0;
    return channels[channel].getPosition();
}

void CSoundPlayer::lockChannel(int channel) {
    if (channel < 0 || channel >= NCHANNELS) return;
    channels[channel].locked = true;
}
void CSoundPlayer::unlockChannel(int channel) {
    if (channel < 0 || channel >= NCHANNELS) return;
    channels[channel].locked = false;
}