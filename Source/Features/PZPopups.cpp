//
//  PZPopups.cpp
//
#include "PZPopups.h"
#include "PZUI.h"
#include "PZLayout.h"
#include "PZHaptics.h"
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

void playSound(int index) {
    auto* gsd = PZGSharedData::sharedInstanse();
    if (!gsd || !gsd->soundResource) return;
    auto* sounds = static_cast<__Array*>(gsd->soundResource->objectForKey("Sounds"));
    if (sounds && sounds->count() > index) {
        if (auto* s = dynamic_cast<PZGSoundData*>(sounds->objectAtIndex(index))) s->playAsSound(false);
    }
}
void playCoin() { playSound(3); }

// Dismisses `layer` on the next frame, then runs `after` (if any).
void dismiss(Node* layer, std::function<void()> after) {
    layer->retain();
    later([layer, after]() {
        layer->removeFromParent();
        layer->release();
        if (after) after();
    });
}

// Modal scaffold: dim base + original-style panel, popping in.
LayerColor* openModal(Node* parent, float w, float h, int z, float dim) {
    auto* base = PZUI::modalBase(dim);
    parent->addChild(base, z);
    const Vec2 mid = PZLayout::popupCentre();
    h = std::min(h, PZLayout::popupMaxHeight());
    auto* panel = PZUI::panel(w, h);
    panel->setPosition(mid);
    base->addChild(panel);
    panel->setScale(0.75f);
    panel->runAction(EaseBackOut::create(ScaleTo::create(0.26f, 1.f)));
    return base;
}
} // namespace

void later(std::function<void()> fn) {
    Director::getInstance()->getScheduler()->runOnAxmolThread(std::move(fn));
}

// ================================================================================================
// Revive (coins or rewarded video)
// ================================================================================================

