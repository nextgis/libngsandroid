#include <jni.h>
#include <android/bitmap.h>

#include <iostream>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

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
    void* pPixels;
    int ret;

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pPixels)) < 0) {
        std::cerr << "Error - AndroidBitmap_lockPixels() Failed! error: " << ret << std::endl;
        return 0;
    }

    return reinterpret_cast<jlong> (pPixels);
}

JNIEXPORT void JNICALL Java_com_nextgis_ngsandroid_NgsAndroidJni_unlockBitmapPixels(
        JNIEnv* env,
        jclass type,
        jobject bitmap)
{
    AndroidBitmap_unlockPixels(env, bitmap);
}

bool renderer(
        void* imagePtr,
        uint32_t imageWidth,
        uint32_t imageHeight)
{
//    BOOST_LOG_SEV(GmLog::log, debug) << "testPath: " << testPath;

    // http://stackoverflow.com/q/28817777
    // http://stackoverflow.com/a/27092070
    // TODO: work with OpenGL errors
    // Step 1 - Get the default display.
    EGLDisplay eglDisplay = eglGetDisplay((EGLNativeDisplayType) 0);

    // Step 2 - Initialize EGL.
    eglInitialize(eglDisplay, 0, 0);

    // Step 3 - Make OpenGL ES the current API.
    eglBindAPI(EGL_OPENGL_API);

    // Step 4 - Specify the required configuration attributes.
    EGLint pi32ConfigAttribs[9];
    pi32ConfigAttribs[0] = EGL_COLOR_BUFFER_TYPE;
    pi32ConfigAttribs[1] = EGL_RGB_BUFFER;
    pi32ConfigAttribs[2] = EGL_LEVEL;
    pi32ConfigAttribs[3] = 0;
    pi32ConfigAttribs[4] = EGL_RENDERABLE_TYPE;
    pi32ConfigAttribs[5] = EGL_OPENGL_ES2_BIT;
    pi32ConfigAttribs[6] = EGL_SURFACE_TYPE;
    pi32ConfigAttribs[7] = EGL_PBUFFER_BIT;
    pi32ConfigAttribs[8] = EGL_NONE;

    // Step 5 - Find a config that matches all requirements.
    int iConfigs;
    EGLConfig eglConfig;
    eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs);
//    BOOST_LOG_SEV(GmLog::log, debug) << "egl error" << eglGetError();

    if (iConfigs != 1) {
//        throw GmCoreErrEx() << GmCoreErrInfo("Error: eglChooseConfig(): config not found.");
        return false;
    }

    EGLint pbufferAttribs[5];
    pbufferAttribs[0] = EGL_WIDTH;
    pbufferAttribs[1] = imageWidth;
    pbufferAttribs[2] = EGL_HEIGHT;
    pbufferAttribs[3] = imageHeight;
    pbufferAttribs[4] = EGL_NONE;

    // Step 6 - Create a surface to draw to.
    EGLSurface eglSurface;
    eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbufferAttribs);
//    BOOST_LOG_SEV(GmLog::log, debug) << "egl error" << eglGetError();

    if (eglSurface == EGL_NO_SURFACE) {
//        BOOST_LOG_SEV(GmLog::log, debug) << "surface issue";
        return false;
    }

    // Step 7 - Create a context.
    EGLContext eglContext;
    eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
//    BOOST_LOG_SEV(GmLog::log, debug) << "egl error" << eglGetError();

    if (eglContext == EGL_NO_CONTEXT) {
//        BOOST_LOG_SEV(GmLog::log, debug) << "context issue";
        return false;
    }

    // Step 8 - Bind the context to the current thread
    bool result = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    if (!result) {
//        BOOST_LOG_SEV(GmLog::log, debug) << "make current error" << eglGetError();
        return false;
    }
//    BOOST_LOG_SEV(GmLog::log, debug) << "egl error" << eglGetError();

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(eglDisplay, eglSurface);
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, imagePtr);

    // set the reply data
    return true;
}

JNIEXPORT jboolean JNICALL Java_com_nextgis_ngsandroid_NgsAndroidJni_fillImage(
        JNIEnv* env,
        jclass type,
        jlong imagePointer,
        jint imageWidth,
        jint imageHeight)
{
    void* imagePtr = reinterpret_cast<void*> (imagePointer);
    return renderer(imagePtr, imageWidth, imageHeight);
}


#ifdef __cplusplus
}
#endif