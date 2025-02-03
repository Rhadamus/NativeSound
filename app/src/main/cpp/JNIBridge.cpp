#include <jni.h>
#include "CSoundPlayer.h"

static CSoundPlayer* getSoundPlayerPtr(JNIEnv* env, jobject obj);
static CSound* getSoundPtr(JNIEnv* env, jobject obj);

// *************************************************************************************************
// * CSOUNDPLAYER INTERFACE                                                                        *
// *************************************************************************************************

static jfieldID soundPlayerPtrField = nullptr;

static CSoundPlayer* getSoundPlayerPtr(JNIEnv* env, jobject obj) {
    if (soundPlayerPtrField == nullptr) {
        jclass soundPlayerClazz = env->GetObjectClass(obj);
        soundPlayerPtrField = env->GetFieldID(soundPlayerClazz, "ptr", "J");
    }
    return (CSoundPlayer*)env->GetLongField(obj, soundPlayerPtrField);
}

extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat volume) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setVolume(volume);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_Application_CSoundPlayer_getVolume(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0.0f;
    return ptr->getVolume();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setPan(JNIEnv *env, jobject thiz, jfloat pan) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setPan(pan);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_Application_CSoundPlayer_getPan(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0.0f;
    return ptr->getPan();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_allocNative(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    delete ptr;
    ptr = new CSoundPlayer();
    env->SetLongField(thiz, soundPlayerPtrField, (jlong)ptr);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_playNative(JNIEnv *env, jobject thiz, jobject sound, jint n_loops,
                                   jint channel, jboolean prio, jint volume, jint pan, jint freq,
                                   jboolean focus) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    CSound* soundPtr = getSoundPtr(env, sound);
    if (soundPtr == nullptr) return;
    ptr->play(soundPtr, n_loops, channel, prio, volume, pan, freq, focus);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setMultipleSounds(JNIEnv *env, jobject thiz, jboolean b_multiple) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->multipleSounds = b_multiple;
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_stopAllSounds(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->stopAllSounds();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_stop(JNIEnv *env, jobject thiz, jshort handle) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->stop(handle);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_Application_CSoundPlayer_isSoundPlaying(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return false;
    return ptr->isSoundPlaying();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_Application_CSoundPlayer_isSamplePlaying(JNIEnv *env, jobject thiz, jshort handle) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return false;
    return ptr->isSamplePlaying(handle);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_Application_CSoundPlayer_isSamplePaused(JNIEnv *env, jobject thiz, jshort handle) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return false;
    return ptr->isSamplePaused(handle);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_Application_CSoundPlayer_isChannelPlaying(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return false;
    return ptr->isChannelPlaying(channel);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_Application_CSoundPlayer_isChannelPaused(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return false;
    return ptr->isChannelPaused(channel);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_pause(JNIEnv *env, jobject thiz, jshort handle) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->pause(handle);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_resumeNative(JNIEnv *env, jobject thiz, jshort handle) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->resume(handle);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_pauseApp(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->pauseApp();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_pauseAllChannels(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->pauseAllChannels();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_resumeApp(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->resumeApp();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_resumeAllChannelsNative(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->resumeAllChannels();
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_pauseChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->pauseChannel(channel);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_stopChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->stopChannel(channel);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_resumeChannelNative(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->resumeChannel(channel);
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getChannel(JNIEnv *env, jobject thiz, jstring name) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return -1;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    int channel = ptr->getSampleChannel(nameUtf);
    env->ReleaseStringUTFChars(name, nameUtf);
    return channel;
}
extern "C"
JNIEXPORT jstring JNICALL
Java_Application_CSoundPlayer_getChannelSampleName(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return env->NewStringUTF("");
    return env->NewStringUTF(ptr->getChannelSampleName(channel).c_str());
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getChannelDuration(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0;
    return ptr->getDurationChannel(channel);
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getSampleDuration(JNIEnv *env, jobject thiz, jstring name) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    int duration = ptr->getDurationSample(nameUtf);
    env->ReleaseStringUTFChars(name, nameUtf);
    return duration;
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setPositionChannel(JNIEnv *env, jobject thiz, jint channel,
                                                 jint pos) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setPositionChannel(channel, pos);
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getPositionChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0;
    return ptr->getPositionChannel(channel);
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getSamplePosition(JNIEnv *env, jobject thiz, jstring name) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    int position = ptr->getPositionSample(nameUtf);
    env->ReleaseStringUTFChars(name, nameUtf);
    return position;
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setFrequencyChannel(JNIEnv *env, jobject thiz, jint channel,
                                                  jint frequency) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setFrequencyChannel(channel, frequency);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setVolumeChannel(JNIEnv *env, jobject thiz, jint channel,
                                               jfloat volume) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setVolumeChannel(channel, volume);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_Application_CSoundPlayer_getVolumeChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0.0f;
    return ptr->getVolumeChannel(channel);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_Application_CSoundPlayer_getSampleVolume(JNIEnv *env, jobject thiz, jstring name) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0.0f;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    float volume = ptr->getVolumeSample(nameUtf);
    env->ReleaseStringUTFChars(name, nameUtf);
    return volume;
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setPanChannel(JNIEnv *env, jobject thiz, jint channel, jfloat pan) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setPanChannel(channel, pan);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_Application_CSoundPlayer_getPanChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0.0f;
    return ptr->getPanChannel(channel);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_Application_CSoundPlayer_getSamplePan(JNIEnv *env, jobject thiz, jstring name) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0.0f;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    float pan = ptr->getPanSample(nameUtf);
    env->ReleaseStringUTFChars(name, nameUtf);
    return pan;
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setPosition(JNIEnv *env, jobject thiz, jshort handle, jint pos) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setPositionSample(handle, pos);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setVolumeSample(JNIEnv *env, jobject thiz, jshort handle,
                                              jfloat volume) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setVolumeSample(handle, volume);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setFrequency(JNIEnv *env, jobject thiz, jshort handle,
                                           jint frequency) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setFrequencySample(handle, frequency);
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getFrequency(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0;
    return ptr->getFrequencyChannel(channel);
}
extern "C"
JNIEXPORT jint JNICALL
Java_Application_CSoundPlayer_getSampleFrequency(JNIEnv *env, jobject thiz, jstring name) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return 0;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    int freq = ptr->getFrequencySample(nameUtf);
    env->ReleaseStringUTFChars(name, nameUtf);
    return freq;
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_setPanSample(JNIEnv *env, jobject thiz, jshort handle, jfloat pan) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->setPanSample(handle, pan);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_lockChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->lockChannel(channel);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_unlockChannel(JNIEnv *env, jobject thiz, jint channel) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->unlockChannel(channel);
}
extern "C"
JNIEXPORT void JNICALL
Java_Application_CSoundPlayer_releaseNative(JNIEnv *env, jobject thiz) {
    CSoundPlayer* ptr = getSoundPlayerPtr(env, thiz);
    if (ptr == nullptr) return;
    delete ptr;
    env->SetLongField(thiz, soundPlayerPtrField, 0);
}

// *************************************************************************************************
// * CSOUND INTERFACE                                                                              *
// *************************************************************************************************

static jfieldID soundPtrField = nullptr;

static CSound* getSoundPtr(JNIEnv* env, jobject obj) {
    if (soundPtrField == nullptr) {
        jclass soundClazz = env->GetObjectClass(obj);
        soundPtrField = env->GetFieldID(soundClazz, "ptr", "J");
    }
    return (CSound*)env->GetLongField(obj, soundPtrField);
}

extern "C"
JNIEXPORT void JNICALL
Java_Banks_CSound_allocNative1(JNIEnv *env, jobject thiz, jstring name,
                               jshort handle, jint frequency, jint flags) {
    CSound* ptr = getSoundPtr(env, thiz);
    delete ptr;
    const char* nameUtf = env->GetStringUTFChars(name, nullptr);
    ptr = new CSound(nameUtf, handle, frequency, flags);
    env->ReleaseStringUTFChars(name, nameUtf);
    env->SetLongField(thiz, soundPtrField, (jlong)ptr);
}
extern "C"
JNIEXPORT void JNICALL
Java_Banks_CSound_allocNative2(JNIEnv *env, jobject thiz) {
    CSound* ptr = getSoundPtr(env, thiz);
    delete ptr;
    ptr = new CSound();
    env->SetLongField(thiz, soundPtrField, (jlong)ptr);
}
extern "C"
JNIEXPORT void JNICALL
Java_Banks_CSound_loadNative(JNIEnv *env, jobject thiz, jint fd, jlong startOffset, jlong length) {
    CSound* ptr = getSoundPtr(env, thiz);
    if (ptr == nullptr) return;
    ptr->load(fd, startOffset, length);
}
extern "C"
JNIEXPORT void JNICALL
Java_Banks_CSound_release(JNIEnv *env, jobject thiz) {
    CSound* ptr = getSoundPtr(env, thiz);
    if (ptr == nullptr) return;
    delete ptr;
    env->SetLongField(thiz, soundPtrField, 0);
}