//
//  PZPopups.cpp
//
#include "PZPopups.h"
#include "PZUI.h"
#include "PZSettingsController.h"
#include "SharedData/PZGSharedData.h"
#include "SharedData/PZGSoundData.h"

#include <cmath>
#include <memory>
#include <vector>

USING_NS_AX;

namespace PZPopups {

namespace {
constexpr int kTagMenuWidgets = 0x4D57;

Size winSize() { return Director::getInstance()->getWinSize(); }

// Popups are centred slightly above the middle so they clear the banner ad strip along the bottom edge.
Vec2 popupMid() {
    const Size w = winSize();
    return Vec2(w.width * 0.5f, w.height * 0.5f + PZCfg::kPopupLift);
}

void playTap() {
    // Reuse the game's own UI sound when available.
    auto* gsd = PZGSharedData::sharedInstanse();
    if (!gsd || !gsd->soundResource) return;
    auto* sounds = static_cast<__Array*>(gsd->soundResource->objectForKey("Sounds"));
    if (sounds && sounds->count() > 2) {
        if (auto* s = dynamic_cast<PZGSoundData*>(sounds->objectAtIndex(2))) s->playAsSound(false);
    }
}

void playCoin() {
    auto* gsd = PZGSharedData::sharedInstanse();
    if (!gsd || !gsd->soundResource) return;
    auto* sounds = static_cast<__Array*>(gsd->soundResource->objectForKey("Sounds"));
    if (sounds && sounds->count() > 3) {
        if (auto* s = dynamic_cast<PZGSoundData*>(sounds->objectAtIndex(3))) s->playAsSound(false);
    }
}

// Dismisses `layer` on the next frame, then runs `after` (if any).
void dismiss(Node* layer, std::function<void()> after) {
    layer->retain();
    later([layer, after]() {
        layer->removeFromParent();
        layer->release();
        if (after) after();
    });
}

DrawNode* coinIcon(float r) {
    auto* d = DrawNode::create();
    d->drawSolidCircle(Vec2::ZERO, r, 0.f, 20, PZUI::c4f(PZCfg::kColAccent));
    d->drawCircle(Vec2::ZERO, r, 0.f, 20, false, PZUI::c4f(Color3B(150, 100, 10)), 2.f);
    d->drawCircle(Vec2::ZERO, r * 0.55f, 0.f, 16, false, PZUI::c4f(Color3B(190, 130, 20)), 1.5f);
    return d;
}
} // namespace

void later(std::function<void()> fn) {
    Director::getInstance()->getScheduler()->runOnAxmolThread(std::move(fn));
}

// ================================================================================================
// Revive
// ================================================================================================

Node* showRevive(Node* parent, int cost, int coinsOwned, std::function<void()> onRevive,
                 std::function<void()> onDecline) {
    if (!parent) return nullptr;
    const Size win = winSize();
    const Vec2 mid = popupMid();

    auto* base = PZUI::modalBase(0.74f);
    parent->addChild(base, 300);

    auto* panel = PZUI::roundedRect(470.f, 350.f, 22.f, PZCfg::kColPanel, 0.97f, PZCfg::kColPanelEdge);
    panel->setPosition(mid);
    base->addChild(panel);
    panel->setScale(0.7f);
    panel->runAction(EaseBackOut::create(ScaleTo::create(0.25f, 1.f)));

    auto* title = PZUI::label(PZCfg::kStrReviveTitle, 24.f, PZCfg::kColAccent);
    title->enableOutline(Color4B(0, 0, 0, 220), 3);
    title->setPosition(mid + Vec2(0, 138));
    base->addChild(title);

    auto* ring = DrawNode::create();
    ring->setPosition(mid + Vec2(0, 40));
    base->addChild(ring);
    auto* secs = PZUI::label("5", 34.f, PZCfg::kColText);
    secs->setPosition(mid + Vec2(0, 40));
    base->addChild(secs);

    char buf[48];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrReviveCoins, coinsOwned);
    auto* owned = PZUI::label(buf, 9.f, PZCfg::kColDim);
    owned->setPosition(mid + Vec2(0, -26));
    base->addChild(owned);

