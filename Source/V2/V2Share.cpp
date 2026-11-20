#include "V2/V2Share.h"
#include "V2/V2Challenge.h"
#include "V2/V2Config.h"
#include "V2/V2Content.h"
#include "V2/V2Daily.h"
#include "Features/PZUI.h"
#include "Features/PZGameConfig.h"
#include "PZSettingsController.h"
#include "SharedData/PZGSharedData.h"
#include "SharedData/PZGArtCharacter.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

#include <cstdio>

USING_NS_AX;

namespace V2 {
namespace Share {

namespace {

std::string withCommas(int v) {
    std::string s = std::to_string(v), out;
    for (size_t i = 0; i < s.size(); ++i) {
        out += s[i];
        const size_t left = s.size() - 1 - i;
        if (left > 0 && left % 3 == 0) out += ',';
    }
    return out;
}

const char* medalName(int m) {
    static const char* n[] = {"NO MEDAL", "BRONZE", "SILVER", "GOLD"};
    return n[std::max(0, std::min(3, m))];
}

Color3B medalColor(int m) {
    switch (m) {
    case 3: return Color3B(255, 205, 50);
    case 2: return Color3B(214, 222, 236);
    case 1: return Color3B(205, 127, 50);
    default: return Color3B(110, 116, 132);
    }
}

// The player's own bike sprite (same lookup the menu uses), or nullptr.
Sprite* playerBike() {
    auto* gsd = PZGSharedData::sharedInstanse();
    if (!gsd || !gsd->artResource) return nullptr;
    auto* characters = static_cast<__Array*>(gsd->artResource->objectForKey("Characters"));
    if (!characters || characters->count() <= 0) return nullptr;
    const int id = PZSettingsController::shared()->characterId;
    const int idx = std::min(std::max(0, id), static_cast<int>(characters->count()) - 1);
    if (auto* art = dynamic_cast<PZGArtCharacter*>(characters->objectAtIndex(idx))) return art->getResource();
    return nullptr;
}

Node* buildCard(const Daily::Result& r, const Daily::Modifier& mod) {
    const float W = 1200.f, H = 630.f;
    auto* root = Node::create();
    root->setContentSize(Size(W, H));

    auto* bg = LayerGradient::create(Color4B(10, 16, 46, 255), Color4B(70, 40, 104, 255), Vec2(0.4f, 1.f));
    bg->setContentSize(Size(W, H));
    root->addChild(bg, 0);

    auto* glow = DrawNode::create();
    for (int i = 0; i < 8; ++i) {
        glow->drawSolidCircle(Vec2(240.f, 60.f), 520.f - i * 58.f, 0.f, 48, Color4F(1.f, 0.46f, 0.1f, 0.035f));
        glow->drawSolidCircle(Vec2(1000.f, 560.f), 460.f - i * 50.f, 0.f, 48, Color4F(0.25f, 0.45f, 1.f, 0.04f));
    }
    root->addChild(glow, 1);
    auto* stars = DrawNode::create();
    unsigned s = 7u;
    for (int i = 0; i < 40; ++i) {
        s = s * 1664525u + 1013904223u;
        const float x = (s >> 8) % 1200;
        s = s * 1664525u + 1013904223u;
        const float y = (s >> 8) % 630;
        stars->drawDot(Vec2(x, y), 1.5f + (s % 3), Color4F(1.f, 0.92f, 0.7f, 0.5f));
    }
    root->addChild(stars, 1);

    auto* title = PZUI::heading("DINO BIKE ESCAPE", 34.f, Color3B(255, 222, 80));
    title->setAnchorPoint(Vec2(0.f, 0.5f));
    title->setPosition(Vec2(64.f, H - 70.f));
    root->addChild(title, 5);
    const std::string sub = "DAILY CHALLENGE  -  " + Daily::dateLabel(r.dateKey);
    auto* subL = PZUI::heading(sub, 15.f, Color3B(230, 238, 255));
    subL->setAnchorPoint(Vec2(0.f, 0.5f));
    subL->setPosition(Vec2(64.f, H - 118.f));
    root->addChild(subL, 5);
    auto* modL = PZUI::heading(mod.title, 17.f, Color3B(120, 220, 255));
    modL->setAnchorPoint(Vec2(0.f, 0.5f));
    modL->setPosition(Vec2(64.f, H - 160.f));
    root->addChild(modL, 5);

    auto* score = PZUI::heading(withCommas(r.score), 78.f, Color3B(255, 255, 255));
    score->setAnchorPoint(Vec2(0.f, 0.5f));
    score->setPosition(Vec2(64.f, 330.f));
    root->addChild(score, 5);
    auto* pts = PZUI::heading("POINTS", 18.f, Color3B(255, 190, 60));
    pts->setAnchorPoint(Vec2(0.f, 0.5f));
    pts->setPosition(Vec2(64.f, 250.f));
    root->addChild(pts, 5);

    char line[96];
    std::snprintf(line, sizeof(line), "%d M   -   %d COINS   -   %d EVENTS", r.distance, r.coins, r.eventsCleared);
    auto* det = PZUI::heading(line, 14.f, Color3B(214, 222, 240));
    det->setAnchorPoint(Vec2(0.f, 0.5f));
    det->setPosition(Vec2(64.f, 196.f));
    root->addChild(det, 5);

    // medal badge
    const Color3B mc = medalColor(r.medal);
    auto* badge = DrawNode::create();
    badge->drawSolidCircle(Vec2::ZERO, 92.f, 0.f, 48, Color4F(0.f, 0.f, 0.f, 0.35f));
    badge->drawSolidCircle(Vec2::ZERO, 84.f, 0.f, 48, Color4F(mc.r / 255.f, mc.g / 255.f, mc.b / 255.f, 1.f));
    badge->drawSolidCircle(Vec2::ZERO, 66.f, 0.f, 48, Color4F(mc.r / 255.f * 0.78f, mc.g / 255.f * 0.78f, mc.b / 255.f * 0.78f, 1.f));
    badge->drawCircle(Vec2::ZERO, 84.f, 0.f, 48, false, Color4F(0.1f, 0.06f, 0.04f, 1.f), 3.f);
    badge->setPosition(Vec2(1010.f, 470.f));
    root->addChild(badge, 4);
    auto* medalL = PZUI::heading(medalName(r.medal), 17.f, Color3B(255, 255, 255));
    medalL->setPosition(Vec2(1010.f, 470.f));
    if (medalL->getContentSize().width > 120.f) medalL->setScale(120.f / medalL->getContentSize().width);
    root->addChild(medalL, 6);

    if (Sprite* bike = playerBike()) {
        const float h = std::max(1.f, bike->getContentSize().height);
        bike->setAnchorPoint(Vec2(0.5f, 0.f));
        bike->setScale(250.f / h);
        bike->setPosition(Vec2(930.f, 70.f));
        root->addChild(bike, 4);
    }

    auto* pill = PZUI::roundedRect(430.f, 64.f, 32.f, Color3B(236, 120, 30), 1.f, Color3B(255, 236, 190), 1.f, 3.f);
    pill->setPosition(Vec2(64.f + 215.f, 80.f));
    root->addChild(pill, 4);
    auto* beat = PZUI::heading("CAN YOU BEAT THIS?", 22.f, Color3B(255, 255, 255));
    beat->setPosition(Vec2(64.f + 215.f, 80.f));
    root->addChild(beat, 6);

    auto* url = PZUI::heading(PZCfg::kShareUrl, 11.f, Color3B(190, 200, 224));
    url->setAnchorPoint(Vec2(1.f, 0.5f));
    url->setPosition(Vec2(W - 40.f, 26.f));
    root->addChild(url, 5);
    return root;
}

} // namespace

std::string shareText(const Daily::Result& r) {
    Challenge::Token t;
    t.date = r.dateKey;
    t.rules = cfg().rulesVersion;
    t.content = Content::shared().contentVersion();
    t.modifier = Daily::modifierFor(r.dateKey).index;
    t.score = r.score;
    t.medal = r.medal;
    char buf[480];
    std::snprintf(buf, sizeof(buf), "I scored %s%s%s on the Dino Bike Escape road of %s! Beat it on the same road: %s",
                  withCommas(r.score).c_str(), r.medal ? " and won " : "", r.medal ? medalName(r.medal) : "",
                  Daily::dateLabel(r.dateKey).c_str(), Challenge::makeUrl(t).c_str());
    return buf;
}

void shareDailyResult(Node* host, const Daily::Result& r, std::function<void(const std::string&)> done) {
    const Daily::Modifier mod = Daily::modifierFor(r.dateKey);
    Node* card = buildCard(r, mod);
    auto* rt = RenderTexture::create(1200, 630);
    if (!rt || !host) {
        if (done) done("");
        return;
    }
    rt->retain();
    card->retain();
    rt->beginWithClear(0.04f, 0.06f, 0.18f, 1.f);
    card->visit();
    rt->end();
    const std::string path = FileUtils::getInstance()->getWritablePath() + "dino_daily_card.png";
    const std::string text = shareText(r);
    // newImage reads the finished texture back (async) and does not depend on the node being drawn again.
    rt->newImage([rt, card, path, text, done](RefPtr<Image> img) {
        bool ok = img && img->saveToFile(path, false);
        rt->release();
        card->release();
        AXLOGI("[V2:share] card {} {}", ok ? "saved" : "FAILED", path);
        if (done) done(ok ? path : std::string());
        if (ok) sendImage(path, text);
    });
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
void sendImage(const std::string& path, const std::string& text) {
    JniHelper::callStaticVoidMethod("dev/axmol/app/ShareBridge", "shareImage", path, text);
}
#elif (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
void sendImage(const std::string& path, const std::string& text) {
    AXLOGI("[V2:share] (no share sheet on this platform) {} :: {}", path, text);
}
#endif
// iOS: V2ShareApple.mm

} // namespace Share
} // namespace V2
