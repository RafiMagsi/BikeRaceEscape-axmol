//
//  PZPopups.cpp
//
#include "PZPopups.h"
#include "PZUI.h"
#include "PZLayout.h"
#include "PZHaptics.h"
#include "PZSettingsController.h"
#include "Ads/AdsController.h"
#include "SharedData/PZGSharedData.h"
#include "SharedData/PZGSoundData.h"
#include "constants.h"

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

Node* showRevive(Node* parent, int cost, int coinsOwned, bool videoSupported, std::function<void()> onCoins,
                 std::function<void()> onVideo, std::function<void()> onDecline) {
    if (!parent) return nullptr;
    const Vec2 mid = PZLayout::popupCentre();
    auto* base = openModal(parent, 520.f, videoSupported ? 430.f : 360.f, 300, 0.78f);

    auto* title = PZUI::heading(PZCfg::kStrReviveTitle, 26.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, videoSupported ? 180.f : 146.f));
    base->addChild(title);

    auto* sub = PZUI::heading(PZCfg::kStrReviveSub, 7.f, PZCfg::kColDim);
    sub->setPosition(mid + Vec2(0, videoSupported ? 152.f : 120.f));
    base->addChild(sub);

    const float ringY = videoSupported ? 92.f : 72.f;
    auto* ring = DrawNode::create();
    ring->setPosition(mid + Vec2(0, ringY));
    base->addChild(ring);
    auto* secs = PZUI::heading("5", 30.f, PZCfg::kColText);
    secs->setPosition(mid + Vec2(0, ringY));
    base->addChild(secs);

    char buf[64];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrReviveCoins, coinsOwned);
    auto* owned = PZUI::heading(buf, 8.f, PZCfg::kColDim);
    owned->setPosition(mid + Vec2(0, ringY - 54.f));
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
    float y = ringY - 108.f;

    std::snprintf(buf, sizeof(buf), canAfford ? PZCfg::kStrReviveBtn : PZCfg::kStrReviveNoCoins, cost);
    auto* coinBtn = PZUI::Button::create(buf, 400.f, 50.f, PZCfg::kColGood, [finish]() { finish(Coins); }, 15.f);
    coinBtn->setPosition(mid + Vec2(0, y));
    coinBtn->setActive(canAfford);
    base->addChild(coinBtn);
    auto* coinIcon = PZUI::coinIcon(26.f);
    coinIcon->setPosition(Vec2(400.f - 36.f, 25.f));
    coinBtn->addChild(coinIcon, 5);
    if (canAfford) {
        coinBtn->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.55f, 1.03f), ScaleTo::create(0.55f, 1.f), nullptr)));
    }
    y -= 60.f;

    PZUI::Button* vidBtn = nullptr;
    auto videoReady = std::make_shared<bool>(false);
    if (videoSupported) {
        const bool ready = PZ::AdsController::shared()->isRewardedAvailable();
        *videoReady = ready;
        const std::string text = ready ? std::string(PZCfg::kStrReviveVideo) + "  " + PZCfg::kStrReviveFree
                                       : std::string(PZCfg::kStrReviveLoading);
        vidBtn = PZUI::Button::create(text, 400.f, 50.f, PZCfg::kColButton,
                                      [finish, videoReady]() { if (*videoReady) finish(Video); }, 13.f);
        vidBtn->setPosition(mid + Vec2(0, y));
        vidBtn->setActive(ready);
        base->addChild(vidBtn);
        auto* tri = DrawNode::create();
        Vec2 pts[3] = {Vec2(-8, -10), Vec2(-8, 10), Vec2(10, 0)};
        tri->drawSolidPoly(pts, 3, PZUI::c4f(Color3B(255, 255, 255)), 2.f, PZUI::c4f(Color3B(90, 30, 0)), true);
        tri->setPosition(Vec2(34.f, 25.f));
        vidBtn->addChild(tri, 5);

        // Poll: reward ads can take a few seconds; keep the button in sync instead of hiding the option.
        base->schedule([done, videoReady, vidBtn, finish] (float) {
            if (*done) return;
            const bool readyNow = PZ::AdsController::shared()->isRewardedAvailable();
            if (readyNow && !*videoReady) {
                *videoReady = true;
                vidBtn->setText(std::string(PZCfg::kStrReviveVideo) + "  " + PZCfg::kStrReviveFree);
                vidBtn->setActive(true);
                vidBtn->setScale(0.9f);
                vidBtn->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.f)));
                PZHaptics::play(PZHaptics::Kind::Light);
            } else if (!readyNow && !*videoReady) {
                PZ::AdsController::shared()->requestRewarded();
            }
            (void)finish;
        }, 0.35f, "pz_revive_adpoll");
        y -= 60.f;
    }

    auto* no = PZUI::Button::create(PZCfg::kStrReviveSkip, 230.f, 36.f, Color3B(120, 124, 134), [finish]() { finish(Decline); }, 10.f);
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

