#include <jni.h>
#include <android/bitmap.h>

#include <iostream>

#include "StdoutsToLogcatRedirector.h"


// https://developer.android.com/training/articles/perf-jni.html#faq_ULE
// http://stackoverflow.com/a/2480564
// http://stackoverflow.com/a/17643762
// if your native file is compiled as C++ you would need extern "C" {...}
#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jboolean JNICALL Java_com_nextgis_ngsandroid_NgsAndroidJni_initLogger(
        JNIEnv* env,
        jclass type)
{
    return ngs::StdoutsToLogcatRedirector::init();
}


// http://stackoverflow.com/a/22693766
JNIEXPORT jlong JNICALL Java_com_nextgis_ngsandroid_NgsAndroidJni_lockBitmapPixels(
        JNIEnv* env,
        jclass type,
        jobject bitmap)
{
    void* p_buffer;
    int ret;

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &p_buffer)) < 0) {
        std::cerr << "Error - AndroidBitmap_lockPixels() Failed! error: " << ret << std::endl;
        return 0;
    }

    return reinterpret_cast<jlong> (p_buffer);
}

JNIEXPORT void JNICALL Java_com_nextgis_ngsandroid_NgsAndroidJni_unlockBitmapPixels(
        JNIEnv* env,
        jclass type,
        jobject bitmap)
{
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT jobject JNICALL Java_com_nextgis_ngsandroid_NgsAndroidJni_fillBitmapFromBuffer(
        JNIEnv* env,
        jclass type,
        jobject buffer,
        jint width,
        jint height)
{
// https://github.com/AndroidDeveloperLB/AndroidJniBitmapOperations
// http://stackoverflow.com/questions/18250951/jni-bitmap-operations-for-helping-to-avoid-oom-when-using-large-images
// http://stackoverflow.com/questions/17900732/how-to-cache-bitmaps-into-native-memory

    void* p_buffer = env->GetDirectBufferAddress(buffer);

    // creating a new bitmap to put the pixels into it
    // using Bitmap Bitmap.createBitmap (int width, int height, Bitmap.Config config)
    jclass classBitmap = env->FindClass("android/graphics/Bitmap");
    jmethodID methodCreateBitmap = env->GetStaticMethodID(
            classBitmap, "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jclass classBitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID methodValueOf = env->GetStaticMethodID(
            classBitmapConfig, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jobject bitmapConfig =
            env->CallStaticObjectMethod(classBitmapConfig, methodValueOf, configName);
    jobject bitmap = env->CallStaticObjectMethod(
            classBitmap, methodCreateBitmap, width, height, bitmapConfig);

    // putting the pixels into the new bitmap:
    int ret;
    void* p_pixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &p_pixels)) < 0) {
        //LOGE("AndroidBitmap_lockPixels() failed! Error=%d", ret);
        return NULL;
    }
    memcpy(p_pixels, p_buffer, sizeof(uint32_t) * width * height);

    AndroidBitmap_unlockPixels(env, bitmap);
    //LOGD("returning the new bitmap");
    return bitmap;
}


#ifdef __cplusplus
}
#endif