//
//  PZLayout.cpp
//
#include "PZLayout.h"
#include "PZGameConfig.h"
#include "PZSettingsController.h"

USING_NS_AX;

namespace PZLayout {

float bannerReserve() {
    PZCfg::ensureLoaded();
    auto* sc = PZSettingsController::shared();
    if (sc && sc->removeAds) return 0.f;
    return PZCfg::kBannerReserve;
}

Rect safeRect() {
    auto* dir = Director::getInstance();
    const Size win = dir->getWinSize();
    Rect safe = dir->getSafeAreaRect();
    if (safe.size.width <= 1.f || safe.size.height <= 1.f) {
        safe = Rect(0, 0, win.width, win.height);
    }
    // Keep a small breathing margin from the physical edges even when the device reports none.
    const float margin = 10.f;
    float left = std::max(safe.getMinX(), margin);
    float right = std::min(safe.getMaxX(), win.width - margin);
    float bottom = std::max(safe.getMinY(), bannerReserve());
    float top = std::min(safe.getMaxY(), win.height - margin);
    return Rect(left, bottom, std::max(1.f, right - left), std::max(1.f, top - bottom));
}

float mapY(float y) {
    const float h = Director::getInstance()->getWinSize().height;
    const float b = bannerReserve();
    if (b <= 0.f || h <= 0.f) return y;
    return b + y * (h - b) / h;
}

void keepInside(Node* node) {
    if (!node) return;
    const Rect safe = safeRect();
    Rect bb = node->getBoundingBox();
    Vec2 shift = Vec2::ZERO;
    if (bb.getMinX() < safe.getMinX()) shift.x = safe.getMinX() - bb.getMinX();
    else if (bb.getMaxX() > safe.getMaxX()) shift.x = safe.getMaxX() - bb.getMaxX();
    if (bb.getMinY() < safe.getMinY()) shift.y = safe.getMinY() - bb.getMinY();
    if (shift.x != 0.f || shift.y != 0.f) node->setPosition(node->getPosition() + shift);
}

Vec2 popupCentre() {
    const Rect s = safeRect();
    return Vec2(s.getMidX(), s.getMidY());
}

float popupMaxHeight() {
    return safeRect().size.height - 16.f;
}

} // namespace PZLayout