void attachResults(Node* gameOverLayer, Label* distanceLabel, const PZRunStats& stats, const PZRunResult& r,
                   std::function<int()> claimDoubleCoins) {
    if (!gameOverLayer) return;
    const Size win = winSize();
    const Rect safe = PZLayout::safeRect();
    const float cx = safe.getMidX();

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

    // The two round art buttons move down together, above the banner, so the stats panel has room.
    if (auto* menu = dynamic_cast<Menu*>(gameOverLayer->getChildByTag(kBaseMenuItemTag))) {
        const float buttonY = safe.getMinY() + (PZLayout::bannerReserve() > 0.f ? 46.f : 70.f);
        if (auto* back = menu->getChildByTag(1)) back->setPosition(Vec2(cx - 74.f, buttonY));
        if (auto* restart = menu->getChildByTag(2)) restart->setPosition(Vec2(cx + 74.f, buttonY));
    }

    auto* root = Node::create();
    root->setTag(0x5253);
    gameOverLayer->addChild(root, 400);

    char buf[64];
    const float headY = PZLayout::mapY(358.f);

    if (r.newBest) {
        auto* nb = PZUI::heading(PZCfg::kStrNewBest, 20.f, PZCfg::kColAccent);
        nb->setPosition(Vec2(cx, headY));
        nb->setScale(0.1f);
        root->addChild(nb);
        nb->runAction(Sequence::create(
            DelayTime::create(1.0f),
            EaseBackOut::create(ScaleTo::create(0.35f, 1.f)),
            CallFunc::create([root, nb]() {
                PZUI::burst(root, nb->getPosition(), PZCfg::kColAccent, 28, 160.f, 0.9f, 20);
                PZHaptics::play(PZHaptics::Kind::Success);
            }),
            RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.06f), ScaleTo::create(0.5f, 1.f), nullptr)),
            nullptr));
    } else {
        std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsBest, r.best);
        auto* bl = PZUI::heading(buf, 11.f, PZCfg::kColDim);
        bl->setPosition(Vec2(cx, headY));
        root->addChild(bl);
    }

    // Stats panel: three large values on top, level/XP + rewarded double coins on the bottom row.
    const float panelW = std::max(360.f, std::min(560.f, safe.size.width - 460.f));
    const float panelY = PZLayout::mapY(225.f);
    auto* panel = PZUI::panel(panelW, 146.f, 16.f);
    panel->setPosition(Vec2(cx, panelY));
    root->addChild(panel);

    const float colX = panelW * 0.30f;
    auto addStat = [&](float x, const char* label, const std::string& value, const Color3B& color) {
        auto* l = PZUI::heading(label, 7.f, PZCfg::kColDim);
        l->setPosition(Vec2(x, panelY + 52.f));
        root->addChild(l);
        auto* v = PZUI::heading(value, 17.f, color);
        v->setPosition(Vec2(x, panelY + 28.f));
        root->addChild(v);
        return v;
    };
    addStat(cx - colX, PZCfg::kStrStatsDistance, std::to_string(stats.distance), PZCfg::kColText);
    auto* coinsValue = addStat(cx, PZCfg::kStrStatsCoins, "+" + std::to_string(stats.coins), PZCfg::kColAccent);
    addStat(cx + colX, PZCfg::kStrStatsBest, std::to_string(r.best), PZCfg::kColAccent2);

    // Level + XP bar, bottom-left of the panel.
    std::snprintf(buf, sizeof(buf), PZCfg::kStrResultsLevel, r.levelBefore);
    auto* lv = PZUI::heading(buf, 8.f, PZCfg::kColAccent2);
    lv->setAnchorPoint(Vec2(0.f, 0.5f));
    lv->setPosition(Vec2(cx - panelW * 0.5f + 20.f, panelY - 36.f));
    root->addChild(lv);

    const bool hasDouble = PZCfg::kEnableDoubleCoins && claimDoubleCoins && stats.coins > 0;
    const float barW = hasDouble ? std::min(190.f, panelW * 0.30f) : std::min(panelW - 150.f, 360.f);
    const float barX = hasDouble ? (cx - panelW * 0.5f + 70.f + barW * 0.5f) : (cx + 10.f);
    auto* xpBar = PZUI::bar(barW, 10.f, r.fracBefore, PZCfg::kColAccent2);
    xpBar->setPosition(Vec2(barX, panelY - 36.f));
    root->addChild(xpBar);

    const bool leveled = r.levelAfter > r.levelBefore;
    const float endFrac = leveled ? 1.f : r.fracAfter;
    xpBar->runAction(Sequence::create(
        DelayTime::create(0.6f),
        ActionFloat::create(0.8f, r.fracBefore, endFrac,
                            [xpBar, barW](float v) { PZUI::redrawBar(xpBar, barW, 10.f, v, PZCfg::kColAccent2); }),
        CallFunc::create([=]() {
            if (!leveled) return;
            char b[48];
            std::snprintf(b, sizeof(b), PZCfg::kStrResultsLevel, r.levelAfter);
            lv->setString(b);
            std::snprintf(b, sizeof(b), PZCfg::kStrLevelUp, r.levelUpCoins);
            PZUI::floatText(root, b, Vec2(cx, panelY + 82.f), PZCfg::kColAccent2, 10.f, 30);
            PZUI::burst(root, xpBar->getPosition(), PZCfg::kColAccent2, 22, 140.f, 0.8f, 20);
            PZHaptics::play(PZHaptics::Kind::Success);
            PZUI::redrawBar(xpBar, barW, 10.f, 0.f, PZCfg::kColAccent2);
            xpBar->runAction(ActionFloat::create(0.6f, 0.f, r.fracAfter,
                                                 [xpBar, barW](float v) { PZUI::redrawBar(xpBar, barW, 10.f, v, PZCfg::kColAccent2); }));
        }),
        nullptr));

    // Rewarded double coins, bottom-right inside the panel (keeps clear of the island cutout).
    if (hasDouble) {
        auto claimed = std::make_shared<bool>(false);
        auto btnRef = std::make_shared<PZUI::Button*>(nullptr);
        const float doubleW = std::min(210.f, panelW * 0.38f);
        *btnRef = PZUI::Button::create(PZCfg::kStrDoubleCoins, doubleW, 38.f, PZCfg::kColButton, [=]() {
            auto* btn = *btnRef;
            if (!btn || *claimed) return;
            auto* ads = PZ::AdsController::shared();
            if (!ads->isRewardedAvailable()) {
                ads->requestRewarded();
                PZUI::banner(root, PZCfg::kStrDoubleCoins, PZCfg::kStrReviveLoading, PZCfg::kColAccent2,
                             PZLayout::mapY(388.f), 0.9f, 30);
                return;
            }
            ads->showRewardedWithCallback([=](bool earned) {
                if (!earned) {
                    PZUI::banner(root, PZCfg::kStrDoubleCoins, PZCfg::kStrVideoNoAd, PZCfg::kColBad,
                                 PZLayout::mapY(388.f), 1.0f, 30);
                    return;
                }
                auto* btnNow = *btnRef;
                if (!btnNow) return;
                const int bonus = claimDoubleCoins();
                if (bonus <= 0) return;
                *claimed = true;
                char b[64];
                std::snprintf(b, sizeof(b), PZCfg::kStrDoubleClaimed, bonus);
                btnNow->setText(b);
                btnNow->setActive(false);
                coinsValue->setString("+" + std::to_string(stats.coins + bonus));
                PZUI::burst(root, btnNow->getPosition(), PZCfg::kColAccent, 24, 120.f, 0.8f, 30);
                PZUI::floatText(root, "+" + std::to_string(bonus), btnNow->getPosition() + Vec2(0, 42.f),
                                PZCfg::kColAccent, 14.f, 30);
                PZHaptics::play(PZHaptics::Kind::Success);
                playCoin();
            });
        }, 9.f);
        auto* btn = *btnRef;
        btn->setPosition(Vec2(cx + panelW * 0.5f - 16.f - doubleW * 0.5f, panelY - 36.f));
        root->addChild(btn);
        if (!PZ::AdsController::shared()->isRewardedAvailable()) PZ::AdsController::shared()->requestRewarded();
    }

    // Missions completed during this run.
    auto* pg = PZProgress::shared();
    float delay = 1.3f;
    for (int slot : r.completedMissions) {
        const std::string text = pg->missionText(slot);
        root->runAction(Sequence::create(DelayTime::create(delay), CallFunc::create([=]() {
            PZUI::banner(root, PZCfg::kStrMissionToast, text, PZCfg::kColGood, PZLayout::mapY(392.f), 1.4f, 30);
            playCoin();
            PZHaptics::play(PZHaptics::Kind::Success);
        }), nullptr));
        delay += 1.9f;
    }
}

