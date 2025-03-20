#include <jni.h>

//
// Created by Dipak Sisodiya on 20/03/25.
//

#include <jni.h>
#include <cassert>
#include <string>
#include <functional>
#include <utility>
#include <vector>
#include "FunctionList.h"
#include "../utils/logging_macros.h"
#include "../utils/effects/Effects.h"
#include "EarbackEffectEngine.h"

static const int kOboeApiAAudio = 0;
static const int kOboeApiOpenSLES = 1;

static EarbackEffectEngine *engine = nullptr;
#define LOG_TAG "NativeFXLab"

extern "C" {

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_create(JNIEnv *env,
                                                                jclass) {
    if (engine == nullptr) {
        engine = new EarbackEffectEngine();
    }

    return (engine != nullptr) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_delete(JNIEnv *env,
                                                                jclass) {
    if (engine) {
        engine->setEffectOn(false);
        delete engine;
        engine = nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_setEffectOn(
        JNIEnv *env, jclass, jboolean isEffectOn) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine before calling this "
                "method");
        return JNI_FALSE;
    }

    return engine->setEffectOn(isEffectOn) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_setRecordingDeviceId(
        JNIEnv *env, jclass, jint deviceId) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine before calling this "
                "method");
        return;
    }

    engine->setRecordingDeviceId(deviceId);
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_setPlaybackDeviceId(
        JNIEnv *env, jclass, jint deviceId) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine before calling this "
                "method");
        return;
    }

    engine->setPlaybackDeviceId(deviceId);
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_setAPI(JNIEnv *env,
                                                                jclass type,
                                                                jint apiType) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine "
                "before calling this method");
        return JNI_FALSE;
    }

    oboe::AudioApi audioApi;
    switch (apiType) {
        case kOboeApiAAudio:
            audioApi = oboe::AudioApi::AAudio;
            break;
        case kOboeApiOpenSLES:
            audioApi = oboe::AudioApi::OpenSLES;
            break;
        default:
            LOGE("Unknown API selection to setAPI() %d", apiType);
            return JNI_FALSE;
    }

    return engine->setAudioApi(audioApi) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_isAAudioRecommended(
        JNIEnv *env, jclass type) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine "
                "before calling this method");
        return JNI_FALSE;
    }
    return engine->isAAudioRecommended() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_native_1setDefaultStreamValues(JNIEnv *env,
                                                                                        jclass type,
                                                                                        jint sampleRate,
                                                                                        jint framesPerBurst) {
    oboe::DefaultStreamValues::SampleRate = (int32_t) sampleRate;
    oboe::DefaultStreamValues::FramesPerBurst = (int32_t) framesPerBurst;
}

