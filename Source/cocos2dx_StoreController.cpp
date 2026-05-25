#include "cocos2dx_StoreController.h"

#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)

void cocos2dx_StoreController::storeOpening() {}
void cocos2dx_StoreController::storeClosing() {}
void cocos2dx_StoreController::initialize(const char*) {}

void cocos2dx_StoreController::buyMarketItem(const char* productId) {
    AXLOGW("[IAP] buyMarketItem('{}') ignored (StoreKit not available on this platform)", productId ? productId : "(null)");
}

void cocos2dx_StoreController::restorePurchases() {
    AXLOGW("[IAP] restorePurchases() ignored (StoreKit not available on this platform)");
}

#endif

