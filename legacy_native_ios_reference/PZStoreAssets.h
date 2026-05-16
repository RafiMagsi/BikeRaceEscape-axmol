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
#import <Foundation/Foundation.h>
#import "IStoreAsssets.h"

extern NSString* const POINTS_CURRENCY_ITEM_ID;



/**
 * This is an example of an implementation of IStoreAssets.
 * You'll need to create you implementation of IStoreAssets so the store will know what are the assets for your specific game.
 */

@interface PZStoreAssets : NSObject <IStoreAsssets>{
    
}

@end
