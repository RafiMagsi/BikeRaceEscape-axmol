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

#include "cocos2dx_StoreInventory.h"
#include "JniHelpers.h"

int cocos2dx_StoreInventory::getCurrencyBalance(string currencyItemId) throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"getCurrencyBalance", 
		"com/soomla/cocos2dx/store/StoreInventoryBridge", 
		currencyItemId.c_str()
	); 
}

int cocos2dx_StoreInventory::addCurrencyAmount(string currencyItemId, int amount)  throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"addCurrencyAmount", 
		"com/soomla/cocos2dx/store/StoreInventoryBridge", 
		currencyItemId.c_str(),
		amount
	); 
}

int cocos2dx_StoreInventory::removeCurrencyAmount(string currencyItemId, int amount)  throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"removeCurrencyAmount", 
		"com/soomla/cocos2dx/store/StoreInventoryBridge", 
		currencyItemId.c_str(),
		amount
	); 
}

int cocos2dx_StoreInventory::getGoodBalance(string goodItemId)  throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"getGoodBalance", 
		"com/soomla/cocos2dx/store/StoreInventoryBridge", 
		goodItemId.c_str()
	);
}

int cocos2dx_StoreInventory::addGoodAmount(string goodItemId, int amount)  throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"addGoodAmount", 
		"com/soomla/cocos2dx/store/StoreInventoryBridge", 
		goodItemId.c_str(),
		amount
	);
}

int cocos2dx_StoreInventory::removeGoodAmount(string goodItemId, int amount)  throw (cocos2dx_VirtualItemNotFoundException&) {
	return JniHelpers::jniCommonIntCall(
		"removeGoodAmount", 
		"com/soomla/cocos2dx/store/StoreInventoryBridge", 
		goodItemId.c_str(),
		amount
	);
}
