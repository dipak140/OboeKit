//
// Created by Dipak Sisodiya on 21/03/25.
//

#include <jni.h>
#include <memory>
#include <android/log.h>
#include "KaraokePlayerEngine.h"
#include <stream/MemInputStream.h>

static const char* TAG = "KaraokePlayerJNIBridge";

static std::unique_ptr<iolib::KaraokePlayerEngine> pKaraokePlayerEngine;

inline iolib::KaraokePlayerEngine* ensureEngine() {
    if (!pKaraokePlayerEngine) {
        pKaraokePlayerEngine = std::make_unique<iolib::KaraokePlayerEngine>();
    }
    return pKaraokePlayerEngine.get();
}

extern "C" {

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_createPlayer(JNIEnv*, jobject) {
    return ensureEngine() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_setupAudioStreamNative(JNIEnv*, jobject, jint numChannels) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "setupAudioStreamNative()");
    ensureEngine()->setupAudioStream(numChannels);
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_startAudioStreamNative(JNIEnv*, jobject) {
    ensureEngine()->startStream();
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_teardownAudioStreamNative(JNIEnv*, jobject) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "teardownAudioStreamNative()");
    if(pKaraokePlayerEngine){
        pKaraokePlayerEngine->teardownAudioStream();
    }
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_loadWavAssetNative(JNIEnv* env, jobject, jbyteArray bytearray, jint index, jfloat pan) {
    auto* engine = ensureEngine();
    int len = env->GetArrayLength (bytearray);
    unsigned char* buf = new unsigned char[len];
    env->GetByteArrayRegion (bytearray, 0, len, reinterpret_cast<jbyte*>(buf));

    parselib::MemInputStream stream(buf, len);

    parselib::WavStreamReader reader(&stream);
    reader.parse();

    reader.getNumChannels();

    iolib::SampleBuffer* sampleBuffer = new iolib::SampleBuffer();
    sampleBuffer->loadSampleData(&reader);

    iolib::OneShotSampleSource* source = new iolib::OneShotSampleSource(sampleBuffer, pan);
    engine->addSampleSource(source, sampleBuffer);

    delete[] buf;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_unloadWavAssetsNative(JNIEnv*, jobject) {
    ensureEngine()->unloadSampleData();
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_seekToPosition(JNIEnv*, jobject, jlong position, jint sampleRate, jint numChannels) {
    ensureEngine()->seekTo(static_cast<int64_t>(position), sampleRate, numChannels);
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_trigger(JNIEnv*, jobject, jint index) {
    ensureEngine()->triggerDown(index);
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_stopTrigger(JNIEnv*, jobject, jint index) {
    if (pKaraokePlayerEngine) {
        pKaraokePlayerEngine->triggerUp(index);
    }
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_pauseTrigger(JNIEnv*, jobject) {
    ensureEngine()->pauseStream();
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_resumeTrigger(JNIEnv*, jobject) {
    ensureEngine()->resumeStream();
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getOutputReset(JNIEnv*, jobject) {
    return ensureEngine()->getOutputReset();
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_clearOutputReset(JNIEnv*, jobject) {
    ensureEngine()->clearOutputReset();
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_restartStream(JNIEnv*, jobject) {
    auto* engine = ensureEngine();
    engine->resetAll();
    if (engine->openStream() && engine->startStream()) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "openStream successful");
    } else {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "openStream failed");
    }
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_setPan(JNIEnv*, jobject, jint index, jfloat pan) {
    ensureEngine()->setPan(index, pan);
}

JNIEXPORT jfloat JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getPan(JNIEnv*, jobject, jint index) {
    return ensureEngine()->getPan(index);
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_setGain(JNIEnv*, jobject, jint index, jfloat gain) {
    if (pKaraokePlayerEngine) {
        pKaraokePlayerEngine->setGain(index, gain);
    }
}

JNIEXPORT jlong JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getPosition(JNIEnv*, jobject, jint index, jint sampleRate, jint numChannels) {
    return pKaraokePlayerEngine ? pKaraokePlayerEngine->getPosition(index, sampleRate, numChannels) : 0;
}

JNIEXPORT jfloat JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getGain(JNIEnv*, jobject, jint index) {
    return ensureEngine()->getGain(index);
}

JNIEXPORT jlong JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getDurationNative(JNIEnv*, jobject, jint sampleRate, jint numChannels) {
    return pKaraokePlayerEngine ? pKaraokePlayerEngine->getDuration(sampleRate, numChannels) : 0;
}

JNIEXPORT jlong JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getMusicPlayerTimeStamp(JNIEnv*, jobject) {
    return ensureEngine()->getFrameTimeStamp();
}

JNIEXPORT jlong JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getMusicPlayerFramePosition(JNIEnv*, jobject) {
    return ensureEngine()->getFramePosition();
}

JNIEXPORT jint JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_getPlayerAudioSessionId(JNIEnv*, jobject) {
    return ensureEngine()->getAudioSessionId();
}

} // extern "C"

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_KaraokePlayerNativeLib_deletePlayer(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "deletePlayer()");
    if (pKaraokePlayerEngine) {
        pKaraokePlayerEngine = nullptr;
    }
}