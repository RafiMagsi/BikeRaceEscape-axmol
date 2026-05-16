

#include "PZStoreAssetsHelperJni.h"
#include "SharedData/PZGSharedData.h"

#ifdef __cplusplus 
extern "C" { 
#endif 


JNIEXPORT jstring Java_com_kayabit_store_PZStoreAssetsHelper_nvGetStoreKey(JNIEnv *env, jobject thiz){
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    sd->readDataFromFile();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettingsAndroid");
    if(array == NULL){
        return NULL;
    }
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);

    if (iapInfo->storeKey){
        const char* storeId = iapInfo->storeKey->getCString();
        return (jstring)env->NewStringUTF(storeId);
    }
    else{
        return NULL;
    }
} 

JNIEXPORT jstring Java_com_kayabit_store_PZStoreAssetsHelper_nvGetChatracterStoreId(JNIEnv *env, jobject thiz, jint character_id){
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    sd->readDataFromFile();
    ax::__Array* array = (ax::__Array*)sd->artResource->objectForKey("Characters");
    if(array == NULL){
        return NULL;
    }
    if(array->count() <= character_id){
        return NULL;
    }

    PZGArtCharacter *charInfo = (PZGArtCharacter*)array->objectAtIndex( character_id );
    if (charInfo->enableIAP) {
        const char* storeId = charInfo->storeId->getCString();
        return (jstring)env->NewStringUTF(storeId);
    }
    else{
        return NULL;
    }

}

JNIEXPORT jstring Java_com_kayabit_store_PZStoreAssetsHelper_nvGetRemoveAdStoreId(JNIEnv *env, jobject thiz){
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    sd->readDataFromFile();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettingsAndroid");
    if(array == NULL){
        return NULL;
    }
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);

    if (iapInfo->removeAd_id){
        const char* storeId = iapInfo->removeAd_id->getCString();
        return (jstring)env->NewStringUTF(storeId);
    }
    else{
        return NULL;
    }
} 

JNIEXPORT jstring Java_com_kayabit_store_PZStoreAssetsHelper_nvGetKidModeStoreId(JNIEnv *env, jobject thiz){
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    sd->readDataFromFile();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettingsAndroid");
    if(array){
        return NULL;
    }
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);

    if (iapInfo->kidMode_id){
        const char* storeId = iapInfo->kidMode_id->getCString();
        return (jstring)env->NewStringUTF(storeId);
    }
    else{
        return NULL;
    }
} 

JNIEXPORT jstring Java_com_kayabit_store_PZStoreAssetsHelper_nvGetCoinPackStoreId(JNIEnv *env, jobject thiz, jint pack_id) { 

    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    sd->readDataFromFile();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettingsAndroid");
    if (array == NULL){
        return NULL;
    }
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);

    if (pack_id == 1){
        if (iapInfo->coinShop1_id) {
            const char* storeId = iapInfo->coinShop1_id->getCString();
            return (jstring)env->NewStringUTF(storeId);
        }
        else{
            return NULL;
        }
    }
    else if (pack_id == 2){
        if (iapInfo->coinShop2_id) {
            const char* storeId = iapInfo->coinShop2_id->getCString();
            return (jstring)env->NewStringUTF(storeId);
        }
        else{
            return NULL;
        }
    }
    else if (pack_id == 3){
        if (iapInfo->coinShop3_id) {
            const char* storeId = iapInfo->coinShop3_id->getCString();
            return (jstring)env->NewStringUTF(storeId);
        }
        else{
            return NULL;
        }
    }

} 

JNIEXPORT jint Java_com_kayabit_store_PZStoreAssetsHelper_nvGetCoinPackValue(JNIEnv *env, jobject thiz, jint pack_id ){
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    sd->readDataFromFile();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettingsAndroid");
    if(array == NULL){
        return NULL;
    }
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);

    if (pack_id == 1){
        return iapInfo->coinShop1_value;
    }
    else if (pack_id == 2){
        return iapInfo->coinShop2_value;
    }
    else if (pack_id == 3){
        return iapInfo->coinShop3_value;
    }
}

#ifdef __cplusplus
} 
#endif 