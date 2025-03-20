#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring
Java_in_reconv_oboekitnative_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}