#pragma once

// Legacy IAP/store bridge stub.
// The original project used a commercial store bridge (see `Source/StoreBridge/`), which is excluded from the core port.
// This header keeps call sites compiling; implementation is intentionally a no-op.

class cocos2dx_StoreController {
public:
    static inline void buyMarketItem(const char*) {}
};

