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

#import "PZStoreAssets.h"
#import "VirtualCategory.h"
#import "VirtualCurrency.h"
#import "VirtualGood.h"
#import "VirtualCurrencyPack.h"
#import "StaticPriceModel.h"

#import "NonConsumableItem.h"

#include "PZGSharedData.h"

NSString* const POINTS_CURRENCY_ITEM_ID      = @"currency_points";


@implementation PZStoreAssets

VirtualCategory* GENERAL_CATEGORY;
VirtualCurrency* POINTS_CURRENCY;

NSMutableArray* nonConsumableItems;

VirtualCurrencyPack* FIRST_PACK;
VirtualCurrencyPack* SECOND_PACK;
VirtualCurrencyPack* THIRD_PACK;

+ (void)initialize{
    nonConsumableItems = [[NSMutableArray alloc] init];
    
    /** Virtual Categories **/
    GENERAL_CATEGORY = [[VirtualCategory alloc] initWithName:@"General"
                                                       andId:0
                                           andEquippingModel:kNone];
    
    /** Virtual Currencies **/
    
    POINTS_CURRENCY = [[VirtualCurrency alloc] initWithName:@"Points"
                                             andDescription:@""
                                                  andItemId:POINTS_CURRENCY_ITEM_ID];
    
    
    /** Non Consumable Items **/
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    cocos2d::CCArray* array = (cocos2d::CCArray*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    
    NonConsumableItem *nonConsumableItem;
    
    if (iapInfo->removeAd_id) {
        nonConsumableItem = [[NonConsumableItem alloc] initWithName:@"RemoveAd"
                                                 andDescription:@"Description"
                                                      andItemId:@"remove_ad"
                                                       andPrice:1.99
                                                   andProductId:[NSString stringWithFormat:@"%s",iapInfo->removeAd_id->getCString()]];
        [nonConsumableItems addObject:nonConsumableItem];
    }
    
    if (iapInfo->kidMode_id) {
        nonConsumableItem = [[NonConsumableItem alloc] initWithName:@"KidMode"
                                                andDescription:@"Description"
                                                     andItemId:@"kid_mode"
                                                      andPrice:1.99
                                                  andProductId:[NSString stringWithFormat:@"%s",iapInfo->kidMode_id->getCString()]];
        [nonConsumableItems addObject:nonConsumableItem];
    }

    array = (cocos2d::CCArray*)sd->artResource->objectForKey("Characters");
    for (int i=0; i < array->count(); i++) {
        PZGArtCharacter *charInfo = (PZGArtCharacter*)array->objectAtIndex( i );
        if (charInfo->enableIAP) {
            nonConsumableItem = [[NonConsumableItem alloc] initWithName:@"Character"
                                                         andDescription:@"Description"
                                                              andItemId:@"character"
                                                               andPrice:0.99
                                                           andProductId:[NSString stringWithFormat:@"%s",charInfo->storeId->getCString()]];
         
            [nonConsumableItems addObject:nonConsumableItem];
        }
    }
    
    
    /** Virtual Currency Pack **/

    
    if (iapInfo->coinShop1_id) {
        FIRST_PACK = [[VirtualCurrencyPack alloc] initWithName:@"CoinPack1"
                                                andDescription:@"Description"
                                                     andItemId:@"coinpack_1"
                                                      andPrice:0.99
                                                  andProductId:[NSString stringWithFormat:@"%s",iapInfo->coinShop1_id->getCString()]
                                             andCurrencyAmount:iapInfo->coinShop1_value
                                                   andCurrency:POINTS_CURRENCY];
    }
    
    if (iapInfo->coinShop2_id) {
        SECOND_PACK = [[VirtualCurrencyPack alloc] initWithName:@"CoinPack2"
                                                 andDescription:@"Description"
                                                      andItemId:@"coinpack_2"
                                                       andPrice:1.99
                                                   andProductId:[NSString stringWithFormat:@"%s",iapInfo->coinShop2_id->getCString()]
                                              andCurrencyAmount:iapInfo->coinShop2_value
                                                    andCurrency:POINTS_CURRENCY];
    }
    
    if (iapInfo->coinShop3_id) {
        THIRD_PACK = [[VirtualCurrencyPack alloc] initWithName:@"CoinPack3"
                                                andDescription:@"Description"
                                                     andItemId:@"coinpack_3"
                                                      andPrice:4.99
                                                  andProductId:[NSString stringWithFormat:@"%s",iapInfo->coinShop3_id->getCString()]
                                             andCurrencyAmount:iapInfo->coinShop3_value
                                                   andCurrency:POINTS_CURRENCY];
    }

    
}

- (int) getVersion {
    return 0;
}

- (NSArray*)virtualCurrencies{
    return @[POINTS_CURRENCY];
}

- (NSArray*)virtualGoods{
    return nil;
}

- (NSArray*)virtualCurrencyPacks{
    if (FIRST_PACK && SECOND_PACK && THIRD_PACK) {
        return @[FIRST_PACK, SECOND_PACK, THIRD_PACK];
    }
    else{
        return nil;
    }

}

- (NSArray*)virtualCategories{
    return @[GENERAL_CATEGORY];
}

- (NSArray*)nonConsumableItems{
    return (NSArray*)nonConsumableItems;
}

@end