    auto done = std::make_shared<bool>(false);
    Node* basePtr = base;
    auto finish = [basePtr, done, onRevive, onDecline](bool accepted) {
        if (*done) return;
        *done = true;
        playTap();
        dismiss(basePtr, accepted ? onRevive : onDecline);
    };

    std::snprintf(buf, sizeof(buf), PZCfg::kStrReviveBtn, cost);
    auto* yes = PZUI::Button::create(buf, 340.f, 56.f, PZCfg::kColGood, [finish]() { finish(true); }, 16.f);
    yes->setPosition(mid + Vec2(0, -78));
    base->addChild(yes);
    yes->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.55f, 1.04f), ScaleTo::create(0.55f, 1.f), nullptr)));

    auto* no = PZUI::Button::create(PZCfg::kStrReviveSkip, 220.f, 40.f, PZCfg::kColDim, [finish]() { finish(false); }, 11.f);
    no->setPosition(mid + Vec2(0, -143));
    base->addChild(no);

    auto left = std::make_shared<float>(PZCfg::kReviveCountdown);
    auto draw = [ring, secs, left]() {
        const float frac = std::max(0.f, *left / PZCfg::kReviveCountdown);
        const Color3B col = *left < 2.f ? PZCfg::kColBad : PZCfg::kColAccent2;
        ring->clear();
        ring->drawCircle(Vec2::ZERO, 46.f, 0.f, 36, false, PZUI::c4f(Color3B(40, 55, 80), 0.9f), 4.f);
        const int segs = 40;
        const int n = static_cast<int>(segs * frac);
        for (int i = 0; i < n; ++i) {
            const float a0 = CC_DEGREES_TO_RADIANS(90.f - 360.f * i / segs);
            const float a1 = CC_DEGREES_TO_RADIANS(90.f - 360.f * (i + 1) / segs);
            ring->drawSegment(Vec2(std::cos(a0), std::sin(a0)) * 46.f, Vec2(std::cos(a1), std::sin(a1)) * 46.f, 2.5f,
                              PZUI::c4f(col));
        }
        char b[8];
        std::snprintf(b, sizeof(b), "%d", static_cast<int>(std::ceil(std::max(0.f, *left))));
        secs->setString(b);
        secs->setColor(col);
    };
    draw();
    base->schedule([left, done, finish, draw](float dt) {
        if (*done) return;
        *left -= dt;
        draw();
        if (*left <= 0.f) finish(false);
    }, 0.f, "pz_revive_tick");
    return base;
}

// ================================================================================================
// Daily reward
// ================================================================================================