// ================================================================================================
// Gadget loadout
// ================================================================================================

namespace {

class GadgetCard : public Node {
public:
    static GadgetCard* make(const PZCfg::GadgetTemplate& def, bool unlocked, bool selected, float w, float h,
                            std::function<void()> onTap) {
        auto* c = new GadgetCard();
        if (!c->init()) { delete c; return nullptr; }
        c->autorelease();
        c->_w = w; c->_h = h; c->_def = def; c->_unlocked = unlocked; c->_selected = selected;
        c->_onTap = std::move(onTap);
        c->setContentSize(Size(w, h));
        c->setAnchorPoint(Vec2(0.5f, 0.5f));
        c->rebuild();

        auto* l = EventListenerTouchOneByOne::create();
        l->setSwallowTouches(true);
        l->onTouchBegan = [c](Touch* t, Event*) {
            const Vec2 p = c->convertToNodeSpace(t->getLocation());
            if (p.x < 0 || p.y < 0 || p.x > c->_w || p.y > c->_h) return false;
            c->setScale(0.97f);
            return true;
        };
        l->onTouchEnded = [c](Touch* t, Event*) {
            c->setScale(1.f);
            const Vec2 p = c->convertToNodeSpace(t->getLocation());
            if (p.x < 0 || p.y < 0 || p.x > c->_w || p.y > c->_h) return;
            if (c->_onTap) c->_onTap();
        };
        l->onTouchCancelled = [c](Touch*, Event*) { c->setScale(1.f); };
        c->getEventDispatcher()->addEventListenerWithSceneGraphPriority(l, c);
        return c;
    }

