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

#include "cocos2dx_StoreInfo.h"
#include "JniHelpers.h"

string cocos2dx_StoreInfo::getPackProductId(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonStringCall(
		"getPackProductId", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		itemId
	); 
}

string cocos2dx_StoreInfo::getPackName(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonStringCall(
		"getPackName", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		itemId
	); 
}

string cocos2dx_StoreInfo::getPackDescription(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonStringCall(
		"getPackDescription", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		itemId
	);
}

double cocos2dx_StoreInfo::getPackPrice(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonDoubleCall(
		"getPackPrice", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		itemId
	);
}

string cocos2dx_StoreInfo::getGoodName(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonStringCall(
		"getGoodName", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		itemId
	);
}

string cocos2dx_StoreInfo::getGoodDescription(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonStringCall(
		"getGoodDescription", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		itemId
	);
}

int cocos2dx_StoreInfo::getGoodPriceForCurrency(const char* goodItemId, const char* currencyItemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"getGoodPriceForCurrency", 
		"com/soomla/cocos2dx/store/StoreInfoBridge", 
		goodItemId,
		currencyItemId
	);
}