Node* showDailyReward(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    const Size win = winSize();
    const Vec2 mid = popupMid();

    auto* base = PZUI::modalBase(0.78f);
    parent->addChild(base, 320);

    auto* panel = PZUI::roundedRect(660.f, 390.f, 24.f, PZCfg::kColPanel, 0.97f, PZCfg::kColPanelEdge);
    panel->setPosition(mid);
    base->addChild(panel);
    panel->setScale(0.75f);
    panel->runAction(EaseBackOut::create(ScaleTo::create(0.28f, 1.f)));

    auto* title = PZUI::label(PZCfg::kStrDailyTitle, 24.f, PZCfg::kColAccent);
    title->enableOutline(Color4B(0, 0, 0, 220), 3);
    title->setPosition(mid + Vec2(0, 152));
    base->addChild(title);

    const int pendingDay = pg->dailyAvailable() ? pg->dailyPendingDay() : pg->streak();
    const int cycleDay = ((std::max(1, pendingDay) - 1) % PZCfg::kDailyRewardCount) + 1;   // 1..N
    const bool available = pg->dailyAvailable();

    char buf[48];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrDailyStreak, available ? std::max(0, pendingDay - 1) : pg->streak());
    auto* streak = PZUI::label(buf, 9.f, PZCfg::kColAccent2);
    streak->setPosition(mid + Vec2(0, 116));
    base->addChild(streak);

    // Day tiles.
    const float tileW = 76.f, gap = 8.f;
    const float total = PZCfg::kDailyRewardCount * tileW + (PZCfg::kDailyRewardCount - 1) * gap;
    auto buildTile = [tileW](Node* tile, int i, bool claimed, bool current) {
        tile->removeAllChildren();
        tile->stopAllActions();
        tile->setScale(1.f);
        char tb[32];
        const Color3B edge = current ? PZCfg::kColAccent : (claimed ? PZCfg::kColGood : PZCfg::kColPanelEdge);
        const Color3B fill = current ? Color3B(70, 56, 12) : (claimed ? Color3B(18, 50, 30) : Color3B(20, 30, 52));
        tile->addChild(PZUI::roundedRect(tileW, 112.f, 10.f, fill, 0.95f, edge, claimed ? 0.6f : 1.f, current ? 3.f : 2.f));
        std::snprintf(tb, sizeof(tb), PZCfg::kStrDailyDay, i);
        auto* d = PZUI::label(tb, 8.f, current ? PZCfg::kColAccent : PZCfg::kColDim);
        d->setPosition(Vec2(0, 42));
        tile->addChild(d);
        auto* icon = coinIcon(14.f);
        icon->setPosition(Vec2(0, 12));
        tile->addChild(icon);
        auto* amt = PZUI::label(std::to_string(PZCfg::kDailyRewards[i - 1]), 12.f, claimed ? PZCfg::kColDim : PZCfg::kColText);
        amt->setPosition(Vec2(0, -22));
        tile->addChild(amt);
        if (claimed) {
            auto* ok = PZUI::label("OK", 9.f, PZCfg::kColGood);
            ok->setPosition(Vec2(0, -42));
            tile->addChild(ok);
        }
        if (current) {
            tile->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.07f), ScaleTo::create(0.5f, 1.f), nullptr)));
        }
    };
    std::vector<Node*> tiles;
    for (int i = 1; i <= PZCfg::kDailyRewardCount; ++i) {
        auto* tile = Node::create();
        const float x = mid.x - total * 0.5f + tileW * 0.5f + (i - 1) * (tileW + gap);
        tile->setPosition(Vec2(x, mid.y + 28.f));
        base->addChild(tile);
        const bool claimed = available ? (i < cycleDay) : (i <= cycleDay);
        buildTile(tile, i, claimed, available && i == cycleDay);
        tiles.push_back(tile);
    }

    auto claimedNow = std::make_shared<bool>(!available);
    auto btnHolder = std::make_shared<PZUI::Button*>(nullptr);
    Node* basePtr = base;
    auto onTap = [=]() {
        if (!*claimedNow) {
            const int got = PZProgress::shared()->claimDaily();
            *claimedNow = true;
            playCoin();
            if (got > 0) {
                PZUI::burst(basePtr, mid + Vec2(0, 28), PZCfg::kColAccent, 22, 160.f, 0.8f, 10);
                char t[32];
                std::snprintf(t, sizeof(t), "+%d", got);
                PZUI::floatText(basePtr, t, mid + Vec2(0, 90), PZCfg::kColAccent, 26.f, 11);
            }
            if (*btnHolder) (*btnHolder)->setText(PZCfg::kStrClose);
            // Reflect the claim in the calendar and streak read-out.
            buildTile(tiles[cycleDay - 1], cycleDay, true, false);
            tiles[cycleDay - 1]->runAction(Sequence::create(ScaleTo::create(0.1f, 1.18f), EaseBackOut::create(ScaleTo::create(0.25f, 1.f)), nullptr));
            char sb[48];
            std::snprintf(sb, sizeof(sb), PZCfg::kStrDailyStreak, PZProgress::shared()->streak());
            streak->setString(sb);
            return;
        }
        playTap();
        dismiss(basePtr, onClosed);
    };
    auto* btn = PZUI::Button::create(available ? PZCfg::kStrDailyClaim : PZCfg::kStrClose, 280.f, 58.f,
                                     available ? PZCfg::kColGood : PZCfg::kColPanelEdge, onTap, 17.f);
    *btnHolder = btn;
    btn->setPosition(mid + Vec2(0, -112));
    base->addChild(btn);
    btn->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.6f, 1.04f), ScaleTo::create(0.6f, 1.f), nullptr)));
    return base;
}

