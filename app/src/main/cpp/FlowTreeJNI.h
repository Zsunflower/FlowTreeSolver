//
// Created by CUONG on 11/26/2018.
//

#ifndef FLOWTREESOLVER_FLOWTREEJNI_H
#define FLOWTREESOLVER_FLOWTREEJNI_H

#include <cmath>
#include <cstring>
#include <cstdio>
#include <jni.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include "FTSolver.h"

#define  LOG_TAG    "JNI_FlowTree"
#define DEBUG 0

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

static bool isKeyValid = false;
static const char *_secretKey = "3c56f06b0cc20e5921100334985173eb";
std::string solutionDir;

# ifdef __cplusplus
extern "C"
{
# endif

	JNIEXPORT void JNICALL Java_com_image_magick_flowtreesolver_MainActivity_setSolutionDir(JNIEnv * env, jclass obj, jstring dir);
	JNIEXPORT void JNICALL Java_com_image_magick_flowtreesolver_MainActivity_validateKey(JNIEnv * env, jclass obj, jstring key);
	JNIEXPORT jboolean JNICALL Java_com_image_magick_flowtreesolver_SolutionActivity_solve(JNIEnv * env, jobject obj, jstring path);

# ifdef __cplusplus
}
# endif


#endif //FLOWTREESOLVER_FLOWTREEJNI_H
