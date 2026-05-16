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
#import "VirtualCurrencyPack.h"
#import "VirtualGood.h"
#import "StoreInfo.h"
#import "AppStoreItem.h"
#import "VirtualItemNotFoundException.h"

/**
 * This implementation is used to let cocos2dx functions retrieve data from StoreInfo.
 *
 * You can see the documentation of every function in StoreInfo.
 */


string cocos2dx_StoreInfo::getPackProductId(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId length:strlen(itemId) encoding:NSUTF8StringEncoding];
        VirtualCurrencyPack* pack = [[StoreInfo getInstance] currencyPackWithItemId:str];
        return [pack.appstoreItem.productId UTF8String];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

string cocos2dx_StoreInfo::getPackName(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId length:strlen(itemId) encoding:NSUTF8StringEncoding];
        VirtualCurrencyPack* pack = [[StoreInfo getInstance] currencyPackWithItemId:str];
        return [pack.name UTF8String];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

string cocos2dx_StoreInfo::getPackDescription(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId length:strlen(itemId) encoding:NSUTF8StringEncoding];
        VirtualCurrencyPack* pack = [[StoreInfo getInstance] currencyPackWithItemId:str];
        return [pack.description UTF8String];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

double cocos2dx_StoreInfo::getPackPrice(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId length:strlen(itemId) encoding:NSUTF8StringEncoding];
        VirtualCurrencyPack* pack = [[StoreInfo getInstance] currencyPackWithItemId:str];
        return pack.appstoreItem.price;
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

string cocos2dx_StoreInfo::getGoodName(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId length:strlen(itemId) encoding:NSUTF8StringEncoding];
        VirtualGood* good = [[StoreInfo getInstance] goodWithItemId:str];
        return [good.name UTF8String];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}

string cocos2dx_StoreInfo::getGoodDescription(const char* itemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId length:strlen(itemId) encoding:NSUTF8StringEncoding];
        VirtualGood* good = [[StoreInfo getInstance] goodWithItemId:str];
        return [good.description UTF8String];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }

}

int cocos2dx_StoreInfo::getGoodPriceForCurrency(const char* goodItemId, const char* currencyItemId) throw (cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str1 = [[NSString alloc] initWithBytes:goodItemId length:strlen(goodItemId) encoding:NSUTF8StringEncoding];
        NSString * str2 = [[NSString alloc] initWithBytes:currencyItemId length:strlen(currencyItemId) encoding:NSUTF8StringEncoding];
        VirtualGood* good = [[StoreInfo getInstance] goodWithItemId:str1];
        return [(NSNumber*)[good.currencyValues objectForKey:str2] intValue];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}
