#include "cocos2dx_StoreController.h"

#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// Implemented in cocos2dx_StoreControllerApple.mm.
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"

namespace {
constexpr const char* kBillingBridge = "dev/axmol/app/BillingBridge";
}

void cocos2dx_StoreController::storeOpening() {}
void cocos2dx_StoreController::storeClosing() {}
void cocos2dx_StoreController::initialize(const char*) {}

void cocos2dx_StoreController::buyMarketItem(const char* productId) {
    if (!productId || productId[0] == '\0') {
        AXLOGW("[IAP] buyMarketItem called with empty productId");
        return;
    }
    AXLOGI("[IAP] buyMarketItem request '{}'", productId);
    ax::JniHelper::callStaticVoidMethod(kBillingBridge, "buy", std::string(productId));
}

void cocos2dx_StoreController::restorePurchases() {
    AXLOGI("[IAP] restorePurchases requested");
    ax::JniHelper::callStaticVoidMethod(kBillingBridge, "restore");
}

#else

void cocos2dx_StoreController::storeOpening() {}
void cocos2dx_StoreController::storeClosing() {}
void cocos2dx_StoreController::initialize(const char*) {}

void cocos2dx_StoreController::buyMarketItem(const char* productId) {
    AXLOGW("[IAP] buyMarketItem('{}') ignored (store not available on this platform)",
           productId ? productId : "(null)");
}

void cocos2dx_StoreController::restorePurchases() {
    AXLOGW("[IAP] restorePurchases() ignored (store not available on this platform)");
}

#endif
