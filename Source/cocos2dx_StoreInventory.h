#pragma once

// Legacy store inventory stub.
// The original project used a Soomla-based store inventory API. Not migrated in the core Axmol port.

class cocos2dx_StoreInventory {
public:
    static inline int getCurrencyBalance(const char*) { return 0; }
};

