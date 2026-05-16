/*
 * Copyright (C) 2012 Soomla Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PZLegacyCompat.h"
#include "com_soomla_cocos2dx_store_EventHandlerBridge.h"

#include "platform/android/jni/JniHelper.h"
#include <android/log.h>

#include "cocos2dx_EventHandlers.h"

#include <string>

using namespace std;

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_marketPurchase
  (JNIEnv * env, jobject obj, jstring productId){
	  string productIdStr = ax::JniHelper::jstring2string(productId);
	  string msg(productIdStr);
	  msg.append(" was just purchased.");
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
	  
	  cocos2dx_EventHandlers::getInstance()->marketPurchase(productIdStr);
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_marketRefund
  (JNIEnv * env, jobject obj, jstring productId){
	  string productIdStr = ax::JniHelper::jstring2string(productId);
	  string msg(productIdStr);
	  msg.append(" was just refunded.");
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
	  
	  cocos2dx_EventHandlers::getInstance()->marketRefund(productIdStr);
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_virtualGoodPurchased
  (JNIEnv * env, jobject obj, jstring itemId){
	  string itemIdStr = ax::JniHelper::jstring2string(itemId);
	  string msg(itemIdStr);
	  msg.append(" was just purchased.");
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());

	  cocos2dx_EventHandlers::getInstance()->virtualGoodPurchased(itemIdStr);	  
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_virtualGoodEquipped
  (JNIEnv * env, jobject obj, jstring itemId){
	  string itemIdStr = ax::JniHelper::jstring2string(itemId);
	  string msg(itemIdStr);
	  msg.append(" was just equipped.");
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
	  
	  cocos2dx_EventHandlers::getInstance()->virtualGoodEquipped(itemIdStr);
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_virtualGoodUnequipped
  (JNIEnv * env, jobject obj, jstring itemId){
	  string itemIdStr = ax::JniHelper::jstring2string(itemId);
	  string msg(itemIdStr);
	  msg.append(" was just unequipped.");
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
	  
	  cocos2dx_EventHandlers::getInstance()->virtualGoodUnequipped(itemIdStr);
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_billingSupported
  (JNIEnv * env, jobject obj){
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", "Billing is supported.");
	  
	  cocos2dx_EventHandlers::getInstance()->billingSupported();
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_billingNotSupported
  (JNIEnv * env, jobject obj){
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", "Billing is not supported.");
	  
	  cocos2dx_EventHandlers::getInstance()->billingNotSupported();
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_marketPurchaseProcessStarted
  (JNIEnv * env, jobject obj, jstring productId){
	  string productIdStr = ax::JniHelper::jstring2string(productId);
	  string msg("bc Market purchase just started for: ");
	  msg += productIdStr;
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
	  
	  cocos2dx_EventHandlers::getInstance()->marketPurchaseProcessStarted(productIdStr);
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_goodsPurchaseProcessStarted
  (JNIEnv * env, jobject obj){
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", "VirtualGoods purchase started.");
	  
	  cocos2dx_EventHandlers::getInstance()->goodsPurchaseProcessStarted();
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_closingStore
  (JNIEnv * env, jobject obj){
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", "Store is closing.");
	  
	  cocos2dx_EventHandlers::getInstance()->closingStore();
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_unexpectedErrorInStore
  (JNIEnv * env, jobject obj){
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", "Unexpected error occured.");
	  string errorMsg("error");
	  cocos2dx_EventHandlers::getInstance()->unexpectedErrorInStore( errorMsg );
  }

JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_openingStore
  (JNIEnv * env, jobject obj){
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", "Store is opening.");
	  
	  cocos2dx_EventHandlers::getInstance()->openingStore();
  }
  
JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_currencyBalanceChanged
  (JNIEnv * env, jobject obj, jstring itemId, jint balance){
	  string itemIdStr = ax::JniHelper::jstring2string(itemId);
	  string msg("Currency balance changed for: ");
	  msg += itemIdStr;
	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
  	  
	  
  	  cocos2dx_EventHandlers::getInstance()->currencyBalanceChanged(itemIdStr, balance);
  }
  
JNIEXPORT void JNICALL Java_com_soomla_cocos2dx_store_EventHandlerBridge_goodBalanceChanged
  (JNIEnv * env, jobject obj, jstring itemId, jint balance){
  	  string itemIdStr = ax::JniHelper::jstring2string(itemId);
  	  string msg("Good balance changed for: ");
  	  msg += itemIdStr;
  	  __android_log_write(ANDROID_LOG_DEBUG, "SOOMLA JNI", msg.c_str());
  	  
	  
   	  cocos2dx_EventHandlers::getInstance()->goodBalanceChanged(itemIdStr, balance);
  }
	
