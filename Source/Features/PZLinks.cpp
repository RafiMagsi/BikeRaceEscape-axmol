#include "PZLinks.h"
#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

namespace PZLinks {

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
void pzOpenInAppBrowser(const std::string& url);   // PZLinksApple.mm
#endif

bool openUrl(const std::string& url) {
    if (url.rfind("https://", 0) != 0 && url.rfind("http://", 0) != 0) return false;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    pzOpenInAppBrowser(url);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    ax::JniHelper::callStaticVoidMethod("dev/axmol/app/LinkBridge", "openUrl", url);
#else
    ax::Application::getInstance()->openURL(url);
#endif
    return true;
}

} // namespace PZLinks