Node* showRevive(Node* parent, int cost, int coinsOwned, bool videoAvailable, std::function<void()> onCoins,
                 std::function<void()> onVideo, std::function<void()> onDecline) {
    if (!parent) return nullptr;
    const Vec2 mid = PZLayout::popupCentre();
    auto* base = openModal(parent, 520.f, videoAvailable ? 410.f : 350.f, 300, 0.76f);

    auto* title = PZUI::heading(PZCfg::kStrReviveTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, videoAvailable ? 172.f : 142.f));
    base->addChild(title);

    const float ringY = videoAvailable ? 100.f : 70.f;
    auto* ring = DrawNode::create();
    ring->setPosition(mid + Vec2(0, ringY));
    base->addChild(ring);
    auto* secs = PZUI::heading("5", 30.f, PZCfg::kColText);
    secs->setPosition(mid + Vec2(0, ringY));
    base->addChild(secs);

    char buf[64];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrReviveCoins, coinsOwned);
    auto* owned = PZUI::heading(buf, 8.f, PZCfg::kColDim);
    owned->setPosition(mid + Vec2(0, ringY - 56.f));
    base->addChild(owned);

    enum Choice { Coins, Video, Decline };
    auto done = std::make_shared<bool>(false);
    Node* basePtr = base;
    auto finish = [basePtr, done, onCoins, onVideo, onDecline](Choice c) {
        if (*done) return;
        *done = true;
        dismiss(basePtr, c == Coins ? onCoins : (c == Video ? onVideo : onDecline));
    };

    const bool canAfford = coinsOwned >= cost;
    float y = ringY - 112.f;

    std::snprintf(buf, sizeof(buf), canAfford ? PZCfg::kStrReviveBtn : PZCfg::kStrReviveNoCoins, cost);
    auto* coinBtn = PZUI::Button::create(buf, 400.f, 52.f, PZCfg::kColGood, [finish]() { finish(Coins); }, 15.f);
    coinBtn->setPosition(mid + Vec2(0, y));
    coinBtn->setActive(canAfford);
    base->addChild(coinBtn);
    auto* coinIcon = PZUI::coinIcon(26.f);
    coinIcon->setPosition(Vec2(400.f - 36.f, 26.f));
    coinBtn->addChild(coinIcon, 5);
    if (canAfford) {
        coinBtn->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.55f, 1.03f), ScaleTo::create(0.55f, 1.f), nullptr)));
    }
    y -= 62.f;

    if (videoAvailable) {
        std::snprintf(buf, sizeof(buf), "%s  %s", PZCfg::kStrReviveVideo, PZCfg::kStrReviveFree);
        auto* vidBtn = PZUI::Button::create(buf, 400.f, 52.f, PZCfg::kColButton, [finish]() { finish(Video); }, 14.f);
        vidBtn->setPosition(mid + Vec2(0, y));
        base->addChild(vidBtn);
        // Play glyph.
        auto* tri = DrawNode::create();
        Vec2 pts[3] = {Vec2(-8, -10), Vec2(-8, 10), Vec2(10, 0)};
        tri->drawSolidPoly(pts, 3, PZUI::c4f(Color3B(255, 255, 255)), 2.f, PZUI::c4f(Color3B(90, 30, 0)), true);
        tri->setPosition(Vec2(34.f, 26.f));
        vidBtn->addChild(tri, 5);
        y -= 62.f;
    }

    auto* no = PZUI::Button::create(PZCfg::kStrReviveSkip, 230.f, 38.f, Color3B(120, 124, 134), [finish]() { finish(Decline); }, 10.f);
    no->setPosition(mid + Vec2(0, y - 4.f));
    base->addChild(no);

    auto left = std::make_shared<float>(PZCfg::kReviveCountdown);
    auto draw = [ring, secs, left]() {
        const float frac = std::max(0.f, *left / PZCfg::kReviveCountdown);
        const Color3B col = *left < 2.f ? PZCfg::kColBad : PZCfg::kColAccent;
        ring->clear();
        ring->drawCircle(Vec2::ZERO, 40.f, 0.f, 36, false, PZUI::c4f(Color3B(40, 34, 28), 0.9f), 6.f);
        const int segs = 40;
        const int n = static_cast<int>(segs * frac);
        for (int i = 0; i < n; ++i) {
            const float a0 = CC_DEGREES_TO_RADIANS(90.f - 360.f * i / segs);
            const float a1 = CC_DEGREES_TO_RADIANS(90.f - 360.f * (i + 1) / segs);
            ring->drawSegment(Vec2(std::cos(a0), std::sin(a0)) * 40.f, Vec2(std::cos(a1), std::sin(a1)) * 40.f, 3.f, PZUI::c4f(col));
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
        if (*left <= 0.f) finish(Decline);
    }, 0.f, "pz_revive_tick");
    PZHaptics::play(PZHaptics::Kind::Warning);
    return base;
}

// ================================================================================================
// Daily reward
// ================================================================================================