// ================================================================================================
// Missions
// ================================================================================================

namespace {
struct MissionsUI {
    Node* base = nullptr;
    Node* rows = nullptr;
    std::function<void()> onClosed;
};

void buildMissionRows(const std::shared_ptr<MissionsUI>& ui, const Vec2& mid);

void buildMissionRows(const std::shared_ptr<MissionsUI>& ui, const Vec2& mid) {
    if (ui->rows) ui->rows->removeFromParent();
    ui->rows = Node::create();
    ui->base->addChild(ui->rows);
    auto* pg = PZProgress::shared();

    for (int i = 0; i < pg->missionCount(); ++i) {
        const auto& m = pg->mission(i);
        const auto& t = pg->missionTemplate(i);
        const bool complete = pg->missionComplete(i);
        const float y = mid.y + 86.f - i * 78.f;

        auto* row = Node::create();
        row->setPosition(Vec2(mid.x, y));
        ui->rows->addChild(row);
        const Color3B edge = m.claimed ? PZCfg::kColDim : (complete ? PZCfg::kColGood : PZCfg::kColPanelEdge);
        row->addChild(PZUI::roundedRect(640.f, 70.f, 12.f, Color3B(20, 30, 52), 0.95f, edge, m.claimed ? 0.5f : 1.f));

        auto* text = PZUI::label(pg->missionText(i), 10.f, m.claimed ? PZCfg::kColDim : PZCfg::kColText);
        text->setAnchorPoint(Vec2(0.f, 0.5f));
        text->setPosition(Vec2(-300.f, 14.f));
        row->addChild(text);

        const int shown = std::min(m.progress, t.target);
        auto* barNode = PZUI::bar(190.f, 11.f, static_cast<float>(shown) / t.target, complete ? PZCfg::kColGood : PZCfg::kColAccent2);
        barNode->setPosition(Vec2(-205.f, -14.f));
        row->addChild(barNode);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d/%d", shown, t.target);
        auto* prog = PZUI::label(buf, 8.f, PZCfg::kColDim);
        prog->setAnchorPoint(Vec2(0.f, 0.5f));
        prog->setPosition(Vec2(-100.f, -14.f));
        row->addChild(prog);

        auto* icon = coinIcon(9.f);
        icon->setPosition(Vec2(105.f, 0.f));
        row->addChild(icon);
        auto* reward = PZUI::label("+" + std::to_string(t.reward), 12.f, PZCfg::kColAccent);
        reward->setAnchorPoint(Vec2(0.f, 0.5f));
        reward->setPosition(Vec2(120.f, 0.f));
        row->addChild(reward);

        if (m.claimed) {
            auto* d = PZUI::label(PZCfg::kStrMissionDone, 10.f, PZCfg::kColGood);
            d->setPosition(Vec2(245.f, 0.f));
            row->addChild(d);
        } else {
            auto* b = PZUI::Button::create(PZCfg::kStrMissionClaim, 120.f, 44.f, complete ? PZCfg::kColGood : PZCfg::kColDim,
                                           [ui, mid, i]() {
                                               if (PZProgress::shared()->claimMission(i) > 0) {
                                                   playCoin();
                                                   PZUI::burst(ui->base, Vec2(mid.x + 245.f, mid.y + 86.f - i * 78.f),
                                                               PZCfg::kColAccent, 14, 90.f, 0.6f, 10);
                                                   later([ui, mid]() { buildMissionRows(ui, mid); });
                                               }
                                           }, 11.f);
            b->setPosition(Vec2(245.f, 0.f));
            b->setActive(complete);
            row->addChild(b);
        }
    }
}
} // namespace

