#pragma once
#include "PZLegacyCompat.h"

class AppDelegate final : private ax::Application {
public:
    AppDelegate();
    ~AppDelegate() override;

    void initGLContextAttrs() override;
    bool applicationDidFinishLaunching() override;
    void applicationDidEnterBackground() override;
    void applicationWillEnterForeground() override;

    static void scheduledLoading();
    static void setDebugModeEnabled(bool enable);
};