    void setState(bool unlocked, bool selected) {
        _unlocked = unlocked; _selected = selected;
        rebuild();
    }

private:
    void rebuild() {
        removeAllChildren();
        const Color3B edge = _selected ? PZCfg::kColAccent : (_unlocked ? PZCfg::kColPanelEdge : Color3B(80, 84, 96));
        auto* bg = PZUI::roundedRect(_w, _h, 14.f, PZCfg::kColPanel, 0.96f, edge, 1.f, _selected ? 3.f : 2.f);
        addChild(bg);
        if (_selected) {
            auto* glow = PZUI::roundedRect(_w - 8.f, _h - 8.f, 11.f, PZCfg::kColAccent, 0.08f, PZCfg::kColAccent, 0.35f, 1.f);
            addChild(glow);
        }

        auto* disc = DrawNode::create();
        disc->drawSolidCircle(Vec2::ZERO, 24.f, 0.f, 24, PZUI::c4f(PZCfg::kColPanelEdge, _unlocked ? 0.22f : 0.10f));
        disc->drawCircle(Vec2::ZERO, 24.f, 0.f, 24, false, PZUI::c4f(edge, 0.95f), 2.f);
        disc->setPosition(Vec2(-_w * 0.5f + 44.f, 8.f));
        addChild(disc);
        auto* glyph = PZUI::heading(_def.icon, 18.f, _unlocked ? PZCfg::kColAccent : PZCfg::kColDim);
        glyph->setPosition(Vec2(-_w * 0.5f + 44.f, 8.f));
        addChild(glyph);

        auto* name = PZUI::heading(_def.name, 11.f, _unlocked ? PZCfg::kColText : PZCfg::kColDim);
        name->setAnchorPoint(Vec2(0.f, 0.5f));
        name->setPosition(Vec2(-_w * 0.5f + 78.f, 18.f));
        const float maxW = _w - 92.f;
        if (name->getContentSize().width > maxW) name->setScale(maxW / name->getContentSize().width);
        addChild(name);

        auto* desc = PZUI::heading(_def.desc, 6.f, PZCfg::kColDim);
        desc->setAnchorPoint(Vec2(0.f, 0.5f));
        desc->setPosition(Vec2(-_w * 0.5f + 78.f, -4.f));
        if (desc->getContentSize().width > maxW) desc->setScale(maxW / desc->getContentSize().width);
        addChild(desc);

        char buf[48];
        if (!_unlocked) {
            std::snprintf(buf, sizeof(buf), PZCfg::kStrGadgetLocked, _def.unlockLevel);
            auto* lock = PZUI::heading(buf, 7.f, PZCfg::kColBad);
            lock->setAnchorPoint(Vec2(0.f, 0.5f));
            lock->setPosition(Vec2(-_w * 0.5f + 78.f, -24.f));
            addChild(lock);
        } else {
            auto* tag = PZUI::heading(_selected ? PZCfg::kStrGadgetEquipped : "", 7.f,
                                      _selected ? PZCfg::kColAccent : PZCfg::kColDim);
            tag->setAnchorPoint(Vec2(0.f, 0.5f));
            tag->setPosition(Vec2(-_w * 0.5f + 78.f, -24.f));
            addChild(tag);
            if (_selected) {
                auto* dot = DrawNode::create();
                dot->drawSolidCircle(Vec2::ZERO, 5.f, 0.f, 14, PZUI::c4f(PZCfg::kColAccent));
                dot->setPosition(Vec2(_w * 0.5f - 18.f, _h * 0.5f - 16.f));
                addChild(dot);
            }
        }
    }