Node* showMissions(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    pg->refreshMissions();
    const Size win = winSize();
    const Vec2 mid = popupMid();

    auto ui = std::make_shared<MissionsUI>();
    ui->base = PZUI::modalBase(0.78f);
    ui->onClosed = onClosed;
    parent->addChild(ui->base, 320);

    auto* panel = PZUI::roundedRect(700.f, 410.f, 24.f, PZCfg::kColPanel, 0.97f, PZCfg::kColPanelEdge);
    panel->setPosition(mid);
    ui->base->addChild(panel);
    panel->setScale(0.75f);
    panel->runAction(EaseBackOut::create(ScaleTo::create(0.28f, 1.f)));

    auto* title = PZUI::label(PZCfg::kStrMissionsTitle, 22.f, PZCfg::kColAccent);
    title->enableOutline(Color4B(0, 0, 0, 220), 3);
    title->setPosition(mid + Vec2(0, 172));
    ui->base->addChild(title);

    buildMissionRows(ui, mid);

    char buf[48];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrMissionNew, pg->timeUntilNewMissions().c_str());
    auto* foot = PZUI::label(buf, 8.f, PZCfg::kColDim);
    foot->setPosition(mid + Vec2(0, -124.f));
    ui->base->addChild(foot);

    Node* basePtr = ui->base;
    auto* close = PZUI::Button::create(PZCfg::kStrClose, 200.f, 40.f, PZCfg::kColPanelEdge,
                                       [basePtr, onClosed]() { playTap(); dismiss(basePtr, onClosed); }, 13.f);
    close->setPosition(mid + Vec2(0, -163.f));
    ui->base->addChild(close);
    return ui->base;
}

// ================================================================================================
// Game-over results
// ================================================================================================