// Adding FXLab Effects to LiveEffect
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_createAudioEngine(
        JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Creating audio engine...");
    engine = new EarbackEffectEngine();
    if (engine) {
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Audio engine created successfully.");
    } else {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Failed to create audio engine.");
    }
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_destroyAudioEngine(
        JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Destroying audio engine...");
    if (engine) {
        // First disable effects to stop processing
        engine->closeStreams();

        // Add a delay to ensure processing has stopped
        usleep(100000); // 100ms delay

        delete engine;
        engine = nullptr;
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Audio engine destroyed successfully.");
    } else {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Audio engine is null.");
    }
}

JNIEXPORT jobjectArray JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_getEffects(JNIEnv *env, jobject) {
    jclass jcl = env->FindClass("in/reconv/oboekitnative/datatype/EffectDescription");
    jclass jparamcl = env->FindClass("in/reconv/oboekitnative/datatype/ParamDescription");
    assert (jcl != nullptr && jparamcl != nullptr);

    auto jparamMethodId = env->GetMethodID(jparamcl, "<init>", "(Ljava/lang/String;FFF)V");
    auto jMethodId = env->GetMethodID(jcl, "<init>",
                                      "(Ljava/lang/String;Ljava/lang/String;I[Lin/reconv/oboekitnative/datatype/ParamDescription;)V");

    auto arr = env->NewObjectArray(numEffects, jcl, nullptr);
    auto lambda = [&](auto &arg, int i) {
        const auto &paramArr = arg.getParams();
        auto jparamArr = env->NewObjectArray(paramArr.size(), jparamcl, nullptr);
        int c = 0;
        for (auto const &elem: paramArr) {
            jobject j = env->NewObject(jparamcl, jparamMethodId,
                                       env->NewStringUTF(std::string(elem.kName).c_str()),
                                       elem.kMinVal, elem.kMaxVal, elem.kDefVal);
            assert(j != nullptr);
            env->SetObjectArrayElement(jparamArr, c++, j);
        }
        jobject j = env->NewObject(jcl, jMethodId,
                                   env->NewStringUTF(std::string(arg.getName()).c_str()),
                                   env->NewStringUTF(std::string(arg.getCategory()).c_str()),
                                   i, jparamArr);
        assert(j != nullptr);
        env->SetObjectArrayElement(arr, i, j);
    };
    int i = 0;
    std::apply([&i, &lambda](auto &&... args) mutable { ((lambda(args, i++)), ...); },
               EffectsTuple);
    return arr;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_addDefaultEffectNative(JNIEnv *, jobject, jint jid) {
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Radnoasndasd");
    if (!engine) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Enginer pointer null");
        return;
    }
    auto id = static_cast<int>(jid);

    std::visit([id](auto &&stack) {
        std::function<void(decltype(stack.getType()), decltype(stack.getType()))> f;
        int i = 0;
        std::apply([id, &f, &i](auto &&... args) mutable {
            ((f = (i++ == id) ?
                  args.template buildDefaultEffect<decltype(stack.getType())>() : f), ...);
        }, EffectsTuple);
        stack.addEffect(std::move(f));
    }, engine->functionList);
}

} // extern "C"

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_removeEffectNative(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jint index) {
    if (!engine) return;
    auto ind = static_cast<size_t>(index);
    std::visit([ind](auto &&arg) {
        arg.removeEffectAt(ind);
    }, engine->functionList);
}
extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_rotateEffectNative(JNIEnv *env,jobject thiz, jint jfrom,
                                                                            jint jto) {
    if (!engine) return;
    auto from = static_cast<size_t>(jfrom);
    auto to = static_cast<size_t>(jto);

    std::visit([from, to](auto &&arg) {
        arg.rotateEffectAt(from, to);
    }, engine->functionList);
}
extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_modifyEffectNative(JNIEnv *env,
                                                                            jobject thiz, jint jid,
                                                                            jint jindex,
                                                                            jfloatArray params) {
    if (!engine) return;
    int id = static_cast<int>(jid);
    int index = static_cast<size_t>(jindex);

    jfloat *data = env->GetFloatArrayElements(params, nullptr);
    std::vector<float> arr{data, data + env->GetArrayLength(params)};
    env->ReleaseFloatArrayElements(params, data, 0);
    std::visit([&arr, &id, &index](auto &&stack) {
        std::function<void(decltype(stack.getType()), decltype(stack.getType()))> ef;
        int i = 0;
        std::apply([&](auto &&... args) mutable {
            ((ef = (i++ == id) ?
                   args.modifyEffectVec(ef, arr) : ef), ...);
        }, EffectsTuple);
        stack.modifyEffectAt(index, std::move(ef));
    }, engine->functionList);
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_enableEffectNative(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jint jindex,
                                                                            jboolean jenable) {
    if (!engine) return;
    auto ind = static_cast<size_t>(jindex);
    auto enable = static_cast<bool>(jenable);
    std::visit([ind, enable](auto &&args) {
        args.enableEffectAt(ind, enable);
    }, engine->functionList);
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboekitnative_EarbackNativeLib_enablePassthroughNative(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jboolean jenable) {
    if (!engine) return;
    std::visit([enable = static_cast<bool>(jenable)](auto &&args) {
        args.mute(!enable);
    }, engine->functionList);
}