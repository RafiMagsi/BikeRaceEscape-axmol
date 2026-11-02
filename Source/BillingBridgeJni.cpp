#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include <jni.h>

#include <string>

#include "EventHandler.h"

namespace {

std::string jstringToStdString(JNIEnv* env, jstring value) {
    if (!value) return {};
    const char* chars = env->GetStringUTFChars(value, nullptr);
    if (!chars) return {};
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    return result;
}

void runOnGameThread(std::string productId, std::string status) {
    ax::Director* director = ax::Director::getInstance();
    if (!director) return;
    ax::Scheduler* scheduler = director->getScheduler();
    if (!scheduler) return;

    scheduler->runOnAxmolThread(
        [productId = std::move(productId), status = std::move(status)]() {
            static EventHandler handler;
            std::string id = productId;
            if (status == "purchased") {
                handler.marketPurchase(id);
            } else if (status == "restored") {
                handler.transactionRestored(id);
            } else if (status == "cancelled") {
                handler.paymentCancelled();
            } else if (status == "pending") {
                AXLOGI("[IAP] purchase pending for '{}'", productId);
            } else {
                AXLOGW("[IAP] unhandled purchase status '{}' for '{}'", status, productId);
            }
        });
}

} // namespace

extern "C" {

JNIEXPORT void JNICALL Java_dev_axmol_app_BillingBridge_nativeOnPurchaseResult(
    JNIEnv* env, jclass, jstring productId, jstring status) {
    runOnGameThread(jstringToStdString(env, productId), jstringToStdString(env, status));
}

JNIEXPORT void JNICALL Java_dev_axmol_app_BillingBridge_nativeOnBillingError(
    JNIEnv* env, jclass, jstring message) {
    std::string text = jstringToStdString(env, message);
    AXLOGW("[IAP] billing error: {}", text);
    ax::Director* director = ax::Director::getInstance();
    if (!director) return;
    ax::Scheduler* scheduler = director->getScheduler();
    if (!scheduler) return;

    scheduler->runOnAxmolThread([text = std::move(text)]() {
        static EventHandler handler;
        std::string message = text;
        handler.unexpectedErrorInStore(message);
    });
}

} // extern "C"

#endif // CC_PLATFORM_ANDROID
