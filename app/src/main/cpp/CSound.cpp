#include "CSound.h"
#include <android/log.h>
#include "Macros.h"

CSound::CSound(const std::string& name, short handle, int frequency, int flags) :
    soundName(name), handle(handle), origFrequency(frequency), flags(flags) {}
CSound::CSound() :
    flags(SNDF_PLAYFROMDISK) {}
CSound::~CSound() {
    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Unloading sound %d", (int)handle);
    if (hasBuffer()) {
        alDeleteBuffers(1, &soundBuffer);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to delete sound buffer (error code %d)", error);
        }
    }
}

void CSound::load(JNIEnv* jniEnv, jobject jniSound) {
    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Loading sound with handle %d and flags 0x%04X", (int)handle, flags);
    
    {
        auto file = std::unique_ptr<CSoundFile>(openFile(jniEnv, jniSound));
        if (file == nullptr) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not load sound %d", (int)handle);
            return;
        }
        frameLength = file->getFrameLength();
    }

    if ((flags & SNDF_LOADONCALL) == 0 && (flags & SNDF_PLAYFROMDISK) == 0) {
        loadBuffer(jniEnv, jniSound);
    }

    __android_log_print(ANDROID_LOG_INFO, NATIVESOUND_TAG, "Success");
}
CSoundFile* CSound::openFile(JNIEnv* jniEnv, jobject jniSound) const {
    jlongArray jniFdInfo = nullptr;
    
    if (handle > -1) {
        jclass soundClazz = jniEnv->GetObjectClass(jniSound);
        jmethodID jniOpenFd = jniEnv->GetMethodID(soundClazz, "openFd", "(S)[J");
        if (jniOpenFd == nullptr) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not find CSound.openFd(short) method");
            return nullptr;
        }
        
        jniFdInfo = (jlongArray)jniEnv->CallObjectMethod(jniSound, jniOpenFd, (jshort)handle);
    } else {
        jclass soundClazz = jniEnv->GetObjectClass(jniSound);
        jmethodID jniOpenFd = jniEnv->GetMethodID(soundClazz, "openFd", "()[J");
        if (jniOpenFd == nullptr) {
            __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not call CSound.openFd() method");
            return nullptr;
        }
        
        jniFdInfo = (jlongArray)jniEnv->CallObjectMethod(jniSound, jniOpenFd);
    }
    if (jniFdInfo == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to get fd info of sound %d", (int)handle);
        return nullptr;
    }
    
    jlong* fdInfo = jniEnv->GetLongArrayElements(jniFdInfo, nullptr);
    if (fdInfo == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to get fd info of sound %d", (int)handle);
        return nullptr;
    }
    CSoundFile* file = CSoundFile::open((int)fdInfo[0], (int64_t)fdInfo[1], (int64_t)fdInfo[2]);
    jniEnv->ReleaseLongArrayElements(jniFdInfo, fdInfo, 0);
    
    return file;
}
void CSound::loadBuffer(JNIEnv* jniEnv, jobject jniSound) {
    if (hasBuffer()) return;
    
    auto file = std::unique_ptr<CSoundFile>(openFile(jniEnv, jniSound));
    if (file == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Could not open file for sound %d", (int)handle);
        return;
    }
    auto fileData = std::unique_ptr<char[]>(new char[file->getFrameLength() * file->getBytesPerFrame()]);

    int64_t resultRead = file->read(fileData.get(), file->getFrameLength());
    if (resultRead < 0) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to read entire sound file (error code %lld)", (long long)resultRead);
        return;
    }

    ALenum error;

    alGenBuffers(1, &soundBuffer);
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to generate sound buffer (error code %d)", error);
        soundBuffer = 0;
        return;
    }
    alBufferData(soundBuffer, file->getFormat(), fileData.get(), file->getFrameLength() * file->getBytesPerFrame(), file->getSampleRate());
    if ((error = alGetError()) != AL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, NATIVESOUND_TAG, "Failed to upload data to sound buffer (error code %d)", error);
        alDeleteBuffers(1, &soundBuffer);
        soundBuffer = 0;
        return;
    }
}