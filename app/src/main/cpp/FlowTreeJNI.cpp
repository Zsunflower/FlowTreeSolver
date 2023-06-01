//
// Created by CUONG on 11/26/2018.
//

#include "FlowTreeJNI.h"
using namespace cv;

JNIEXPORT void JNICALL Java_com_image_magick_flowtreesolver_MainActivity_setSolutionDir(JNIEnv * env, jclass obj, jstring dir)
{
	const char *_key;
	_key = env->GetStringUTFChars(dir, NULL);
	solutionDir = _key;
	env->ReleaseStringUTFChars(dir, _key);
}

JNIEXPORT void JNICALL Java_com_image_magick_flowtreesolver_MainActivity_validateKey(JNIEnv * env, jclass obj, jstring key)
{
	const char *_key;

	_key = env->GetStringUTFChars(key, NULL);
	if (!strcmp(_key, _secretKey))
	{
		isKeyValid = true;
		LOGD("Key is valid");
	}
	else
	{
		isKeyValid = false;
		LOGD("Key is invalid");
	}
	env->ReleaseStringUTFChars(key, _key);
}

JNIEXPORT jboolean JNICALL Java_com_image_magick_flowtreesolver_SolutionActivity_solve(JNIEnv * env, jobject obj, jstring path)
{
    /*
     * Solve Flow Tree game
     * Arguments:
     * path: path to screenshoot
     */
    if(!isKeyValid)
        return JNI_FALSE;
    jclass thiz = env->GetObjectClass(obj);
    jmethodID onSuccess = env->GetMethodID(thiz, "onJNISuccess", "()V");
    jmethodID onFail = env->GetMethodID(thiz, "onJNIFailed", "()V");

	const char *_path;
	_path = env->GetStringUTFChars(path, NULL);
    Mat gameScreen = imread(_path);
	if (gameScreen.cols != 1080)
		resize(gameScreen, gameScreen, Size(1080, 1080 * gameScreen.rows / gameScreen.cols));
    if(gameScreen.rows < 1 || gameScreen.cols < 1)
	{
		LOGD("Can not read image file: %s", _path);
		env->ReleaseStringUTFChars(path, _path);
        env->CallVoidMethod(obj, onFail);
        return JNI_FALSE;
	}
	LOGD("Read image %s success", _path);
    LOGD("Size %d x %d", gameScreen.rows, gameScreen.cols);
	env->ReleaseStringUTFChars(path, _path);

	FTSolver solver(gameScreen);
    if(!solver.preprocessMat())
	{
		LOGD("Preprocess failed, Game has no solution");
		env->CallVoidMethod(obj, onFail);
		return JNI_FALSE;
	}
    solver.initBoard();
    solver.initVar();
//    solver.logDebug();
    if(solver.solveBoard())
	{
		Mat solution_mat;
		solver.drawSolution(solution_mat);
		LOGD("Found a solution, log to %s", solutionDir.c_str());
		imwrite(solutionDir + "/" + "solution.png", solution_mat);
        env->CallVoidMethod(obj, onSuccess);
        return JNI_TRUE;
	}
	else
	{
		LOGD("JNI Game has no solution");
        env->CallVoidMethod(obj, onFail);
        return JNI_FALSE;
	}
}