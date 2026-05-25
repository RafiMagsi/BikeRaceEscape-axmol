#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#include <mutex>
#include <string>
#include <unordered_map>

#include "PZLegacyCompat.h"
#include "cocos2dx_StoreController.h"
#include "EventHandler.h"

@interface PZIAPObserver : NSObject <SKPaymentTransactionObserver, SKProductsRequestDelegate>
@end

namespace {

static PZIAPObserver* g_observer = nil;
static std::mutex g_mu;
static std::unordered_map<std::string, SKProductsRequest*> g_requests;
static std::string g_pendingPurchaseProductId;

static EventHandler* g_eventHandler() {
    static EventHandler handler;
    return &handler;
}

static void _dispatchOnMain(void (^block)(void)) {
    if ([NSThread isMainThread]) {
        block();
    } else {
        dispatch_async(dispatch_get_main_queue(), block);
    }
}

static void _firePurchaseStarted(const std::string& productId) {
    std::string id = productId;
    _dispatchOnMain(^{
        std::string tmp = id;
        g_eventHandler()->marketPurchaseProcessStarted(tmp);
    });
}

static void _firePurchaseSucceeded(const std::string& productId) {
    std::string id = productId;
    _dispatchOnMain(^{
        std::string tmp = id;
        g_eventHandler()->marketPurchase(tmp);
    });
}

static void _fireRestoreSucceeded(const std::string& productId) {
    std::string id = productId;
    _dispatchOnMain(^{
        std::string tmp = id;
        g_eventHandler()->transactionRestored(tmp);
    });
}

static void _fireCancelled() {
    _dispatchOnMain(^{
        g_eventHandler()->paymentCancelled();
    });
}

static void _fireError(NSString* message) {
    std::string msg = message ? std::string([message UTF8String]) : std::string("Store error");
    _dispatchOnMain(^{
        std::string tmp = msg;
        g_eventHandler()->unexpectedErrorInStore(tmp);
    });
}

static void _ensureObserverInstalled() {
    if (g_observer) return;
    g_observer = [PZIAPObserver new];
    [[SKPaymentQueue defaultQueue] addTransactionObserver:g_observer];
    AXLOGI("[IAP] StoreKit observer installed");
}

} // namespace

@implementation PZIAPObserver

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response {
    std::string wanted;
    {
        std::lock_guard<std::mutex> lock(g_mu);
        wanted = g_pendingPurchaseProductId;
    }

    SKProduct* product = nil;
    for (SKProduct* p in response.products) {
        if ([p.productIdentifier isEqualToString:[NSString stringWithUTF8String:wanted.c_str()]]) {
            product = p;
            break;
        }
    }

    if (!product) {
        NSString* invalid = response.invalidProductIdentifiers.count > 0 ? [response.invalidProductIdentifiers componentsJoinedByString:@", "] : @"(none)";
        _fireError([NSString stringWithFormat:@"Invalid product id (wanted=%s, invalid=%@)", wanted.c_str(), invalid]);
        return;
    }

    if (![SKPaymentQueue canMakePayments]) {
        _fireError(@"Payments are disabled on this device.");
        return;
    }

    SKPayment* payment = [SKPayment paymentWithProduct:product];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

- (void)request:(SKRequest*)request didFailWithError:(NSError*)error {
    (void)request;
    _fireError(error.localizedDescription ?: @"Store request failed");
}

- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray<SKPaymentTransaction*>*)transactions {
    (void)queue;
    for (SKPaymentTransaction* tx in transactions) {
        NSString* pid = tx.payment.productIdentifier ?: @"";
        std::string productId([pid UTF8String]);

        switch (tx.transactionState) {
            case SKPaymentTransactionStatePurchasing:
                // already reported via marketPurchaseProcessStarted
                break;
            case SKPaymentTransactionStatePurchased:
                _firePurchaseSucceeded(productId);
                [[SKPaymentQueue defaultQueue] finishTransaction:tx];
                break;
            case SKPaymentTransactionStateRestored:
                _fireRestoreSucceeded(productId);
                [[SKPaymentQueue defaultQueue] finishTransaction:tx];
                break;
            case SKPaymentTransactionStateFailed: {
                NSError* err = tx.error;
                if (err && err.code == SKErrorPaymentCancelled) {
                    _fireCancelled();
                } else {
                    _fireError(err.localizedDescription ?: @"Payment failed");
                }
                [[SKPaymentQueue defaultQueue] finishTransaction:tx];
                break;
            }
            case SKPaymentTransactionStateDeferred:
                AXLOGI("[IAP] transaction deferred for '{}'", productId);
                break;
        }
    }
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue*)queue {
    (void)queue;
    AXLOGI("[IAP] restore completed");
}

- (void)paymentQueue:(SKPaymentQueue*)queue restoreCompletedTransactionsFailedWithError:(NSError*)error {
    (void)queue;
    _fireError(error.localizedDescription ?: @"Restore failed");
}

@end

void cocos2dx_StoreController::storeOpening() {
    _dispatchOnMain(^{
        _ensureObserverInstalled();
    });
}

void cocos2dx_StoreController::storeClosing() {
    // Keep observer installed for app lifetime; StoreKit transactions can complete later.
}

void cocos2dx_StoreController::initialize(const char*) {
    // Legacy no-op (Android secret).
    storeOpening();
}

void cocos2dx_StoreController::buyMarketItem(const char* productIdC) {
    if (!productIdC || productIdC[0] == '\0') {
        AXLOGW("[IAP] buyMarketItem called with empty productId");
        return;
    }

    const std::string productId(productIdC);
    AXLOGI("[IAP] buyMarketItem request '{}'", productId);

    _dispatchOnMain(^{
        _ensureObserverInstalled();

        {
            std::lock_guard<std::mutex> lock(g_mu);
            g_pendingPurchaseProductId = productId;
        }

        _firePurchaseStarted(productId);

        NSSet<NSString*>* ids = [NSSet setWithObject:[NSString stringWithUTF8String:productId.c_str()]];
        SKProductsRequest* req = [[SKProductsRequest alloc] initWithProductIdentifiers:ids];
        req.delegate = g_observer;
        [req start];
    });
}

void cocos2dx_StoreController::restorePurchases() {
    _dispatchOnMain(^{
        _ensureObserverInstalled();
        AXLOGI("[IAP] restorePurchases requested");
        [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
    });
}

#endif // iOS

