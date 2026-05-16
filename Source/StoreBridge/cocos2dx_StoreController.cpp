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

#include "cocos2dx_StoreController.h"
#include "JniHelpers.h"

void cocos2dx_StoreController::storeOpening() {
	JniHelpers::jniCommonVoidCall(
		"storeOpening", 
		"com/soomla/cocos2dx/store/StoreControllerBridge"
	);

}

void cocos2dx_StoreController::storeClosing() {
	JniHelpers::jniCommonVoidCall(
		"storeClosing", 
		"com/soomla/cocos2dx/store/StoreControllerBridge"
	);

}

void cocos2dx_StoreController::initialize(string customSecret) {
	JniHelpers::jniCommonVoidCall(
		"initialize", 
		"com/soomla/cocos2dx/store/StoreControllerBridge",
		customSecret.c_str()
	);
}

void cocos2dx_StoreController::buyMarketItem(string productId) throw(cocos2dx_VirtualItemNotFoundException&){
	JniHelpers::jniCommonVoidCall(
		"buyGoogleMarketItem", 
		"com/soomla/cocos2dx/store/StoreControllerBridge", 
		productId.c_str()
	);
}

void cocos2dx_StoreController::buyVirtualGood(string itemId) throw (cocos2dx_VirtualItemNotFoundException&, cocos2dx_InsufficientFundsException&) {
	JniHelpers::jniCommonVoidCall(
		"buyVirtualGood", 
		"com/soomla/cocos2dx/store/StoreControllerBridge", 
		itemId.c_str()
	);
}

void cocos2dx_StoreController::equipVirtualGood(string itemId) throw (cocos2dx_NotEnoughGoodsException&, cocos2dx_VirtualItemNotFoundException&) {
	JniHelpers::jniCommonVoidCall(
		"equipVirtualGood", 
		"com/soomla/cocos2dx/store/StoreControllerBridge", 
		itemId.c_str()
	);
}

void cocos2dx_StoreController::unequipVirtualGood(string itemId) throw(cocos2dx_VirtualItemNotFoundException&) {
	JniHelpers::jniCommonVoidCall(
		"unequipVirtualGood", 
		"com/soomla/cocos2dx/store/StoreControllerBridge", 
		itemId.c_str()
	);
}

