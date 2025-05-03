#include <jni.h>
#include <string>
#include <android/log.h>
#include "RecordingEngine.h"

#include <memory>
static std::unique_ptr<RecordingEngine> recordingEngine = std::make_unique<RecordingEngine>();

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_startRecording(JNIEnv *env, jobject thiz,
                                                               jstring full_path_tofile,
                                                               jint input_preset_preference,
                                                               jlong start_recording_time) {
    oboe::InputPreset inputPreset;
    switch (input_preset_preference) {
        case 10:
            inputPreset = oboe::InputPreset::VoicePerformance;
            break;
        case 9:
            inputPreset = oboe::InputPreset::Unprocessed;
            break;
        default:
            return;
    }

    const char *path = (*env).GetStringUTFChars(full_path_tofile, 0);
    if (!path) {
        return;
    }

    if (!recordingEngine) {
        recordingEngine = std::make_unique<RecordingEngine>(); // Reinitialize if deleted
    }

    recordingEngine->startRecording(path, inputPreset, start_recording_time);
    env->ReleaseStringUTFChars(full_path_tofile, path); // Release memory
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_stopRecording(JNIEnv *env, jobject thiz) {
    if (recordingEngine != nullptr) {
        recordingEngine->stopRecording();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_resumeRecording(JNIEnv *env, jobject thiz) {
    if (recordingEngine != nullptr) {
        recordingEngine->resumeRecording();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_pauseRecording(JNIEnv *env, jobject thiz) {
    if (recordingEngine != nullptr) {
        recordingEngine->pauseRecording();
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_getRecorderTimeStamp(JNIEnv *env, jobject thiz) {
    return recordingEngine->getFrameTimeStamp();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_getRecorderFramePosition(JNIEnv *env, jobject thiz) {
    if (recordingEngine != nullptr) {
        return recordingEngine->getFramePosition();
    }
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_getRecorderAudioSessionId(JNIEnv *env, jobject thiz) {
    if (recordingEngine != nullptr) {
        return recordingEngine->getAudioSessionId();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_RecordingNativeLib_releaseRecordingEngine(JNIEnv *env, jobject thiz) {
    recordingEngine.reset(); // Properly deletes and deallocates memory
}