Node* showDailyReward(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    const Vec2 mid = PZLayout::popupCentre();
    auto* base = openModal(parent, 680.f, 400.f, 320, 0.78f);

    auto* title = PZUI::heading(PZCfg::kStrDailyTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, 152));
    base->addChild(title);

    const int rewardCount = static_cast<int>(PZCfg::kDailyRewards.size());
    const int pendingDay = pg->dailyAvailable() ? pg->dailyPendingDay() : pg->streak();
    const int cycleDay = ((std::max(1, pendingDay) - 1) % rewardCount) + 1;   // 1..N
    const bool available = pg->dailyAvailable();

    char buf[64];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrDailyStreak, available ? std::max(0, pendingDay - 1) : pg->streak());
    auto* streak = PZUI::heading(buf, 9.f, PZCfg::kColAccent2);
    streak->setPosition(mid + Vec2(0, 114));
    base->addChild(streak);

    const float tileW = 76.f, gap = 8.f;
    const float total = rewardCount * tileW + (rewardCount - 1) * gap;
    auto buildTile = [tileW](Node* tile, int i, bool claimed, bool current) {
        tile->removeAllChildren();
        tile->stopAllActions();
        tile->setScale(1.f);
        char tb[32];
        const Color3B edge = current ? PZCfg::kColAccent : (claimed ? PZCfg::kColGood : PZCfg::kColPanelEdge);
        const Color3B fill = current ? Color3B(78, 56, 16) : (claimed ? Color3B(22, 52, 30) : Color3B(30, 34, 52));
        tile->addChild(PZUI::roundedRect(tileW, 112.f, 10.f, fill, 0.96f, edge, claimed ? 0.7f : 1.f, current ? 4.f : 3.f));
        std::snprintf(tb, sizeof(tb), PZCfg::kStrDailyDay, i);
        auto* d = PZUI::heading(tb, 8.f, current ? PZCfg::kColAccent : PZCfg::kColDim);
        d->setPosition(Vec2(0, 42));
        tile->addChild(d);
        auto* icon = PZUI::coinIcon(30.f);
        icon->setPosition(Vec2(0, 10));
        tile->addChild(icon);
        auto* amt = PZUI::heading(std::to_string(PZCfg::kDailyRewards[i - 1]), 12.f, claimed ? PZCfg::kColDim : PZCfg::kColText);
        amt->setPosition(Vec2(0, -24));
        tile->addChild(amt);
        if (claimed) {
            auto* ok = PZUI::heading("OK", 9.f, PZCfg::kColGood);
            ok->setPosition(Vec2(0, -44));
            tile->addChild(ok);
        }
        if (current) {
            tile->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.07f), ScaleTo::create(0.5f, 1.f), nullptr)));
        }
    };
    std::vector<Node*> tiles;
    for (int i = 1; i <= rewardCount; ++i) {
        auto* tile = Node::create();
        const float x = mid.x - total * 0.5f + tileW * 0.5f + (i - 1) * (tileW + gap);
        tile->setPosition(Vec2(x, mid.y + 26.f));
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
            PZHaptics::play(PZHaptics::Kind::Success);
            if (got > 0) {
                PZUI::burst(basePtr, mid + Vec2(0, 26), PZCfg::kColAccent, 22, 160.f, 0.8f, 10);
                char t[32];
                std::snprintf(t, sizeof(t), "+%d", got);
                PZUI::floatText(basePtr, t, mid + Vec2(0, 90), PZCfg::kColAccent, 26.f, 11);
            }
            if (*btnHolder) (*btnHolder)->setText(PZCfg::kStrClose);
            buildTile(tiles[cycleDay - 1], cycleDay, true, false);
            tiles[cycleDay - 1]->runAction(Sequence::create(ScaleTo::create(0.1f, 1.18f), EaseBackOut::create(ScaleTo::create(0.25f, 1.f)), nullptr));
            char sb[48];
            std::snprintf(sb, sizeof(sb), PZCfg::kStrDailyStreak, PZProgress::shared()->streak());
            streak->setString(sb);
            return;
        }
        dismiss(basePtr, onClosed);
    };
    auto* btn = PZUI::Button::create(available ? PZCfg::kStrDailyClaim : PZCfg::kStrClose, 290.f, 58.f,
                                     available ? PZCfg::kColGood : PZCfg::kColButton, onTap, 17.f);
    *btnHolder = btn;
    btn->setPosition(mid + Vec2(0, -112));
    base->addChild(btn);
    if (available) {
        btn->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.6f, 1.04f), ScaleTo::create(0.6f, 1.f), nullptr)));
    }
    return base;
}

// ================================================================================================
// Missions
// ================================================================================================

namespace {
struct MissionsUI {
    Node* base = nullptr;
    Node* rows = nullptr;
};

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
        row->addChild(PZUI::roundedRect(650.f, 70.f, 12.f, Color3B(30, 34, 52), 0.96f, edge, m.claimed ? 0.5f : 1.f, 3.f));

        auto* text = PZUI::heading(pg->missionText(i), 10.f, m.claimed ? PZCfg::kColDim : PZCfg::kColText);
        text->setAnchorPoint(Vec2(0.f, 0.5f));
        text->setPosition(Vec2(-300.f, 15.f));
        row->addChild(text);

        const int shown = std::min(m.progress, t.target);
        auto* barNode = PZUI::bar(190.f, 11.f, static_cast<float>(shown) / t.target, complete ? PZCfg::kColGood : PZCfg::kColAccent2);
        barNode->setPosition(Vec2(-205.f, -14.f));
        row->addChild(barNode);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d/%d", shown, t.target);
        auto* prog = PZUI::heading(buf, 8.f, PZCfg::kColDim);
        prog->setAnchorPoint(Vec2(0.f, 0.5f));
        prog->setPosition(Vec2(-100.f, -14.f));
        row->addChild(prog);