void attachResults(Node* gameOverLayer, Label* distanceLabel, const PZRunStats& stats, const PZRunResult& r) {
    if (!gameOverLayer) return;
    const Size win = winSize();

    // Score count-up.
    if (distanceLabel) {
        const int target = stats.distance;
        distanceLabel->setString("0");
        distanceLabel->runAction(Sequence::create(
            DelayTime::create(0.25f),
            ActionFloat::create(0.9f, 0.f, static_cast<float>(target),
                                [distanceLabel](float v) {
                                    char b[16];
                                    std::snprintf(b, sizeof(b), "%d", static_cast<int>(v));
                                    distanceLabel->setString(b);
                                }),
            nullptr));
    }

    auto* root = Node::create();
    root->setTag(0x5253);
    gameOverLayer->addChild(root, 400);

    const float cx = win.width * 0.5f;
    char buf[64];

    // Layout: headline just under the "GAME OVER" art, the rest in the free space flanking the two round buttons
    // (the bottom strip is covered by the banner ad).
    const float headY = win.height * 0.535f;
    const float leftX = win.width * 0.255f, rightX = win.width * 0.745f;
    const float rowY = win.height * 0.355f;

    if (r.newBest) {
        auto* nb = PZUI::label(PZCfg::kStrNewBest, 22.f, PZCfg::kColAccent);
        nb->enableOutline(Color4B(0, 0, 0, 230), 3);
        nb->setPosition(Vec2(cx, headY));
        nb->setScale(0.1f);
        root->addChild(nb);
        nb->runAction(Sequence::create(
            DelayTime::create(1.0f),
            EaseBackOut::create(ScaleTo::create(0.35f, 1.f)),
            CallFunc::create([root, nb]() { PZUI::burst(root, nb->getPosition(), PZCfg::kColAccent, 26, 150.f, 0.9f, 20); }),
            RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.08f), ScaleTo::create(0.5f, 1.f), nullptr)),
            nullptr));
    } else {
        std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsBest, r.best);
        auto* bl = PZUI::label(buf, 12.f, PZCfg::kColDim);
        bl->enableOutline(Color4B(0, 0, 0, 230), 2);
        bl->setPosition(Vec2(cx, headY));
        root->addChild(bl);
    }

    // Coins earned (left of the buttons).
    std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsCoins, stats.coins);
    auto* coins = PZUI::label(buf, 12.f, PZCfg::kColAccent);
    coins->enableOutline(Color4B(0, 0, 0, 230), 2);
    coins->setPosition(Vec2(leftX, rowY));
    root->addChild(coins);

    // Level + XP bar (right of the buttons).
    std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsLevel, r.levelBefore);
    auto* lv = PZUI::label(buf, 11.f, PZCfg::kColAccent2);
    lv->enableOutline(Color4B(0, 0, 0, 230), 2);
    lv->setPosition(Vec2(rightX, rowY + 14.f));
    root->addChild(lv);

    auto* xpBar = PZUI::bar(190.f, 12.f, r.fracBefore, PZCfg::kColAccent2);
    xpBar->setPosition(Vec2(rightX, rowY - 14.f));
    root->addChild(xpBar);

    const bool leveled = r.levelAfter > r.levelBefore;
    const float endFrac = leveled ? 1.f : r.fracAfter;
    xpBar->runAction(Sequence::create(
        DelayTime::create(0.6f),
        ActionFloat::create(0.8f, r.fracBefore, endFrac,
                            [xpBar](float v) { PZUI::redrawBar(xpBar, 190.f, 12.f, v, PZCfg::kColAccent2); }),
        CallFunc::create([=]() {
            if (!leveled) return;
            char b[48];
            std::snprintf(b, sizeof(b), PZCfg::kStrResultsLevel, r.levelAfter);
            lv->setString(b);
            std::snprintf(b, sizeof(b), PZCfg::kStrLevelUp, r.levelUpCoins);
            PZUI::floatText(root, b, Vec2(rightX, rowY + 44.f), PZCfg::kColAccent2, 11.f, 30);
            PZUI::burst(root, xpBar->getPosition(), PZCfg::kColAccent2, 22, 140.f, 0.8f, 20);
            PZUI::redrawBar(xpBar, 190.f, 12.f, 0.f, PZCfg::kColAccent2);
            xpBar->runAction(ActionFloat::create(0.6f, 0.f, r.fracAfter,
                                                 [xpBar](float v) { PZUI::redrawBar(xpBar, 190.f, 12.f, v, PZCfg::kColAccent2); }));
        }),
        nullptr));

    // Missions completed during this run.
    auto* pg = PZProgress::shared();
    float delay = 1.3f;
    for (int slot : r.completedMissions) {
        const std::string text = pg->missionText(slot);
        root->runAction(Sequence::create(DelayTime::create(delay), CallFunc::create([=]() {
            PZUI::banner(root, PZCfg::kStrMissionToast, text, PZCfg::kColGood, win.height * 0.47f, 1.4f, 30);
            playCoin();
        }), nullptr));
        delay += 1.9f;
    }
}

// ================================================================================================
// Main-menu widgets
// ================================================================================================

namespace {
class MenuWidgets : public Node {
public:
    static MenuWidgets* make(Node* host) {
        auto* w = new MenuWidgets();
        if (w->init()) {
            w->autorelease();
            w->_host = host;
            w->build();
            return w;
        }
        delete w;
        return nullptr;
    }

