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
#import "StoreController.h"
#import "VirtualItemNotFoundException.h"
#import "InsufficientFundsException.h"
#import "NotEnoughGoodsException.h"

#import "PZStoreAssets.h"

/**
 * This implementation is used to let cocos2dx functions perform actions on StoreController.
 *
 * You can see the documentation of every function in StoreController.
 */

void cocos2dx_StoreController::storeOpening() {
    [[StoreController getInstance] storeOpening];
}

void cocos2dx_StoreController::storeClosing() {
    [[StoreController getInstance] storeClosing];
}

void cocos2dx_StoreController::initialize(string customSecret) {

    NSString * str = [[NSString alloc] initWithBytes:customSecret.c_str() length:strlen(customSecret.c_str()) encoding:NSUTF8StringEncoding];
    [[StoreController getInstance] initializeWithStoreAssets:[[PZStoreAssets alloc] init] andCustomSecret:str];

}

void cocos2dx_StoreController::buyMarketItem(string productId) throw(cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:productId.c_str() length:strlen(productId.c_str()) encoding:NSUTF8StringEncoding];
        [[StoreController getInstance] buyAppStoreItemWithProcuctId:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        // throw cocos2dx_VirtualItemNotFoundException();
    }
}

void cocos2dx_StoreController::buyVirtualGood(string itemId) throw (cocos2dx_VirtualItemNotFoundException&, cocos2dx_InsufficientFundsException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId.c_str() length:strlen(itemId.c_str()) encoding:NSUTF8StringEncoding];
        [[StoreController getInstance] buyVirtualGood:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
    @catch (InsufficientFundsException *exception) {
        throw cocos2dx_InsufficientFundsException();
    }
}

void cocos2dx_StoreController::equipVirtualGood(string itemId) throw (cocos2dx_NotEnoughGoodsException&, cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId.c_str() length:strlen(itemId.c_str()) encoding:NSUTF8StringEncoding];
        [[StoreController getInstance] equipVirtualGood:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
    @catch (NotEnoughGoodsException *exception) {
        throw cocos2dx_NotEnoughGoodsException();
    }
}

void cocos2dx_StoreController::unequipVirtualGood(string itemId) throw(cocos2dx_VirtualItemNotFoundException&) {
    @try {
        NSString * str = [[NSString alloc] initWithBytes:itemId.c_str() length:strlen(itemId.c_str()) encoding:NSUTF8StringEncoding];
        [[StoreController getInstance] unequipVirtualGood:str];
    }
    @catch (VirtualItemNotFoundException *exception) {
        throw cocos2dx_VirtualItemNotFoundException();
    }
}