        auto* icon = PZUI::coinIcon(24.f);
        icon->setPosition(Vec2(105.f, 0.f));
        row->addChild(icon);
        auto* reward = PZUI::heading("+" + std::to_string(t.reward), 12.f, PZCfg::kColAccent);
        reward->setAnchorPoint(Vec2(0.f, 0.5f));
        reward->setPosition(Vec2(124.f, 0.f));
        row->addChild(reward);

        if (m.claimed) {
            auto* d = PZUI::heading(PZCfg::kStrMissionDone, 10.f, PZCfg::kColGood);
            d->setPosition(Vec2(245.f, 0.f));
            row->addChild(d);
        } else {
            auto* b = PZUI::Button::create(PZCfg::kStrMissionClaim, 124.f, 44.f, complete ? PZCfg::kColGood : Color3B(120, 124, 134),
                                           [ui, mid, i]() {
                                               if (PZProgress::shared()->claimMission(i) > 0) {
                                                   playCoin();
                                                   PZHaptics::play(PZHaptics::Kind::Success);
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
    const Vec2 mid = PZLayout::popupCentre();

    auto ui = std::make_shared<MissionsUI>();
    ui->base = openModal(parent, 710.f, 420.f, 320, 0.78f);

    auto* title = PZUI::heading(PZCfg::kStrMissionsTitle, 22.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, 176));
    ui->base->addChild(title);

    buildMissionRows(ui, mid);

    char buf[64];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrMissionNew, pg->timeUntilNewMissions().c_str());
    auto* foot = PZUI::heading(buf, 8.f, PZCfg::kColDim);
    foot->setPosition(mid + Vec2(0, -128.f));
    ui->base->addChild(foot);

    Node* basePtr = ui->base;
    auto* close = PZUI::Button::create(PZCfg::kStrClose, 210.f, 42.f, PZCfg::kColButton,
                                       [basePtr, onClosed]() { dismiss(basePtr, onClosed); }, 13.f);
    close->setPosition(mid + Vec2(0, -168.f));
    ui->base->addChild(close);
    return ui->base;
}

// ================================================================================================
// Game-over results
// ================================================================================================

void attachResults(Node* gameOverLayer, Label* distanceLabel, const PZRunStats& stats, const PZRunResult& r) {
    if (!gameOverLayer) return;
    const Size win = winSize();
    const Rect safe = PZLayout::safeRect();

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

    const float cx = safe.getMidX();
    char buf[64];

    // Layout: headline just under the "GAME OVER" art, the rest in the free space flanking the two round buttons.
    const float headY = win.height * 0.535f;
    const float leftX = win.width * 0.255f, rightX = win.width * 0.745f;
    const float rowY = std::max(win.height * 0.355f, safe.getMinY() + 90.f);

    if (r.newBest) {
        auto* nb = PZUI::heading(PZCfg::kStrNewBest, 22.f, PZCfg::kColAccent);
        nb->setPosition(Vec2(cx, headY));
        nb->setScale(0.1f);
        root->addChild(nb);
        nb->runAction(Sequence::create(
            DelayTime::create(1.0f),
            EaseBackOut::create(ScaleTo::create(0.35f, 1.f)),
            CallFunc::create([root, nb]() {
                PZUI::burst(root, nb->getPosition(), PZCfg::kColAccent, 26, 150.f, 0.9f, 20);
                PZHaptics::play(PZHaptics::Kind::Success);
            }),
            RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.08f), ScaleTo::create(0.5f, 1.f), nullptr)),
            nullptr));
    } else {
        std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsBest, r.best);
        auto* bl = PZUI::heading(buf, 12.f, PZCfg::kColDim);
        bl->setPosition(Vec2(cx, headY));
        root->addChild(bl);
    }

    // Coins earned (left of the buttons).
    std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsCoins, stats.coins);
    auto* coins = PZUI::heading(buf, 12.f, PZCfg::kColAccent);
    coins->setPosition(Vec2(leftX, rowY));
    root->addChild(coins);

    // Level + XP bar (right of the buttons).
    std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsLevel, r.levelBefore);
    auto* lv = PZUI::heading(buf, 11.f, PZCfg::kColAccent2);
    lv->setPosition(Vec2(rightX, rowY + 16.f));
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
            PZUI::floatText(root, b, Vec2(rightX, rowY + 48.f), PZCfg::kColAccent2, 11.f, 30);
            PZUI::burst(root, xpBar->getPosition(), PZCfg::kColAccent2, 22, 140.f, 0.8f, 20);
            PZHaptics::play(PZHaptics::Kind::Success);
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
            PZHaptics::play(PZHaptics::Kind::Success);
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
        PZUI::redrawBar(_xp, 160.f, 10.f, pg->levelFraction(), PZCfg::kColAccent2);
        std::snprintf(b, sizeof(b), PZCfg::kStrBest, pg->bestScore());
        _best->setString(b);
        if (_daily) _daily->setBadge(pg->dailyAvailable() ? 1 : 0);
        if (_missions) _missions->setBadge(pg->unclaimedMissions());
        if (_vibe) {
            std::snprintf(b, sizeof(b), "%s %s", PZCfg::kStrVibration, pg->hapticsEnabled() ? PZCfg::kStrOn : PZCfg::kStrOff);
            _vibe->setText(b);
        }
    }

