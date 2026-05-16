#pragma once

// Legacy store bridge event handler interface stub.
// The original project used Soomla's Cocos2d-x store bridge. The core Axmol port excludes it for now.

#include <string>

class IEventHandler {
public:
    virtual ~IEventHandler() = default;

    virtual void marketPurchase(std::string&) {}
    virtual void transactionRestored(std::string&) {}
    virtual void marketRefund(std::string&) {}
    virtual void virtualGoodPurchased(std::string&) {}
    virtual void virtualGoodEquipped(std::string&) {}
    virtual void virtualGoodUnequipped(std::string&) {}
    virtual void billingSupported() {}
    virtual void billingNotSupported() {}
    virtual void marketPurchaseProcessStarted(std::string&) {}
    virtual void goodsPurchaseProcessStarted() {}
    virtual void closingStore() {}
    virtual void unexpectedErrorInStore(std::string&) {}
    virtual void paymentCancelled() {}
    virtual void openingStore() {}
    virtual void currencyBalanceChanged(std::string&, int) {}
    virtual void goodBalanceChanged(std::string&, int) {}
};

