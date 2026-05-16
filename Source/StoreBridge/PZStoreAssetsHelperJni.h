#include <jni.h>
#ifndef _Included_PZStoreAssetsHelperJNI
#define _Included_PZStoreAssetsHelperJNI

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring JNICALL Java_com_kayabit_store_PZStoreAssetsHelper_nvGetStoreKey(JNIEnv *env, jobject thiz);
JNIEXPORT jstring JNICALL Java_com_kayabit_store_PZStoreAssetsHelper_nvGetChatracterStoreId(JNIEnv *env, jobject thiz, jint character_id);
JNIEXPORT jstring JNICALL Java_com_kayabit_store_PZStoreAssetsHelper_nvGetKidModeStoreId(JNIEnv *env, jobject thiz);
JNIEXPORT jstring JNICALL Java_com_kayabit_store_PZStoreAssetsHelper_nvGetRemoveAdStoreId(JNIEnv *env, jobject thiz);
JNIEXPORT jstring JNICALL Java_com_kayabit_store_PZStoreAssetsHelper_nvGetCoinPackStoreId(JNIEnv *env, jobject thiz, jint pack_id );
JNIEXPORT jint JNICALL Java_com_kayabit_store_PZStoreAssetsHelper_nvGetCoinPackValue(JNIEnv *env, jobject thiz, jint pack_id );

#ifdef __cplusplus
}
#endif
#endif