private:
    void build() {
        auto* pg = PZProgress::shared();
        const Rect safe = PZLayout::safeRect();
        const float colX = safe.getMaxX() - 14.f - 110.f;        // column centre (buttons are 220 wide)
        float y = safe.getMaxY() - 12.f;

        // Player plate (level, XP, best).
        auto* plate = Node::create();
        plate->addChild(PZUI::panel(220.f, 76.f, 14.f));
        plate->setPosition(Vec2(colX, y - 38.f));
        addChild(plate);
        _lv = PZUI::heading("", 12.f, PZCfg::kColAccent);
        _lv->setAnchorPoint(Vec2(0.f, 0.5f));
        _lv->setPosition(Vec2(-94.f, 17.f));
        plate->addChild(_lv);
        _best = PZUI::heading("", 8.f, PZCfg::kColDim);
        _best->setAnchorPoint(Vec2(1.f, 0.5f));
        _best->setPosition(Vec2(94.f, 17.f));
        plate->addChild(_best);
        _xp = PZUI::bar(160.f, 10.f, pg->levelFraction(), PZCfg::kColAccent2);
        _xp->setPosition(Vec2(0.f, -15.f));
        plate->addChild(_xp);
        y -= 76.f + 18.f;

        Node* host = _host;
        if (PZCfg::kEnableDailyReward) {
            _daily = PZUI::Button::create(PZCfg::kStrDailyBtn, 220.f, 50.f, PZCfg::kColButton, [this, host]() {
                PZPopups::showDailyReward(host, [this]() { refresh(); });
            }, 13.f);
            _daily->setPosition(Vec2(colX, y - 25.f));
            addChild(_daily);
            y -= 50.f + 16.f;
        }
        if (PZCfg::kEnableMissions) {
            _missions = PZUI::Button::create(PZCfg::kStrMissionsBtn, 220.f, 50.f, Color3B(70, 150, 240), [this, host]() {
                PZPopups::showMissions(host, [this]() { refresh(); });
            }, 13.f);
            _missions->setPosition(Vec2(colX, y - 25.f));
            addChild(_missions);
            y -= 50.f + 16.f;
        }
        _vibe = PZUI::Button::create("", 220.f, 36.f, Color3B(120, 124, 134), [this]() {
            auto* p = PZProgress::shared();
            p->setHapticsEnabled(!p->hapticsEnabled());
            PZHaptics::play(PZHaptics::Kind::Medium);   // instant preview when turning on
            refresh();
        }, 8.f);
        _vibe->setPosition(Vec2(colX, y - 18.f));
        addChild(_vibe);
        refresh();
    }

    Node* _host = nullptr;
    Label* _lv = nullptr;
    Label* _best = nullptr;
    DrawNode* _xp = nullptr;
    PZUI::Button* _daily = nullptr;
    PZUI::Button* _missions = nullptr;
    PZUI::Button* _vibe = nullptr;
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
