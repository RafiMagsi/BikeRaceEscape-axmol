#pragma once

// Minimal in-app purchase bridge for this Axmol port.
//
// The legacy project used Soomla's Cocos2d-x store bridge. In this repo we keep
// the existing call sites but implement iOS purchases via StoreKit.

class cocos2dx_StoreController {
public:
    // Start/stop hooks (safe to call multiple times).
    static void storeOpening();
    static void storeClosing();

    // Legacy init hook (Android used a custom secret). No-op on iOS.
    static void initialize(const char* customSecret);

    // Purchase a product by its StoreKit product identifier.
    static void buyMarketItem(const char* productId);

    // Restore non-consumable purchases.
    static void restorePurchases();
};