    void refresh() {
        auto* pg = PZProgress::shared();
        pg->refreshMissions();
        char b[48];
        std::snprintf(b, sizeof(b), PZCfg::kStrLevelPlate, pg->level());
        _lv->setString(b);
        PZUI::redrawBar(_xp, 150.f, 10.f, pg->levelFraction(), PZCfg::kColAccent2);
        std::snprintf(b, sizeof(b), PZCfg::kStrBest, pg->bestScore());
        _best->setString(b);
        if (_daily) _daily->setBadge(pg->dailyAvailable() ? 1 : 0);
        if (_missions) _missions->setBadge(pg->unclaimedMissions());
    }

private:
    void build() {
        const Size win = winSize();
        auto* pg = PZProgress::shared();

        // Player plate.
        auto* plate = Node::create();
        // Top-right column: the bottom of the menu is covered by the banner ad, the top-left by the promo ribbon.
        addChild(plate);
        plate->addChild(PZUI::roundedRect(210.f, 74.f, 12.f, PZCfg::kColPanel, 0.82f, PZCfg::kColPanelEdge, 0.9f, 2.f), -1);
        plate->setPosition(Vec2(win.width - 24.f - 105.f, win.height - 24.f - 37.f));
        _lv = PZUI::label("", 12.f, PZCfg::kColAccent);
        _lv->setAnchorPoint(Vec2(0.f, 0.5f));
        _lv->setPosition(Vec2(-92.f, 16.f));
        plate->addChild(_lv);
        _best = PZUI::label("", 8.f, PZCfg::kColDim);
        _best->setAnchorPoint(Vec2(1.f, 0.5f));
        _best->setPosition(Vec2(92.f, 16.f));
        plate->addChild(_best);
        _xp = PZUI::bar(150.f, 10.f, pg->levelFraction(), PZCfg::kColAccent2);
        _xp->setPosition(Vec2(0.f, -14.f));
        plate->addChild(_xp);

        // Buttons, stacked under the plate.
        Node* host = _host;
        if (PZCfg::kEnableMissions) {
            _missions = PZUI::Button::create(PZCfg::kStrMissionsBtn, 210.f, 50.f, PZCfg::kColAccent2, [this, host]() {
                PZPopups::showMissions(host, [this]() { refresh(); });
            }, 13.f);
            _missions->setPosition(Vec2(win.width - 24.f - 105.f, win.height - 24.f - 74.f - 14.f - 25.f - 62.f));
            addChild(_missions);
        }
        if (PZCfg::kEnableDailyReward) {
            _daily = PZUI::Button::create(PZCfg::kStrDailyBtn, 210.f, 50.f, PZCfg::kColAccent, [this, host]() {
                PZPopups::showDailyReward(host, [this]() { refresh(); });
            }, 13.f);
            _daily->setPosition(Vec2(win.width - 24.f - 105.f, win.height - 24.f - 74.f - 14.f - 25.f));
            addChild(_daily);
        }
        refresh();
    }

    Node* _host = nullptr;
    Label* _lv = nullptr;
    Label* _best = nullptr;
    DrawNode* _xp = nullptr;
    PZUI::Button* _daily = nullptr;
    PZUI::Button* _missions = nullptr;
};
} // namespace

void attachMenuWidgets(Node* menuLayer) {
    if (!menuLayer) return;
    menuLayer->removeChildByTag(kTagMenuWidgets);
    auto* w = MenuWidgets::make(menuLayer);
    if (!w) return;
    w->setTag(kTagMenuWidgets);
    menuLayer->addChild(w, 450);
}

void refreshMenuWidgets(Node* menuLayer, bool allowDailyPopup) {
    if (!menuLayer) return;
    auto* w = dynamic_cast<MenuWidgets*>(menuLayer->getChildByTag(kTagMenuWidgets));
    if (!w) {
        attachMenuWidgets(menuLayer);
        w = dynamic_cast<MenuWidgets*>(menuLayer->getChildByTag(kTagMenuWidgets));
    }
    if (w) w->refresh();
    if (allowDailyPopup && PZProgress::shared()->dailyAvailable()) {
        menuLayer->runAction(Sequence::create(DelayTime::create(0.7f), CallFunc::create([menuLayer]() {
            if (PZProgress::shared()->dailyAvailable()) {
                showDailyReward(menuLayer, [menuLayer]() { refreshMenuWidgets(menuLayer, false); });
            }
        }), nullptr));
    }
}

} // namespace PZPopups