    float _w = 0, _h = 0;
    PZCfg::GadgetTemplate _def;
    bool _unlocked = false;
    bool _selected = false;
    std::function<void()> _onTap;
};

} // namespace

Node* showGadgets(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    const Vec2 mid = PZLayout::popupCentre();
    const float panelW = std::min(720.f, PZLayout::safeRect().size.width - 60.f);
    const float panelH = std::min(430.f, PZLayout::popupMaxHeight());
    auto* base = openModal(parent, panelW, panelH, 330, 0.80f);

    auto* title = PZUI::heading(PZCfg::kStrGadgetsTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, panelH * 0.5f - 44.f));
    base->addChild(title);

    char buf[64];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrGadgetsSub, PZCfg::kGadgetSlots);
    auto* sub = PZUI::heading(buf, 8.f, PZCfg::kColDim);
    sub->setPosition(mid + Vec2(0, panelH * 0.5f - 72.f));
    base->addChild(sub);

    const int count = pg->gadgetPoolCount();
    const int cols = 2;
    const int rows = std::max(1, (count + cols - 1) / cols);
    const float cardW = std::min(300.f, (panelW - 90.f) * 0.5f);
    const float cardH = 92.f;
    const float gapX = 18.f;
    const float gapY = 12.f;
    const float totalW = cols * cardW + (cols - 1) * gapX;
    const float totalH = rows * cardH + (rows - 1) * gapY;
    const float left = mid.x - totalW * 0.5f + cardW * 0.5f;
    const float top = mid.y + 48.f;

    auto refreshCards = std::make_shared<std::function<void()>>();
    std::vector<GadgetCard*> cards;
    for (int i = 0; i < count; ++i) {
        const auto& def = PZCfg::kGadgetPool[i];
        const int col = i % cols;
        const int row = i / cols;
        const Vec2 pos(left + col * (cardW + gapX), top - row * (cardH + gapY));
        auto* card = GadgetCard::make(def, pg->gadgetUnlocked(i), pg->gadgetSelected(def.id), cardW, cardH,
            [=]() {
                char notice[64];
                if (!pg->gadgetUnlocked(i)) {
                    std::snprintf(notice, sizeof(notice), PZCfg::kStrGadgetLocked, def.unlockLevel);
                    PZHaptics::play(PZHaptics::Kind::Warning);
                    PZUI::banner(base, PZCfg::kStrGadgetsTitle, notice, PZCfg::kColBad,
                                 mid.y - panelH * 0.5f + 70.f, 0.8f, 40);
                    return;
                }
                const int before = pg->selectedGadgetCount();
                pg->toggleGadget(def.id);
                if (before == pg->selectedGadgetCount() && !pg->gadgetSelected(def.id) && pg->selectedGadgetCount() >= PZCfg::kGadgetSlots) {
                    std::snprintf(notice, sizeof(notice), PZCfg::kStrGadgetsSub, PZCfg::kGadgetSlots);
                    PZUI::banner(base, PZCfg::kStrGadgetsTitle, notice, PZCfg::kColAccent2,
                                 mid.y - panelH * 0.5f + 70.f, 0.8f, 40);
                    PZHaptics::play(PZHaptics::Kind::Warning);
                    return;
                }
                PZHaptics::play(PZHaptics::Kind::Selection);
                if (*refreshCards) (*refreshCards)();
            });
        if (!card) continue;
        card->setPosition(pos);
        cards.push_back(card);
        base->addChild(card);
    }
    *refreshCards = [pg, cards]() {
        for (size_t i = 0; i < cards.size(); ++i) {
            const auto& def = PZCfg::kGadgetPool[i];
            cards[i]->setState(pg->gadgetUnlocked(static_cast<int>(i)), pg->gadgetSelected(def.id));
        }
    };

    Node* basePtr = base;
    float closeY = mid.y - panelH * 0.5f + 34.f;
    auto* close = PZUI::Button::create(PZCfg::kStrClose, 210.f, 40.f, PZCfg::kColButton,
                                       [basePtr, onClosed]() { dismiss(basePtr, onClosed); }, 12.f);
    close->setPosition(Vec2(mid.x, closeY));
    base->addChild(close);

    return base;
}

