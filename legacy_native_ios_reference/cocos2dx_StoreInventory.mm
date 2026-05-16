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
#import "StoreInventory.h"
#import "VirtualItemNotFoundException.h"


/**
 * This implementation is used to let cocos2dx functions perform actions on StoreInventory.
 *
 * You can see the documentation of every function in StoreInventory.
 */


int cocos2dx_StoreInventory::getCurrencyBalance(string currencyItemId) throw (cocos2dx_VirtualItemNotFoundException&){
    @try {
        NSString * str = [[NSString alloc] initWithBytes:currencyItemId.c_str() length:strlen(currencyItemId.c_str()) encoding:NSUTF8StringEncoding];
        return [StoreInventory getCurrencyBalance:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

int cocos2dx_StoreInventory::addCurrencyAmount(string currencyItemId, int amount) throw (cocos2dx_VirtualItemNotFoundException&){
    @try {
        NSString * str = [[NSString alloc] initWithBytes:currencyItemId.c_str() length:strlen(currencyItemId.c_str()) encoding:NSUTF8StringEncoding];
        return [StoreInventory addAmount:amount toCurrency:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

int cocos2dx_StoreInventory::removeCurrencyAmount(string currencyItemId, int amount) throw (cocos2dx_VirtualItemNotFoundException&){
    @try {
        NSString * str = [[NSString alloc] initWithBytes:currencyItemId.c_str() length:strlen(currencyItemId.c_str()) encoding:NSUTF8StringEncoding];
        return [StoreInventory removeAmount:amount fromCurrency:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

int cocos2dx_StoreInventory::getGoodBalance(string goodItemId) throw (cocos2dx_VirtualItemNotFoundException&){
    @try {
        NSString * str = [[NSString alloc] initWithBytes:goodItemId.c_str() length:strlen(goodItemId.c_str()) encoding:NSUTF8StringEncoding];
        return [StoreInventory getGoodBalance:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

int cocos2dx_StoreInventory::addGoodAmount(string goodItemId, int amount) throw (cocos2dx_VirtualItemNotFoundException&){
    @try {
        NSString * str = [[NSString alloc] initWithBytes:goodItemId.c_str() length:strlen(goodItemId.c_str()) encoding:NSUTF8StringEncoding];
        return [StoreInventory addAmount:amount toGood:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

int cocos2dx_StoreInventory::removeGoodAmount(string goodItemId, int amount) throw (cocos2dx_VirtualItemNotFoundException&){
    @try {
        NSString * str = [[NSString alloc] initWithBytes:goodItemId.c_str() length:strlen(goodItemId.c_str()) encoding:NSUTF8StringEncoding];
        return [StoreInventory removeAmount:amount fromGood:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}