// ================================================================================================
// Main-menu widgets// ================================================================================================
// Main-menu widgets
// ================================================================================================

namespace {

std::string clockText(int secs) {
    char b[24];
    std::snprintf(b, sizeof(b), "%d:%02d", secs / 60, secs % 60);
    return b;
}

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
        if (_gadgets) _gadgets->setBadge(pg->selectedGadgetCount());
        if (_vibe) {
            std::snprintf(b, sizeof(b), "%s %s", PZCfg::kStrVibration, pg->hapticsEnabled() ? PZCfg::kStrOn : PZCfg::kStrOff);
            _vibe->setText(b);
        }
        updateFreeLabel();
    }

private:
    void build() {
        auto* pg = PZProgress::shared();
        const Size win = winSize();
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
        y -= 76.f + 14.f;

        Node* host = _host;
        if (PZCfg::kEnableDailyReward) {
            _daily = PZUI::Button::create(PZCfg::kStrDailyBtn, 220.f, 42.f, PZCfg::kColButton, [this, host]() {
                PZPopups::showDailyReward(host, [this]() { refresh(); });
            }, 12.f);
            _daily->setPosition(Vec2(colX, y - 21.f));
            addChild(_daily);
            y -= 42.f + 10.f;
        }
        if (PZCfg::kEnableMissions) {
            _missions = PZUI::Button::create(PZCfg::kStrMissionsBtn, 220.f, 42.f, Color3B(70, 150, 240), [this, host]() {
                PZPopups::showMissions(host, [this]() { refresh(); });
            }, 12.f);
            _missions->setPosition(Vec2(colX, y - 21.f));
            addChild(_missions);
            y -= 42.f + 10.f;
        }

        // Secondary 2-up row: compact, and nudged inwards on island phones so nothing hides under the cutout.
        const bool islandPhone = win.width / std::max(1.f, win.height) > 2.0f;
        float rowRight = safe.getMaxX() - 14.f - (islandPhone ? 90.f : 0.f);
        const float secW = 104.f, secH = 36.f, secGap = 10.f;
        const float rowY = y - 8.f - secH * 0.5f;
        if (PZCfg::kEnableFreeCoins) {
            rowRight -= secW;
            _free = PZUI::Button::create(PZCfg::kStrFreeCoinsBtn, secW, secH, PZCfg::kColButton, [this]() {
                onFreeCoins();
            }, 7.f);
            _free->setPosition(Vec2(rowRight + secW * 0.5f, rowY));
            addChild(_free);
            rowRight -= secGap;
        }
        if (PZCfg::kEnableGadgets) {
            rowRight -= secW;
            _gadgets = PZUI::Button::create(PZCfg::kStrGadgetsBtn, secW, secH, Color3B(150, 105, 220), [this, host]() {
                PZPopups::showGadgets(host, [this]() { refresh(); });
            }, 7.f);
            _gadgets->setPosition(Vec2(rowRight + secW * 0.5f, rowY));
            addChild(_gadgets);
        }

        // Vibration lives in the bottom-right corner: out of the meta cluster, above the banner, below the cutout.
        _vibe = PZUI::Button::create("", 126.f, 32.f, Color3B(120, 124, 134), [this]() {
            auto* p = PZProgress::shared();
            p->setHapticsEnabled(!p->hapticsEnabled());
            PZHaptics::play(PZHaptics::Kind::Medium);   // instant preview when turning on
            refresh();
        }, 8.f);
        _vibe->setPosition(Vec2(safe.getMaxX() - 16.f - 63.f, safe.getMinY() + 34.f));
        addChild(_vibe);

        refresh();
        schedule([this, win](float dt) {
            updateFreeLabel();
            if (!_freePending) return;
            _freeWait += dt;
            if (PZ::AdsController::shared()->isRewardedAvailable()) {
                _freePending = false;
                showFreeCoinsAd();
            } else if (_freeWait > 8.f) {
                _freePending = false;
                PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, PZCfg::kStrVideoNoAd, PZCfg::kColBad,
                             win.height * 0.55f, 1.0f, 30);
            } else {
                PZ::AdsController::shared()->requestRewarded();
            }
        }, 0.35f, "pz_menu_free_poll");
    }

    void updateFreeLabel() {
        if (!_free) return;
        auto* pg = PZProgress::shared();
        std::string text;
        if (pg->freeCoinsAvailable()) {
            text = PZCfg::kStrFreeCoinsBtn;
            _free->setBadge(1);
        } else {
            char b[40];
            std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsReady, clockText(pg->freeCoinsSecondsLeft()).c_str());
            text = b;
            _free->setBadge(0);
        }
        if (text != _freeText) {
            _freeText = text;
            _free->setText(text);
        }
    }

    void onFreeCoins() {
        auto* pg = PZProgress::shared();
        if (!pg->freeCoinsAvailable()) {
            char b[48];
            std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsWait, clockText(pg->freeCoinsSecondsLeft()).c_str());
            PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, b, PZCfg::kColAccent2, winSize().height * 0.55f, 1.2f, 30);
            return;
        }
        showFreeCoinsAd();
    }

    void showFreeCoinsAd() {
        auto* ads = PZ::AdsController::shared();
        if (!ads->isRewardedAvailable()) {
            ads->requestRewarded();
            _freePending = true;
            _freeWait = 0.f;
            PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, PZCfg::kStrReviveLoading, PZCfg::kColAccent2,
                         winSize().height * 0.55f, 1.0f, 30);
            return;
        }
        ads->showRewardedWithCallback([this](bool earned) {
            _freePending = false;
            if (!earned) {
                PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, PZCfg::kStrVideoNoAd, PZCfg::kColBad,
                             winSize().height * 0.55f, 1.0f, 30);
                return;
            }
            const int got = PZProgress::shared()->claimFreeCoins(PZCfg::kFreeCoinsAmount);
            if (got <= 0) return;
            char b[64];
            std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsSub, got);
            PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, b, PZCfg::kColGood, winSize().height * 0.55f, 1.3f, 30);
            if (_free) {
                const Vec2 world = _free->convertToWorldSpace(Vec2::ZERO);
                PZUI::burst(_host, _host->convertToNodeSpace(world), PZCfg::kColAccent, 20, 100.f, 0.7f, 30);
            }
            playCoin();
            PZHaptics::play(PZHaptics::Kind::Success);
            refresh();
        });
    }

    Node* _host = nullptr;
    Label* _lv = nullptr;
    Label* _best = nullptr;
    DrawNode* _xp = nullptr;
    PZUI::Button* _daily = nullptr;
    PZUI::Button* _missions = nullptr;
    PZUI::Button* _gadgets = nullptr;
    PZUI::Button* _free = nullptr;
    PZUI::Button* _vibe = nullptr;
    std::string _freeText;
    bool _freePending = false;
    float _freeWait = 0.f;
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
