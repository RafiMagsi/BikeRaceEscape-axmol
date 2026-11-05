//
//  PZPopups.cpp
//
#include "PZPopups.h"
#include "PZUI.h"
#include "PZLayout.h"
#include "PZHaptics.h"
#include "PZNotifications.h"
#include "PZSettingsController.h"
#include "Ads/AdsController.h"
#include "SharedData/PZGSharedData.h"
#include "SharedData/PZGSoundData.h"
#include "constants.h"
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "cocos2dx_StoreController.h"
#endif
#include "SharedData/PZGGameInfoIAP.h"

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

// Every popup must sit above the in-run HUD (power-up chips, mission tracker, labels are at z ~1000) and above
// the main-menu meta widgets (z 450), otherwise its panel is drawn underneath live read-outs.
constexpr int kZModal = 1500;

// Modal scaffold: dim base + original-style panel, popping in.
LayerColor* openModal(Node* parent, float w, float h, int z, float dim) {
    // `z` is kept for readability at the call sites; popups always take the reserved top layer.
    (void)z;
    auto* base = PZUI::modalBase(dim);
    parent->addChild(base, kZModal);
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
                 std::function<void()> onVideo, std::function<void()> onDecline, float countdown) {
    if (!parent) return nullptr;
    if (countdown < 0.f) countdown = PZCfg::kReviveCountdown;
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

    auto left = std::make_shared<float>(countdown);
    auto draw = [ring, secs, left, countdown]() {
        const float frac = std::max(0.f, *left / std::max(0.01f, countdown));
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
        dismiss(basePtr, [parent, onClosed, claimedNow]() {
            if (onClosed) onClosed();
            if (*claimedNow) PZNotif::maybeOffer(parent);       // a claimed reward is the best moment to ask
        });
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
                   std::function<int()> claimDoubleCoins, std::function<void()> onReviveCoins,
                   std::function<void()> onReviveVideo) {
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

    // Bottom row: BACK and RETRY (standard, smaller round art buttons) with the two revive options between them.
    // The revive options are always both offered while this run has not been revived: spend coins, or watch an ad.
    if (auto* menu = dynamic_cast<Menu*>(gameOverLayer->getChildByTag(kBaseMenuItemTag))) {
        const float buttonY = safe.getMinY() + (PZLayout::bannerReserve() > 0.f ? 50.f : 66.f);
        const bool revive = onReviveCoins && onReviveVideo;
        const float pillW = 236.f, pillH = 46.f, gap = 12.f;
        const float roundX = revive ? (pillW + gap * 0.5f + 44.f + gap) : 74.f;
        if (auto* back = menu->getChildByTag(1)) { back->setScale(0.72f); back->setPosition(Vec2(cx - roundX, buttonY)); }
        if (auto* restart = menu->getChildByTag(2)) { restart->setScale(0.72f); restart->setPosition(Vec2(cx + roundX, buttonY)); }
        if (revive) {
            auto* sc = PZSettingsController::shared();
            char b[48];
            // Coins option.
            std::snprintf(b, sizeof(b), PZCfg::kStrReviveBtn, PZCfg::kReviveCost);
            auto coinBtn = std::make_shared<PZUI::Button*>(nullptr);
            *coinBtn = PZUI::Button::create(b, pillW, pillH, PZCfg::kColGood, [onReviveCoins]() { onReviveCoins(); }, 14.f);
            (*coinBtn)->setPosition(Vec2(cx - (pillW + gap) * 0.5f, buttonY));
            gameOverLayer->addChild(*coinBtn, 450);
            auto* coinIcon = PZUI::coinIcon(24.f);
            coinIcon->setPosition(Vec2(pillW - 30.f, pillH * 0.5f));
            (*coinBtn)->addChild(coinIcon, 5);
            // Ad option.
            auto adBtn = std::make_shared<PZUI::Button*>(nullptr);
            auto adReady = std::make_shared<bool>(PZ::AdsController::shared()->isRewardedAvailable());
            *adBtn = PZUI::Button::create(*adReady ? PZCfg::kStrReviveAdButton : PZCfg::kStrReviveLoading, pillW, pillH,
                                          PZCfg::kColButton, [onReviveVideo, adReady]() { if (*adReady) onReviveVideo(); }, 11.f);
            (*adBtn)->setPosition(Vec2(cx + (pillW + gap) * 0.5f, buttonY));
            (*adBtn)->setActive(*adReady);
            gameOverLayer->addChild(*adBtn, 450);
            auto* tri = DrawNode::create();
            Vec2 pts[3] = {Vec2(-7, -9), Vec2(-7, 9), Vec2(9, 0)};
            tri->drawSolidPoly(pts, 3, PZUI::c4f(Color3B(255, 255, 255)), 2.f, PZUI::c4f(Color3B(90, 30, 0)), true);
            tri->setScale(0.8f);
            tri->setPosition(Vec2(18.f, pillH * 0.5f));
            (*adBtn)->addChild(tri, 5);
            if (!*adReady) PZ::AdsController::shared()->requestRewarded();

            // Keep both in sync: coins can change (double coins), and the ad may finish loading any time.
            gameOverLayer->schedule([coinBtn, adBtn, adReady, sc](float) {
                const bool afford = sc->coins >= PZCfg::kReviveCost;
                char t[48];
                std::snprintf(t, sizeof(t), afford ? PZCfg::kStrReviveBtn : PZCfg::kStrReviveNoCoins, PZCfg::kReviveCost);
                (*coinBtn)->setText(t);
                (*coinBtn)->setActive(afford);
                const bool ready = PZ::AdsController::shared()->isRewardedAvailable();
                if (ready != *adReady) {
                    *adReady = ready;
                    (*adBtn)->setText(ready ? PZCfg::kStrReviveAdButton : PZCfg::kStrReviveLoading);
                    (*adBtn)->setActive(ready);
                    if (!ready) PZ::AdsController::shared()->requestRewarded();
                    else PZHaptics::play(PZHaptics::Kind::Light);
                }
            }, 0.4f, "pz_go_revive_sync");
            if (sc->coins >= PZCfg::kReviveCost) {
                (*coinBtn)->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.6f, 1.04f), ScaleTo::create(0.6f, 1.f), nullptr)));
            }
        }
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
    const float doubleW = std::min(210.f, panelW * 0.38f);
    // XP bar fills the row from the LEVEL label up to the DOUBLE COINS button (or the panel edge).
    const float barLeft = cx - panelW * 0.5f + 88.f;
    const float barRight = hasDouble ? (cx + panelW * 0.5f - 22.f - doubleW - 16.f) : (cx + panelW * 0.5f - 24.f);
    const float barW = std::max(80.f, std::min(420.f, barRight - barLeft));
    const float barX = barLeft + barW * 0.5f;
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
        btn->setPosition(Vec2(cx + panelW * 0.5f - 22.f - doubleW * 0.5f, panelY - 36.f));
        root->addChild(btn);
        if (!PZ::AdsController::shared()->isRewardedAvailable()) PZ::AdsController::shared()->requestRewarded();
    }

    // Reminder offer: only at a good moment (new best, level-up, mission done, or a decent haul), after the animations.
    {
        const bool happy = r.newBest || r.levelAfter > r.levelBefore || !r.completedMissions.empty() || stats.coins >= 25;
        Node* host = gameOverLayer->getParent() ? gameOverLayer->getParent() : gameOverLayer;
        if (happy && PZNotif::shouldOffer()) {
            root->runAction(Sequence::create(DelayTime::create(3.2f), CallFunc::create([host]() { PZNotif::maybeOffer(host); }), nullptr));
        }
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

// Vertical shop-style card: drawn icon, name, description, status line (owned / equipped / locked) and a BUY pill.
// Tapping the BUY pill calls onBuy; tapping anywhere else calls onTap (equip / unequip) when set.
struct GadgetView {
    bool unlocked = true;
    bool selected = false;
    int  stock = 0;
    bool canAfford = true;
    bool showEquip = true;     // loadout popup: card tap equips; shop: only the BUY pill is live
};

class GadgetCard : public Node {
public:
    static GadgetCard* make(const PZCfg::GadgetTemplate& def, const GadgetView& v, float w, float h,
                            std::function<void()> onTap, std::function<void()> onBuy) {
        auto* c = new GadgetCard();
        if (!c->init()) { delete c; return nullptr; }
        c->autorelease();
        c->_w = w; c->_h = h; c->_def = def; c->_v = v;
        c->_onTap = std::move(onTap);
        c->_onBuy = std::move(onBuy);
        // Content is drawn centred on the node's local origin, so the origin must stay at the node's position
        // (anchor 0,0). A (0.5,0.5) anchor shifted the whole card by half its size and offset the touch test.
        c->setContentSize(Size(w, h));
        c->setAnchorPoint(Vec2::ZERO);
        c->rebuild();

        auto* l = EventListenerTouchOneByOne::create();
        l->setSwallowTouches(true);
        l->onTouchBegan = [c](Touch* t, Event*) {
            const Vec2 p = c->convertToNodeSpace(t->getLocation());
            if (std::abs(p.x) > c->_w * 0.5f || std::abs(p.y) > c->_h * 0.5f) return false;
            c->setScale(0.97f);
            return true;
        };
        l->onTouchEnded = [c](Touch* t, Event*) {
            c->setScale(1.f);
            const Vec2 p = c->convertToNodeSpace(t->getLocation());
            if (std::abs(p.x) > c->_w * 0.5f || std::abs(p.y) > c->_h * 0.5f) return;
            const bool inBuyZone = p.y < -c->_h * 0.5f + 40.f;
            if (inBuyZone) {
                if (c->_onBuy) c->_onBuy();
            } else if (c->_v.showEquip && c->_onTap) {
                c->_onTap();
            }
        };
        l->onTouchCancelled = [c](Touch*, Event*) { c->setScale(1.f); };
        c->getEventDispatcher()->addEventListenerWithSceneGraphPriority(l, c);
        return c;
    }

    void setState(const GadgetView& v) {
        _v = v;
        rebuild();
    }

    // Quick "got it" feedback on the card after a purchase.
    void pulse() {
        stopAllActions();
        setScale(1.f);
        runAction(Sequence::create(ScaleTo::create(0.08f, 1.06f), ScaleTo::create(0.12f, 1.f), nullptr));
    }

private:
    void rebuild() {
        removeAllChildren();
        const bool on = _v.unlocked;
        const bool sel = _v.selected && _v.showEquip;
        const Color3B edge = sel ? PZCfg::kColAccent : (on ? PZCfg::kColPanelEdge : Color3B(80, 84, 96));
        addChild(PZUI::roundedRect(_w, _h, 14.f, PZCfg::kColPanel, 0.96f, edge, 1.f, sel ? 3.f : 2.f));
        if (sel) {
            addChild(PZUI::roundedRect(_w - 8.f, _h - 8.f, 11.f, PZCfg::kColAccent, 0.08f, PZCfg::kColAccent, 0.35f, 1.f));
        }

        const float top = _h * 0.5f;
        auto* disc = DrawNode::create();
        disc->drawSolidCircle(Vec2::ZERO, 30.f, 0.f, 28, PZUI::c4f(PZCfg::kColPanelEdge, on ? 0.22f : 0.10f));
        disc->drawCircle(Vec2::ZERO, 30.f, 0.f, 28, false, PZUI::c4f(edge, 0.95f), 2.f);
        disc->setPosition(Vec2(0.f, top - 38.f));
        addChild(disc);
        auto* icon = PZUI::gadgetIcon(_def.id, 38.f, _def.icon, !on);
        icon->setPosition(Vec2(0.f, top - 38.f));
        addChild(icon);

        const float maxW = _w - 18.f;
        auto fit = [maxW](Label* l) { if (l->getContentSize().width > maxW) l->setScale(maxW / l->getContentSize().width); };
        auto* name = PZUI::heading(_def.name, 10.f, on ? PZCfg::kColText : PZCfg::kColDim);
        name->setPosition(Vec2(0.f, top - 80.f));
        fit(name);
        addChild(name);
        auto* desc = PZUI::heading(_def.desc, 6.5f, PZCfg::kColDim);
        desc->setPosition(Vec2(0.f, top - 96.f));
        fit(desc);
        addChild(desc);

        // Status line.
        char buf[48];
        if (!on) {
            std::snprintf(buf, sizeof(buf), PZCfg::kStrGadgetLocked, _def.unlockLevel);
        } else if (_v.selected && _v.showEquip) {
            std::snprintf(buf, sizeof(buf), "%s  x%d", PZCfg::kStrGadgetEquipped, _v.stock);
        } else {
            std::snprintf(buf, sizeof(buf), PZCfg::kStrGadgetOwned, _v.stock);
        }
        auto* status = PZUI::heading(buf, 7.f, !on ? PZCfg::kColBad : (_v.stock > 0 ? PZCfg::kColAccent : PZCfg::kColDim));
        status->setPosition(Vec2(0.f, -top + 52.f));
        fit(status);
        addChild(status);

        // BUY pill.
        const bool can = on && _v.canAfford;
        const Color3B pill = can ? PZCfg::kColGood : Color3B(96, 100, 112);
        addChild([&]() {
            auto* b = PZUI::roundedRect(_w - 28.f, 28.f, 14.f, pill, 1.f, Color3B(255, 255, 255), 0.9f, 1.5f);
            b->setPosition(Vec2(0.f, -top + 22.f));
            return b;
        }());
        std::snprintf(buf, sizeof(buf), "%d", _def.price);
        auto* price = PZUI::heading(buf, 11.f, on ? Color3B(255, 255, 255) : Color3B(170, 172, 182));
        price->setAnchorPoint(Vec2(1.f, 0.5f));
        price->setPosition(Vec2(8.f, -top + 22.f));
        addChild(price);
        auto* coin = PZUI::coinIcon(18.f);
        coin->setPosition(Vec2(20.f, -top + 22.f));
        if (!on) coin->setOpacity(120);
        addChild(coin);
    }

    float _w = 0, _h = 0;
    PZCfg::GadgetTemplate _def;
    GadgetView _v;
    std::function<void()> _onTap;
    std::function<void()> _onBuy;
};

GadgetView viewFor(int poolIndex, bool showEquip) {
    auto* pg = PZProgress::shared();
    const auto& def = PZCfg::kGadgetPool[poolIndex];
    GadgetView v;
    v.unlocked = pg->gadgetUnlocked(poolIndex);
    v.selected = pg->gadgetSelected(def.id);
    v.stock = pg->gadgetStock(def.id);
    v.canAfford = PZSettingsController::shared()->coins >= def.price;
    v.showEquip = showEquip;
    return v;
}

// Lays the gadget cards out in a grid inside `host`, centred in the rect (cx, top..bottom). Returns the cards.
// `onChanged` runs after a purchase or equip change so the owner can refresh its own readouts.
std::vector<GadgetCard*> buildGadgetGrid(Node* host, Node* noticeHost, const Vec2& centre, float width, float top, float bottom,
                                         bool showEquip, std::function<void()> onChanged) {
    auto* pg = PZProgress::shared();
    const int count = pg->gadgetPoolCount();
    const int cols = std::max(1, std::min(4, static_cast<int>((width + 14.f) / 190.f)));
    const int rows = std::max(1, (count + cols - 1) / cols);
    const float gapX = 14.f, gapY = 12.f;
    const float cardW = (width - (cols - 1) * gapX) / cols;
    const float cardH = std::max(128.f, std::min(158.f, ((top - bottom) - (rows - 1) * gapY) / rows));
    const float totalH = rows * cardH + (rows - 1) * gapY;
    const float left = centre.x - width * 0.5f + cardW * 0.5f;
    const float firstY = top - std::max(0.f, ((top - bottom) - totalH) * 0.5f) - cardH * 0.5f;

    auto cards = std::make_shared<std::vector<GadgetCard*>>();
    auto refresh = std::make_shared<std::function<void()>>();
    *refresh = [cards, showEquip]() {
        for (size_t i = 0; i < cards->size(); ++i) (*cards)[i]->setState(viewFor(static_cast<int>(i), showEquip));
    };
    for (int i = 0; i < count; ++i) {
        const auto& def = PZCfg::kGadgetPool[i];
        const int col = i % cols, row = i / cols;
        const Vec2 pos(left + col * (cardW + gapX), firstY - row * (cardH + gapY));
        const float noticeY = bottom + 6.f;
        auto* card = GadgetCard::make(def, viewFor(i, showEquip), cardW, cardH,
            [=]() {                              // equip / unequip
                if (!pg->gadgetUnlocked(i)) {
                    char notice[64];
                    std::snprintf(notice, sizeof(notice), PZCfg::kStrGadgetLocked, def.unlockLevel);
                    PZHaptics::play(PZHaptics::Kind::Warning);
                    PZUI::banner(noticeHost, PZCfg::kStrGadgetsTitle, notice, PZCfg::kColBad, noticeY, 0.8f, 1600);
                    return;
                }
                const int before = pg->selectedGadgetCount();
                pg->toggleGadget(def.id);
                if (before == pg->selectedGadgetCount() && !pg->gadgetSelected(def.id)) {
                    char notice[64];
                    std::snprintf(notice, sizeof(notice), PZCfg::kStrGadgetsSub, PZCfg::kGadgetSlots);
                    PZUI::banner(noticeHost, PZCfg::kStrGadgetsTitle, notice, PZCfg::kColAccent2, noticeY, 0.8f, 1600);
                    PZHaptics::play(PZHaptics::Kind::Warning);
                    return;
                }
                PZHaptics::play(PZHaptics::Kind::Selection);
                (*refresh)();
                if (onChanged) onChanged();
            },
            [=]() {                              // buy one charge
                if (!pg->gadgetUnlocked(i)) {
                    char notice[64];
                    std::snprintf(notice, sizeof(notice), PZCfg::kStrGadgetLocked, def.unlockLevel);
                    PZHaptics::play(PZHaptics::Kind::Warning);
                    PZUI::banner(noticeHost, PZCfg::kStrShopTitle, notice, PZCfg::kColBad, noticeY, 0.8f, 1600);
                    return;
                }
                if (!pg->buyGadget(def.id)) {
                    PZHaptics::play(PZHaptics::Kind::Error);
                    PZUI::banner(noticeHost, PZCfg::kStrShopTitle, PZCfg::kStrReviveNoCoinsShort, PZCfg::kColBad, noticeY, 0.9f, 1600);
                    return;
                }
                // New charges go straight into an empty loadout slot so buying always has a visible effect.
                if (!pg->gadgetSelected(def.id) && pg->selectedGadgetCount() < PZCfg::kGadgetSlots) pg->toggleGadget(def.id);
                playCoin();
                PZHaptics::play(PZHaptics::Kind::Success);
                (*refresh)();
                (*cards)[static_cast<size_t>(i)]->pulse();
                PZUI::burst(host, pos, PZCfg::kColGood, 12, 70.f, 0.5f, 1600);
                if (onChanged) onChanged();
            });
        if (!card) continue;
        card->setPosition(pos);
        cards->push_back(card);
        host->addChild(card);
    }
    return *cards;
}

} // namespace

Node* showGadgets(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    const Vec2 mid = PZLayout::popupCentre();
    const float panelW = std::min(900.f, PZLayout::safeRect().size.width - 40.f);
    const float panelH = std::min(500.f, PZLayout::popupMaxHeight());
    auto* base = openModal(parent, panelW, panelH, 330, 0.80f);

    auto* title = PZUI::heading(PZCfg::kStrGadgetsTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, panelH * 0.5f - 40.f));
    base->addChild(title);

    char buf[64];
    std::snprintf(buf, sizeof(buf), PZCfg::kStrGadgetsSub, PZCfg::kGadgetSlots);
    auto* sub = PZUI::heading(buf, 8.f, PZCfg::kColDim);
    sub->setPosition(mid + Vec2(0, panelH * 0.5f - 66.f));
    base->addChild(sub);

    // Coin balance (top-right inside the panel), kept live while buying.
    auto* coin = PZUI::coinIcon(24.f);
    coin->setPosition(mid + Vec2(panelW * 0.5f - 112.f, panelH * 0.5f - 40.f));
    base->addChild(coin);
    auto* bal = PZUI::heading("0", 13.f, PZCfg::kColAccent);
    bal->setAnchorPoint(Vec2(0.f, 0.5f));
    bal->setPosition(mid + Vec2(panelW * 0.5f - 94.f, panelH * 0.5f - 40.f));
    base->addChild(bal);
    auto updateBal = [bal]() {
        char b[16];
        std::snprintf(b, sizeof(b), "%d", static_cast<int>(PZSettingsController::shared()->coins));
        bal->setString(b);
    };
    updateBal();

    const float closeY = mid.y - panelH * 0.5f + 34.f;
    buildGadgetGrid(base, base, mid, panelW - 56.f, mid.y + panelH * 0.5f - 84.f, closeY + 38.f, true, updateBal);

    Node* basePtr = base;
    auto* close = PZUI::Button::create(PZCfg::kStrClose, 210.f, 40.f, PZCfg::kColButton,
                                       [basePtr, onClosed]() { dismiss(basePtr, onClosed); }, 12.f);
    close->setPosition(Vec2(mid.x, closeY));
    base->addChild(close);
    return base;
}

// ================================================================================================
// Shop: power-up charges for coins, and coins from a rewarded video / store packs
// ================================================================================================

namespace {

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
void buyStorePack(int pack) {
    PZGSharedData* sd = PZGSharedData::sharedInstanse();
    if (!sd || !sd->gameInfoResource) return;
    auto* array = (__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    if (!array || array->count() <= 0) return;
    auto* iap = (PZGGameInfoIAP*)array->objectAtIndex(0);
    if (!iap) return;
    __String* id = pack == 1 ? iap->coinShop1_id : (pack == 2 ? iap->coinShop2_id : iap->coinShop3_id);
    if (id) cocos2dx_StoreController::buyMarketItem(id->getCString());
}
#endif

} // namespace

Node* showShop(Node* parent, std::function<void()> onClosed, bool startOnCoins) {
    if (!parent) return nullptr;
    const Vec2 mid = PZLayout::popupCentre();
    const float panelW = std::min(900.f, PZLayout::safeRect().size.width - 40.f);
    const float panelH = std::min(520.f, PZLayout::popupMaxHeight());
    auto* base = openModal(parent, panelW, panelH, 340, 0.82f);

    auto* title = PZUI::heading(PZCfg::kStrShopTitle, 26.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, panelH * 0.5f - 40.f));
    base->addChild(title);

    auto* coin = PZUI::coinIcon(26.f);
    coin->setPosition(mid + Vec2(panelW * 0.5f - 118.f, panelH * 0.5f - 40.f));
    base->addChild(coin);
    auto* bal = PZUI::heading("0", 14.f, PZCfg::kColAccent);
    bal->setAnchorPoint(Vec2(0.f, 0.5f));
    bal->setPosition(mid + Vec2(panelW * 0.5f - 100.f, panelH * 0.5f - 40.f));
    base->addChild(bal);
    auto updateBal = [bal]() {
        char b[16];
        std::snprintf(b, sizeof(b), "%d", static_cast<int>(PZSettingsController::shared()->coins));
        bal->setString(b);
    };
    updateBal();

    const float closeY = mid.y - panelH * 0.5f + 34.f;
    const float tabY = mid.y + panelH * 0.5f - 84.f;
    auto* content = Node::create();
    base->addChild(content);
    auto* tabMark = DrawNode::create();
    base->addChild(tabMark);

    auto tab = std::make_shared<int>(startOnCoins ? 1 : 0);
    auto showTab = std::make_shared<std::function<void()>>();
    auto rowRefresh = std::make_shared<std::function<void()>>();   // live labels on the coins tab

    PZUI::Button* tabPower = nullptr;
    PZUI::Button* tabCoins = nullptr;
    tabPower = PZUI::Button::create(PZCfg::kStrShopPowerUps, 250.f, 38.f, Color3B(150, 105, 220), [tab, showTab]() { *tab = 0; (*showTab)(); }, 11.f);
    tabPower->setPosition(Vec2(mid.x - 134.f, tabY));
    base->addChild(tabPower);
    tabCoins = PZUI::Button::create(PZCfg::kStrShopCoins, 250.f, 38.f, PZCfg::kColButton, [tab, showTab]() { *tab = 1; (*showTab)(); }, 11.f);
    tabCoins->setPosition(Vec2(mid.x + 134.f, tabY));
    base->addChild(tabCoins);

    Node* basePtr = base;
    *showTab = [=]() {
        content->removeAllChildren();
        tabMark->clear();
        const float mx = mid.x + (*tab == 0 ? -134.f : 134.f);
        tabMark->drawSolidRect(Vec2(mx - 125.f, tabY - 28.f), Vec2(mx + 125.f, tabY - 24.f), PZUI::c4f(PZCfg::kColAccent));
        *rowRefresh = nullptr;
        const float top = tabY - 36.f;
        const float bottom = closeY + 38.f;
        if (*tab == 0) {
            buildGadgetGrid(content, basePtr, mid, panelW - 56.f, top, bottom, false, updateBal);
            return;
        }
        // ---- COINS tab: rewarded video + store packs ----
        auto* pg = PZProgress::shared();
        const float rowW = std::min(620.f, panelW - 80.f);
        auto vidHolder = std::make_shared<PZUI::Button*>(nullptr);
        auto pending = std::make_shared<bool>(false);
        auto vidText = std::make_shared<std::string>();
        auto setVid = [vidHolder, vidText](const std::string& t, bool active) {
            if (t != *vidText) { *vidText = t; (*vidHolder)->setText(t); }
            (*vidHolder)->setActive(active);
        };
        auto clock = [](int secs) {
            char b[24];
            std::snprintf(b, sizeof(b), "%d:%02d", secs / 60, secs % 60);
            return std::string(b);
        };
        auto fmtReady = [pg]() {
            char b[64];
            std::snprintf(b, sizeof(b), PZCfg::kStrShopWatchAd, PZCfg::kFreeCoinsAmount);
            return std::string(b);
        };
        const auto onVideo = [=]() {
            auto* ads = PZ::AdsController::shared();
            if (!pg->freeCoinsAvailable()) return;
            if (!ads->isRewardedAvailable()) {
                ads->requestRewarded();
                PZUI::banner(basePtr, PZCfg::kStrShopTitle, PZCfg::kStrReviveLoading, PZCfg::kColAccent2, closeY + 70.f, 0.9f, 1600);
                return;
            }
            ads->showRewardedWithCallback([=](bool earned) {
                if (!earned) {
                    PZUI::banner(basePtr, PZCfg::kStrShopTitle, PZCfg::kStrVideoNoAd, PZCfg::kColBad, closeY + 70.f, 1.0f, 1600);
                    return;
                }
                const int got = pg->claimFreeCoins(PZCfg::kFreeCoinsAmount);
                if (got <= 0) return;
                char b[48];
                std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsSub, got);
                PZUI::banner(basePtr, PZCfg::kStrShopTitle, b, PZCfg::kColGood, closeY + 70.f, 1.2f, 1600);
                PZUI::burst(basePtr, (*vidHolder)->getPosition(), PZCfg::kColAccent, 24, 120.f, 0.8f, 1600);
                playCoin();
                PZHaptics::play(PZHaptics::Kind::Success);
                updateBal();
            });
        };
        auto* vidReal = PZUI::Button::create("", rowW, 64.f, Color3B(255, 140, 20), onVideo, 15.f);
        vidReal->setPosition(Vec2(mid.x, top - 56.f));
        content->addChild(vidReal);
        *vidHolder = vidReal;
        auto* tri = DrawNode::create();
        Vec2 pts[3] = {Vec2(-9, -12), Vec2(-9, 12), Vec2(12, 0)};
        tri->drawSolidPoly(pts, 3, PZUI::c4f(Color3B(255, 255, 255)), 2.f, PZUI::c4f(Color3B(90, 30, 0)), true);
        tri->setPosition(Vec2(38.f, 32.f));
        vidReal->addChild(tri, 5);
        auto* vcoin = PZUI::coinIcon(30.f);
        vcoin->setPosition(Vec2(rowW - 40.f, 32.f));
        vidReal->addChild(vcoin, 5);
        auto* note = PZUI::heading(PZCfg::kStrShopAdNote, 6.f, PZCfg::kColDim);
        note->setPosition(Vec2(mid.x, top - 102.f));
        content->addChild(note);

        *rowRefresh = [=]() {
            if (pg->freeCoinsAvailable()) {
                const bool ready = PZ::AdsController::shared()->isRewardedAvailable();
                setVid(ready ? fmtReady() : std::string(PZCfg::kStrReviveLoading), ready);
                if (!ready) PZ::AdsController::shared()->requestRewarded();
                vidReal->setBadge(ready ? 1 : 0);
            } else {
                char b[48];
                const int secs = pg->freeCoinsSecondsLeft();
                std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsReady, clock(secs).c_str());
                setVid(b, false);
                vidReal->setBadge(0);
            }
        };
        (*rowRefresh)();

        // Store packs (real money, via the platform billing already wired for the legacy coin store).
        bool packs = false;
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
        if (PZGSharedData* sd = PZGSharedData::sharedInstanse(); sd && sd->gameInfoResource) {
            if (auto* array = (__Array*)sd->gameInfoResource->objectForKey("IAPSettings"); array && array->count() > 0) {
                auto* iap = (PZGGameInfoIAP*)array->objectAtIndex(0);
                packs = iap && iap->coinShop_enabled;
                if (packs) {
                    const int values[3] = {iap->coinShop1_value, iap->coinShop2_value, iap->coinShop3_value};
                    const float cw = (rowW - 2 * 14.f) / 3.f;
                    const float cy = top - 208.f;
                    auto* hd = PZUI::heading(PZCfg::kStrShopPacks, 8.f, PZCfg::kColDim);
                    hd->setPosition(Vec2(mid.x, top - 140.f));
                    content->addChild(hd);
                    for (int i = 0; i < 3; ++i) {
                        const float cx = mid.x - rowW * 0.5f + cw * 0.5f + i * (cw + 14.f);
                        auto* card = PZUI::roundedRect(cw, 104.f, 14.f, PZCfg::kColPanel, 0.96f, PZCfg::kColPanelEdge, 1.f, 2.f);
                        card->setPosition(Vec2(cx, cy));
                        content->addChild(card);
                        auto* ci = PZUI::coinIcon(30.f);
                        ci->setPosition(Vec2(cx - 36.f, cy + 20.f));
                        content->addChild(ci);
                        char b[24];
                        std::snprintf(b, sizeof(b), "%d", values[i]);
                        auto* amt = PZUI::heading(b, 15.f, PZCfg::kColAccent);
                        amt->setAnchorPoint(Vec2(0.f, 0.5f));
                        amt->setPosition(Vec2(cx - 14.f, cy + 20.f));
                        content->addChild(amt);
                        const int pack = i + 1;
                        auto* buy = PZUI::Button::create(PZCfg::kStrShopBuy, cw - 30.f, 34.f, PZCfg::kColGood, [pack]() { buyStorePack(pack); }, 11.f);
                        buy->setPosition(Vec2(cx, cy - 26.f));
                        content->addChild(buy);
                    }
                }
            }
        }
#endif
        (void)packs;
    };
    (*showTab)();

    base->schedule([rowRefresh, updateBal](float) {
        updateBal();
        if (*rowRefresh) (*rowRefresh)();
    }, 0.5f, "pz_shop_tick");

    auto* close = PZUI::Button::create(PZCfg::kStrClose, 210.f, 40.f, PZCfg::kColButton,
                                       [basePtr, onClosed]() { dismiss(basePtr, onClosed); }, 12.f);
    close->setPosition(Vec2(mid.x, closeY));
    base->addChild(close);
    return base;
}

// ================================================================================================
// Gifts: Gift Center, Lucky Spin, Welcome Back
// ================================================================================================

namespace {

std::string giftClock(int secs) {
    char b[24];
    if (secs >= 3600) std::snprintf(b, sizeof(b), "%d:%02d:%02d", secs / 3600, (secs / 60) % 60, secs % 60);
    else std::snprintf(b, sizeof(b), "%d:%02d", secs / 60, secs % 60);
    return b;
}

// Reward feedback shared by every gift: coin sound, glitter, burst, floating amount and a success haptic.
void giftFx(Node* host, const Vec2& at, int amount) {
    char t[32];
    std::snprintf(t, sizeof(t), "+%d", amount);
    PZUI::floatText(host, t, at + Vec2(0, 20), PZCfg::kColAccent, 24.f, 1800);
    PZUI::burst(host, at, PZCfg::kColAccent, 20, 120.f, 0.7f, 1700);
    PZUI::glitter(host, at, PZCfg::kColAccent, 14, 110.f, 0.9f, 1700);
    playCoin();
    PZHaptics::play(PZHaptics::Kind::Success);
}

enum class GiftKind { Daily, Hourly, Spin, Video, Chest, MissionChest };

// Tappable card with a drawn icon, a name and a status line. Content is centred on the node origin (anchor 0,0).
class GiftCard : public Node {
public:
    static GiftCard* make(GiftKind kind, float w, float h, std::function<void()> onTap) {
        auto* c = new GiftCard();
        if (!c->init()) { delete c; return nullptr; }
        c->autorelease();
        c->_kind = kind; c->_w = w; c->_h = h; c->_onTap = std::move(onTap);
        c->setContentSize(Size(w, h));
        c->setAnchorPoint(Vec2::ZERO);
        auto* l = EventListenerTouchOneByOne::create();
        l->setSwallowTouches(true);
        l->onTouchBegan = [c](Touch* t, Event*) {
            const Vec2 p = c->convertToNodeSpace(t->getLocation());
            if (!c->isVisible() || std::abs(p.x) > c->_w * 0.5f || std::abs(p.y) > c->_h * 0.5f) return false;
            c->setScale(0.96f);
            return true;
        };
        l->onTouchEnded = [c](Touch* t, Event*) {
            c->setScale(1.f);
            const Vec2 p = c->convertToNodeSpace(t->getLocation());
            if (std::abs(p.x) > c->_w * 0.5f || std::abs(p.y) > c->_h * 0.5f) return;
            PZHaptics::play(PZHaptics::Kind::Selection);
            if (c->_onTap) c->_onTap();
        };
        l->onTouchCancelled = [c](Touch*, Event*) { c->setScale(1.f); };
        c->getEventDispatcher()->addEventListenerWithSceneGraphPriority(l, c);
        return c;
    }

    void setState(const std::string& name, const std::string& status, bool ready) {
        _name = name; _status = status; _ready = ready;
        rebuild();
    }

private:
    void drawIcon(DrawNode* d, const Vec2& c) {
        const Color3B gold = PZCfg::kColAccent;
        const Color3B edge(120, 80, 10);
        switch (_kind) {
        case GiftKind::Daily: {            // wrapped present
            d->drawSolidRect(c + Vec2(-17, -15), c + Vec2(17, 8), PZUI::c4f(PZCfg::kColAccent2));
            d->drawSolidRect(c + Vec2(-19, 8), c + Vec2(19, 17), PZUI::c4f(gold));
            d->drawSolidRect(c + Vec2(-3, -15), c + Vec2(3, 17), PZUI::c4f(Color3B(255, 240, 200)));
            d->drawSolidCircle(c + Vec2(-7, 22), 6.f, 0.f, 12, PZUI::c4f(gold));
            d->drawSolidCircle(c + Vec2(7, 22), 6.f, 0.f, 12, PZUI::c4f(gold));
            break;
        }
        case GiftKind::Hourly: {           // clock
            d->drawSolidCircle(c, 21.f, 0.f, 28, PZUI::c4f(PZCfg::kColPanelEdge));
            d->drawSolidCircle(c, 17.f, 0.f, 28, PZUI::c4f(Color3B(240, 232, 210)));
            d->drawSegment(c, c + Vec2(0, 11), 2.2f, PZUI::c4f(Color3B(40, 30, 20)));
            d->drawSegment(c, c + Vec2(8, -3), 2.2f, PZUI::c4f(PZCfg::kColBad));
            d->drawDot(c, 2.5f, PZUI::c4f(Color3B(40, 30, 20)));
            break;
        }
        case GiftKind::Spin: {             // wheel
            const Color3B cols[2] = {PZCfg::kColAccent2, Color3B(70, 150, 240)};
            for (int i = 0; i < 8; ++i) {
                const float a0 = CC_DEGREES_TO_RADIANS(45.f * i), a1 = CC_DEGREES_TO_RADIANS(45.f * (i + 1));
                Vec2 tri[3] = {c, c + Vec2(std::cos(a0), std::sin(a0)) * 21.f, c + Vec2(std::cos(a1), std::sin(a1)) * 21.f};
                d->drawSolidPoly(tri, 3, PZUI::c4f(cols[i % 2]), 0.f, Color4F(0, 0, 0, 0), true);
            }
            d->drawCircle(c, 21.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColPanelEdge2), 2.5f);
            d->drawDot(c, 5.f, PZUI::c4f(gold));
            break;
        }
        case GiftKind::Video: {            // screen + play triangle
            d->drawSolidRect(c + Vec2(-22, -14), c + Vec2(22, 14), PZUI::c4f(PZCfg::kColPanelEdge));
            d->drawSolidRect(c + Vec2(-19, -11), c + Vec2(19, 11), PZUI::c4f(Color3B(14, 18, 32)));
            Vec2 tri[3] = {c + Vec2(-6, -8), c + Vec2(-6, 8), c + Vec2(9, 0)};
            d->drawSolidPoly(tri, 3, PZUI::c4f(PZCfg::kColGood), 0.f, Color4F(0, 0, 0, 0), true);
            break;
        }
        case GiftKind::Chest: {            // treasure chest
            d->drawSolidRect(c + Vec2(-20, -15), c + Vec2(20, 3), PZUI::c4f(Color3B(150, 92, 40)));
            Vec2 lid[6] = {c + Vec2(-20, 3), c + Vec2(-17, 12), c + Vec2(-8, 17), c + Vec2(8, 17), c + Vec2(17, 12), c + Vec2(20, 3)};
            d->drawSolidPoly(lid, 6, PZUI::c4f(Color3B(190, 120, 55)), 0.f, Color4F(0, 0, 0, 0), true);
            d->drawSolidRect(c + Vec2(-20, -3), c + Vec2(20, 1), PZUI::c4f(gold));
            d->drawSolidRect(c + Vec2(-4, -8), c + Vec2(4, 4), PZUI::c4f(gold));
            break;
        }
        case GiftKind::MissionChest: {     // star
            Vec2 pts[10];
            for (int i = 0; i < 10; ++i) {
                const float a = CC_DEGREES_TO_RADIANS(90.f + 36.f * i);
                const float r = (i % 2 == 0) ? 22.f : 9.f;
                pts[i] = c + Vec2(std::cos(a), std::sin(a)) * r;
            }
            d->drawSolidPoly(pts, 10, PZUI::c4f(gold), 2.f, PZUI::c4f(edge), false);
            break;
        }
        }
    }

    void rebuild() {
        removeAllChildren();
        const Color3B edge = _ready ? PZCfg::kColAccent : PZCfg::kColPanelEdge;
        addChild(PZUI::roundedRect(_w, _h, 14.f, Color3B(24, 30, 50), 0.97f, edge, _ready ? 1.f : 0.75f, _ready ? 4.f : 3.f));
        if (_ready) {
            addChild(PZUI::roundedRect(_w - 8.f, _h - 8.f, 11.f, PZCfg::kColAccent, 0.09f, PZCfg::kColAccent, 0.3f, 1.f));
        }
        auto* icon = DrawNode::create();
        drawIcon(icon, Vec2(0.f, _h * 0.5f - 46.f));
        addChild(icon);
        auto* name = PZUI::heading(_name, 8.f, PZCfg::kColText);
        name->setPosition(Vec2(0.f, -_h * 0.5f + 40.f));
        if (name->getContentSize().width > _w - 20.f) name->setScale((_w - 20.f) / name->getContentSize().width);
        addChild(name);
        auto* st = PZUI::heading(_status, 8.f, _ready ? PZCfg::kColGood : PZCfg::kColDim);
        st->setPosition(Vec2(0.f, -_h * 0.5f + 20.f));
        if (st->getContentSize().width > _w - 20.f) st->setScale((_w - 20.f) / st->getContentSize().width);
        addChild(st);
        if (_ready) {
            auto* dot = DrawNode::create();
            dot->drawSolidCircle(Vec2::ZERO, 6.f, 0.f, 14, PZUI::c4f(PZCfg::kColBad));
            dot->drawCircle(Vec2::ZERO, 6.f, 0.f, 14, false, PZUI::c4f(Color3B(255, 255, 255)), 1.5f);
            dot->setPosition(Vec2(_w * 0.5f - 14.f, _h * 0.5f - 14.f));
            addChild(dot);
            icon->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.6f, 1.08f), ScaleTo::create(0.6f, 1.f), nullptr)));
        }
    }

    GiftKind _kind = GiftKind::Daily;
    float _w = 0, _h = 0;
    std::string _name, _status;
    bool _ready = false;
    std::function<void()> _onTap;
};

} // namespace

Node* showGiftCenter(Node* parent, std::function<void()> onClosed, std::function<void()> onVideoGift) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    pg->refreshMissions();
    const Vec2 mid = PZLayout::popupCentre();
    const float panelW = std::min(700.f, PZLayout::safeRect().size.width - 40.f);
    const float panelH = std::min(430.f, PZLayout::popupMaxHeight());
    auto* base = openModal(parent, panelW, panelH, 330, 0.8f);

    auto* title = PZUI::heading(PZCfg::kStrGiftsTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, panelH * 0.5f - 40.f));
    base->addChild(title);

    const float gap = 14.f;
    const float cardW = std::min(204.f, (panelW - 48.f - gap * 2.f) / 3.f);
    const float cardH = std::min(136.f, (panelH - 150.f - gap) / 2.f);
    const float gridTop = mid.y + panelH * 0.5f - 78.f;
    const float x0 = mid.x - (cardW + gap);
    std::vector<std::pair<GiftKind, GiftCard*>> cards;
    Node* basePtr = base;

    auto refresh = std::make_shared<std::function<void()>>();
    auto claimFx = [basePtr, mid](int amount, const Vec2& at) { giftFx(basePtr, at, amount); (void)mid; };

    auto makeCard = [&](GiftKind kind, int idx, std::function<void(const Vec2&)> action) {
        const Vec2 pos(x0 + (idx % 3) * (cardW + gap), gridTop - cardH * 0.5f - (idx / 3) * (cardH + gap));
        auto* card = GiftCard::make(kind, cardW, cardH, [action, pos]() { action(pos); });
        if (!card) return;
        card->setPosition(pos);
        base->addChild(card);
        cards.push_back({kind, card});
    };

    makeCard(GiftKind::Daily, 0, [parent, refresh](const Vec2&) {
        PZPopups::showDailyReward(parent, [refresh]() { if (*refresh) (*refresh)(); });
    });
    makeCard(GiftKind::Hourly, 1, [pg, claimFx, refresh](const Vec2& at) {
        const int got = pg->claimHourly();
        if (got > 0) { claimFx(got, at); if (*refresh) (*refresh)(); }
        else PZHaptics::play(PZHaptics::Kind::Warning);
    });
    makeCard(GiftKind::Spin, 2, [parent, refresh](const Vec2&) {
        PZPopups::showLuckySpin(parent, [refresh]() { if (*refresh) (*refresh)(); });
    });
    makeCard(GiftKind::Video, 3, [basePtr, onVideoGift](const Vec2&) {
        dismiss(basePtr, onVideoGift);
    });
    makeCard(GiftKind::Chest, 4, [pg, claimFx, refresh](const Vec2& at) {
        const int got = pg->claimChest();
        if (got > 0) { claimFx(got, at); if (*refresh) (*refresh)(); }
        else PZHaptics::play(PZHaptics::Kind::Warning);
    });
    makeCard(GiftKind::MissionChest, 5, [pg, claimFx, refresh](const Vec2& at) {
        const int got = pg->claimMissionChest();
        if (got > 0) { claimFx(got, at); if (*refresh) (*refresh)(); }
        else PZHaptics::play(PZHaptics::Kind::Warning);
    });

    *refresh = [pg, cards]() {
        char b[48];
        for (const auto& e : cards) {
            switch (e.first) {
            case GiftKind::Daily:
                e.second->setState(PZCfg::kStrGiftDaily, pg->dailyAvailable() ? PZCfg::kStrGiftReady : PZCfg::kStrGiftClaimed, pg->dailyAvailable());
                break;
            case GiftKind::Hourly:
                if (pg->hourlyAvailable()) e.second->setState(PZCfg::kStrGiftHourly, PZCfg::kStrGiftReady, true);
                else { std::snprintf(b, sizeof(b), PZCfg::kStrGiftIn, giftClock(pg->hourlySecondsLeft()).c_str()); e.second->setState(PZCfg::kStrGiftHourly, b, false); }
                break;
            case GiftKind::Spin:
                e.second->setState(PZCfg::kStrGiftSpin, pg->spinAvailable() ? PZCfg::kStrGiftReady : PZCfg::kStrSpinVideo, pg->spinAvailable());
                break;
            case GiftKind::Video:
                if (pg->freeCoinsAvailable()) e.second->setState(PZCfg::kStrGiftVideo, PZCfg::kStrGiftReady, true);
                else { std::snprintf(b, sizeof(b), PZCfg::kStrGiftIn, giftClock(pg->freeCoinsSecondsLeft()).c_str()); e.second->setState(PZCfg::kStrGiftVideo, b, false); }
                break;
            case GiftKind::Chest:
                if (pg->chestLevelPending() > 0) e.second->setState(PZCfg::kStrGiftChest, PZCfg::kStrGiftReady, true);
                else { std::snprintf(b, sizeof(b), PZCfg::kStrGiftNextChest, pg->nextChestLevel()); e.second->setState(PZCfg::kStrGiftChest, b, false); }
                break;
            case GiftKind::MissionChest:
                e.second->setState(PZCfg::kStrGiftMissions, pg->missionChestAvailable() ? PZCfg::kStrGiftReady : PZCfg::kStrGiftMissionHint, pg->missionChestAvailable());
                break;
            }
        }
    };
    (*refresh)();
    base->schedule([refresh](float) { if (*refresh) (*refresh)(); }, 1.0f, "pz_gift_refresh");

    auto* close = PZUI::Button::create(PZCfg::kStrClose, 210.f, 40.f, PZCfg::kColButton, [basePtr, onClosed]() { dismiss(basePtr, onClosed); }, 12.f);
    close->setPosition(Vec2(mid.x, mid.y - panelH * 0.5f + 34.f));
    base->addChild(close);
    return base;
}

Node* showLuckySpin(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    const Vec2 mid = PZLayout::popupCentre();
    const float panelH = std::min(470.f, PZLayout::popupMaxHeight());
    auto* base = openModal(parent, 560.f, panelH, 340, 0.82f);
    Node* basePtr = base;

    auto* title = PZUI::heading(PZCfg::kStrSpinTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, panelH * 0.5f - 38.f));
    base->addChild(title);

    // Wheel: wedges + amounts, spinning as one node.
    const int n = static_cast<int>(PZCfg::kSpinRewards.size());
    const float R = std::min(128.f, (panelH - 200.f) * 0.5f);
    auto* wheel = Node::create();
    wheel->setPosition(mid + Vec2(0, 8));
    base->addChild(wheel);
    const float span = 360.f / n;
    const Color3B wedgeCols[3] = {Color3B(235, 130, 40), Color3B(40, 60, 110), Color3B(200, 70, 60)};
    for (int i = 0; i < n; ++i) {
        const float a0 = 90.f + span * i - span * 0.5f;                  // wedge i is centred at 90 + span*i (math degrees)
        auto* wedge = DrawNode::create();
        std::vector<Vec2> pts;
        pts.push_back(Vec2::ZERO);
        for (int s = 0; s <= 6; ++s) {
            const float a = CC_DEGREES_TO_RADIANS(a0 + span * s / 6.f);
            pts.push_back(Vec2(std::cos(a), std::sin(a)) * R);
        }
        wedge->drawSolidPoly(pts.data(), static_cast<unsigned>(pts.size()), PZUI::c4f(wedgeCols[(i == n - 1 && n % 2) ? 2 : i % 2]),
                             2.f, PZUI::c4f(PZCfg::kColPanelEdge2, 0.9f), true);
        wheel->addChild(wedge);
        const float am = CC_DEGREES_TO_RADIANS(90.f + span * i);
        auto* lbl = PZUI::heading(std::to_string(PZCfg::kSpinRewards[i]), 10.f, Color3B(255, 255, 255));
        lbl->setPosition(Vec2(std::cos(am), std::sin(am)) * (R * 0.66f));
        lbl->setRotation(-(90.f + span * i - 90.f));                       // reads outward from the hub
        wheel->addChild(lbl);
    }
    auto* rim = DrawNode::create();
    rim->drawCircle(Vec2::ZERO, R, 0.f, 48, false, PZUI::c4f(PZCfg::kColPanelEdge), 6.f);
    rim->drawSolidCircle(Vec2::ZERO, 16.f, 0.f, 24, PZUI::c4f(PZCfg::kColPanelEdge));
    rim->drawSolidCircle(Vec2::ZERO, 11.f, 0.f, 24, PZUI::c4f(PZCfg::kColAccent));
    wheel->addChild(rim);
    // Pointer (fixed, top).
    auto* ptr = DrawNode::create();
    Vec2 tri[3] = {Vec2(-12, 0), Vec2(12, 0), Vec2(0, -24)};
    ptr->drawSolidPoly(tri, 3, PZUI::c4f(PZCfg::kColAccent), 2.f, PZUI::c4f(Color3B(90, 50, 10)), true);
    ptr->setPosition(mid + Vec2(0, 8 + R + 14.f));
    base->addChild(ptr, 5);

    auto spinning = std::make_shared<bool>(false);
    auto spunToday = std::make_shared<bool>(!pg->spinAvailable());
    auto doSpin = std::make_shared<std::function<void(bool)>>();

    *doSpin = [=](bool freeSpin) {
        if (*spinning) return;
        *spinning = true;
        const int idx = pg->pickSpinIndex();
        const float jitter = (CCRANDOM_0_1() - 0.5f) * span * 0.6f;
        // Wedge i is centred at 90+span*i; rotating clockwise by theta moves it to 90+span*i-theta: land it at 90.
        const float theta = 360.f * 5.f + span * idx + jitter;
        auto lastSeg = std::make_shared<int>(0);
        auto* spin = ActionFloat::create(4.4f, 0.f, theta, [wheel, span, lastSeg](float v) {
            wheel->setRotation(v);
            const int seg = static_cast<int>(v / span);
            if (seg != *lastSeg) { *lastSeg = seg; PZHaptics::play(PZHaptics::Kind::Tick); }
        });
        wheel->runAction(Sequence::create(
            EaseCubicActionOut::create(spin),
            CallFunc::create([=]() {
                *spinning = false;
                const int got = pg->claimSpin(idx, freeSpin);
                giftFx(basePtr, mid + Vec2(0, 8), got);
                if (freeSpin) *spunToday = true;
            }),
            nullptr));
    };

    // Primary button: free spin if available, otherwise a rewarded-video spin.
    auto btnHolder = std::make_shared<PZUI::Button*>(nullptr);
    auto onTap = [=]() {
        if (*spinning) return;
        if (!*spunToday) {
            (*doSpin)(true);
            if (*btnHolder) (*btnHolder)->setText(PZCfg::kStrSpinVideo);
            return;
        }
        auto* ads = PZ::AdsController::shared();
        if (!ads->isRewardedAvailable()) {
            ads->requestRewarded();
            PZUI::banner(basePtr, PZCfg::kStrSpinTitle, PZCfg::kStrReviveLoading, PZCfg::kColAccent2, mid.y - panelH * 0.5f + 100.f, 1.0f, 1800);
            return;
        }
        ads->showRewardedWithCallback([=](bool earned) { if (earned) (*doSpin)(false); });
    };
    auto* spinBtn = PZUI::Button::create(pg->spinAvailable() ? PZCfg::kStrSpinBtn : PZCfg::kStrSpinVideo, 300.f, 52.f,
                                         pg->spinAvailable() ? PZCfg::kColGood : PZCfg::kColButton, onTap, 15.f);
    *btnHolder = spinBtn;
    spinBtn->setPosition(mid + Vec2(0, -R - 34.f));
    base->addChild(spinBtn);

    auto* close = PZUI::Button::create(PZCfg::kStrClose, 180.f, 34.f, Color3B(120, 124, 134), [basePtr, onClosed, spinning]() {
        if (*spinning) return;
        dismiss(basePtr, onClosed);
    }, 10.f);
    close->setPosition(mid + Vec2(0, -panelH * 0.5f + 24.f));
    base->addChild(close);
    return base;
}

Node* showComebackGift(Node* parent, std::function<void()> onClosed) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    if (pg->comebackCoins() <= 0) return nullptr;
    const Vec2 mid = PZLayout::popupCentre();
    auto* base = openModal(parent, 520.f, 360.f, 340, 0.8f);
    Node* basePtr = base;

    auto* title = PZUI::heading(PZCfg::kStrComebackTitle, 24.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, 134));
    base->addChild(title);
    char b[64];
    std::snprintf(b, sizeof(b), PZCfg::kStrComebackSub, pg->comebackDays());
    auto* sub = PZUI::heading(b, 9.f, PZCfg::kColDim);
    sub->setPosition(mid + Vec2(0, 100));
    base->addChild(sub);

    // Big present with the amount.
    auto* box = DrawNode::create();
    box->drawSolidRect(Vec2(-42, -34), Vec2(42, 20), PZUI::c4f(PZCfg::kColAccent2));
    box->drawSolidRect(Vec2(-48, 20), Vec2(48, 40), PZUI::c4f(PZCfg::kColAccent));
    box->drawSolidRect(Vec2(-7, -34), Vec2(7, 40), PZUI::c4f(Color3B(255, 240, 200)));
    box->drawSolidCircle(Vec2(-16, 50), 14.f, 0.f, 16, PZUI::c4f(PZCfg::kColAccent));
    box->drawSolidCircle(Vec2(16, 50), 14.f, 0.f, 16, PZUI::c4f(PZCfg::kColAccent));
    box->setPosition(mid + Vec2(0, 24));
    base->addChild(box);
    box->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.07f), ScaleTo::create(0.5f, 1.f), nullptr)));
    PZUI::glitter(base, mid + Vec2(0, 30), PZCfg::kColAccent, 12, 120.f, 1.2f, 10);

    std::snprintf(b, sizeof(b), "+%d", pg->comebackCoins());
    auto* amt = PZUI::heading(b, 22.f, PZCfg::kColAccent);
    amt->setPosition(mid + Vec2(0, -42));
    base->addChild(amt);

    auto claimed = std::make_shared<bool>(false);
    auto* btn = PZUI::Button::create(PZCfg::kStrDailyClaim, 300.f, 54.f, PZCfg::kColGood, [=]() {
        if (*claimed) return;
        *claimed = true;
        const int got = PZProgress::shared()->claimComeback();
        giftFx(basePtr, mid + Vec2(0, 24), got);
        basePtr->runAction(Sequence::create(DelayTime::create(0.9f), CallFunc::create([=]() { dismiss(basePtr, onClosed); }), nullptr));
    }, 16.f);
    btn->setPosition(mid + Vec2(0, -108));
    base->addChild(btn);
    btn->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.6f, 1.04f), ScaleTo::create(0.6f, 1.f), nullptr)));
    PZHaptics::play(PZHaptics::Kind::Success);
    return base;
}

// ================================================================================================
// Reminder offer (pre-permission)
// ================================================================================================

Node* showNotificationOffer(Node* parent) {
    if (!parent) return nullptr;
    auto* pg = PZProgress::shared();
    const Vec2 mid = PZLayout::popupCentre();
    const auto& asks = PZCfg::kNotifAsks;
    const auto& text = asks[static_cast<size_t>(pg->notifAsks()) % asks.size()];
    auto* base = openModal(parent, 560.f, 360.f, 330, 0.8f);

    // Bell glyph in a bronze medallion.
    auto* bell = DrawNode::create();
    bell->drawSolidCircle(Vec2::ZERO, 38.f, 0.f, 32, PZUI::c4f(Color3B(0, 0, 0), 0.35f));
    bell->drawSolidCircle(Vec2::ZERO, 35.f, 0.f, 32, PZUI::c4f(PZCfg::kColPanelEdge));
    bell->drawSolidCircle(Vec2::ZERO, 30.f, 0.f, 32, PZUI::c4f(Color3B(18, 22, 40)));
    const Color3B gold = PZCfg::kColAccent;
    Vec2 body[8] = {Vec2(-15, -8), Vec2(-12, 6), Vec2(-7, 14), Vec2(0, 17), Vec2(7, 14), Vec2(12, 6), Vec2(15, -8), Vec2(0, -8)};
    bell->drawSolidPoly(body, 8, PZUI::c4f(gold), 1.5f, PZUI::c4f(Color3B(120, 80, 10)), false);
    bell->drawSolidRect(Vec2(-18, -12), Vec2(18, -7), PZUI::c4f(gold));
    bell->drawDot(Vec2(0, -17), 4.f, PZUI::c4f(gold));
    bell->setPosition(mid + Vec2(0, 118));
    base->addChild(bell);
    bell->runAction(RepeatForever::create(Sequence::create(RotateTo::create(0.12f, 10.f), RotateTo::create(0.12f, -10.f),
                                                           RotateTo::create(0.12f, 6.f), RotateTo::create(0.12f, 0.f),
                                                           DelayTime::create(1.6f), nullptr)));

    auto* title = PZUI::heading(text.title, 20.f, PZCfg::kColAccent);
    title->setPosition(mid + Vec2(0, 62));
    if (title->getContentSize().width > 480.f) title->setScale(480.f / title->getContentSize().width);
    base->addChild(title);

    auto* body2 = PZUI::heading(text.body, 9.f, PZCfg::kColText);
    body2->setMaxLineWidth(440.f);
    body2->setAlignment(TextHAlignment::CENTER);
    body2->setPosition(mid + Vec2(0, 6));
    base->addChild(body2);

    Node* basePtr = base;
    auto* yes = PZUI::Button::create(PZCfg::kStrNotifYes, 380.f, 52.f, PZCfg::kColGood, [basePtr, parent]() {
        PZProgress::shared()->markNotifAsked();
        PZProgress::shared()->setNotifEnabled(true);
        dismiss(basePtr, [parent]() {
            PZNotif::requestPermission([parent](bool granted) {
                if (granted) {
                    PZUI::banner(parent, PZCfg::kStrNotifOn, "", PZCfg::kColGood, winSize().height * 0.55f, 1.2f, 1200);
                    PZHaptics::play(PZHaptics::Kind::Success);
                }
            });
        });
    }, 14.f);
    yes->setPosition(mid + Vec2(0, -78));
    base->addChild(yes);
    yes->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.6f, 1.04f), ScaleTo::create(0.6f, 1.f), nullptr)));

    auto* no = PZUI::Button::create(PZCfg::kStrNotifNo, 230.f, 38.f, Color3B(120, 124, 134), [basePtr]() {
        PZProgress::shared()->markNotifAsked();
        dismiss(basePtr, nullptr);
    }, 10.f);
    no->setPosition(mid + Vec2(0, -138));
    base->addChild(no);
    PZHaptics::play(PZHaptics::Kind::Light);
    return base;
}

// ================================================================================================
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
        if (_remind) {
            char rb[48];
            std::snprintf(rb, sizeof(rb), "%s %s", PZCfg::kStrNotifBtn, (pg->notifEnabled() && PZNotif::status() == PZNotif::Status::Granted) ? PZCfg::kStrOn : PZCfg::kStrOff);
            _remind->setText(rb);
        }
        char b[48];
        _lv->setString(std::to_string(pg->level()));
        _lv->setScale(pg->level() >= 100 ? 0.6f : (pg->level() >= 10 ? 0.8f : 1.f));
        std::snprintf(b, sizeof(b), PZCfg::kStrXpFormat, pg->xpIntoLevel(), pg->xpSpanOfLevel());
        _xpText->setString(b);
        std::snprintf(b, sizeof(b), PZCfg::kStrBest, pg->bestScore());
        _best->setString(b);
        updateNextUnlock(pg);
        animateXpBar(pg);
        if (_daily) _daily->setBadge(PZCfg::kEnableGifts ? pg->giftsReady() : (pg->dailyAvailable() ? 1 : 0));
        if (_missions) _missions->setBadge(pg->unclaimedMissions());
        if (_gadgets) _gadgets->setBadge(pg->selectedGadgetCount());
        if (_vibe) {
            std::snprintf(b, sizeof(b), "%s %s", PZCfg::kStrVibration, pg->hapticsEnabled() ? PZCfg::kStrOn : PZCfg::kStrOff);
            _vibe->setText(b);
        }
        updateFreeLabel();
    }

private:
    // The next gadget the player has not unlocked yet ("what am I levelling towards?").
    void updateNextUnlock(PZProgress* pg) {
        const PZCfg::GadgetTemplate* next = nullptr;
        for (const auto& g : PZCfg::kGadgetPool) {
            if (g.unlockLevel > pg->level() && (!next || g.unlockLevel < next->unlockLevel)) next = &g;
        }
        char b[64];
        if (next) std::snprintf(b, sizeof(b), PZCfg::kStrNextUnlock, next->name.c_str(), next->unlockLevel);
        else std::snprintf(b, sizeof(b), "%s", PZCfg::kStrAllUnlocked);
        _next->setString(b);
        _next->setScale(1.f);
        if (_next->getContentSize().width > _nextMaxW) _next->setScale(_nextMaxW / _next->getContentSize().width);
    }

    // Animates the bar from what the player last saw to the current XP; a level-up fills the bar, flashes the
    // badge, then restarts from empty.
    void animateXpBar(PZProgress* pg) {
        const float target = pg->levelFraction();
        const int level = pg->level();
        if (level == _xpLevel && std::abs(target - _shownFrac) < 0.002f) {
            PZUI::redrawBar(_xp, _xpBarW, 12.f, target, PZCfg::kColAccent2);
            return;
        }
        _xp->stopAllActions();
        const bool levelledUp = level > _xpLevel;
        const float from = _shownFrac;
        _xpLevel = level;
        _shownFrac = target;
        auto draw = [this](float v) { PZUI::redrawBar(_xp, _xpBarW, 12.f, v, PZCfg::kColAccent2); };
        if (levelledUp) {
            _xp->runAction(Sequence::create(
                ActionFloat::create(0.5f, from, 1.f, draw),
                CallFunc::create([this]() {
                    PZUI::burst(this, _xp->getParent()->convertToWorldSpace(_xp->getPosition()), PZCfg::kColAccent, 18, 80.f, 0.7f, 1200);
                    PZHaptics::play(PZHaptics::Kind::Success);
                    _lv->runAction(Sequence::create(ScaleTo::create(0.12f, 1.4f), EaseBackOut::create(ScaleTo::create(0.25f, 1.f)), nullptr));
                }),
                ActionFloat::create(0.5f, 0.f, target, draw), nullptr));
        } else {
            _xp->runAction(ActionFloat::create(0.6f, from, target, draw));
        }
    }

    void build() {
        auto* pg = PZProgress::shared();
        const Size win = winSize();
        const Rect safe = PZLayout::safeRect();

        // Compact meta column in the top-right corner: a small level plate plus a stack of full-width pills. The
        // big START button lives in the same corner further down, so the column is sized to end well above it
        // (measured from the safe rect, never from hard-coded pixels). Full-width pills also leave room for the
        // notification badge at the right edge without ever covering the label.
        const float colW = 224.f;
        const float colX = safe.getMaxX() - 14.f - colW * 0.5f;
        const float btnH = 30.f, gapY = 4.f;
        float y = safe.getMaxY() - 8.f;

        // Player plate: round level badge on the left; best score, XP counter, framed XP bar and the next
        // unlock on the right. The bar animates when XP changes (see refresh()).
        const float plateH = 66.f;
        auto* plate = Node::create();
        plate->addChild(PZUI::panel(colW, plateH, 12.f));
        plate->setPosition(Vec2(colX, y - plateH * 0.5f));
        addChild(plate);

        const float badgeX = -colW * 0.5f + 33.f;
        auto* badge = DrawNode::create();
        badge->drawSolidCircle(Vec2::ZERO, 27.f, 0.f, 28, PZUI::c4f(Color3B(0, 0, 0), 0.35f));
        badge->drawSolidCircle(Vec2::ZERO, 25.f, 0.f, 28, PZUI::c4f(PZCfg::kColPanelEdge));
        badge->drawSolidCircle(Vec2::ZERO, 21.f, 0.f, 28, PZUI::c4f(Color3B(14, 18, 32)));
        badge->drawCircle(Vec2::ZERO, 21.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColAccent, 0.9f), 2.f);
        badge->drawCircle(Vec2::ZERO, 25.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColPanelEdge2, 0.8f), 1.2f);
        badge->setPosition(Vec2(badgeX, 0.f));
        plate->addChild(badge);
        auto* lvTag = PZUI::heading("LV", 6.f, PZCfg::kColDim);
        lvTag->setPosition(Vec2(badgeX, 11.f));
        plate->addChild(lvTag);
        _lv = PZUI::heading("", 15.f, PZCfg::kColAccent);
        _lv->setPosition(Vec2(badgeX, -3.f));
        plate->addChild(_lv);

        const float rx = -colW * 0.5f + 68.f;                 // left edge of the right-hand block
        const float rw = colW * 0.5f - 12.f - rx;             // its width
        _xpText = PZUI::heading("", 6.f, PZCfg::kColAccent);
        _xpText->setAnchorPoint(Vec2(0.f, 0.5f));
        _xpText->setPosition(Vec2(rx, 20.f));
        plate->addChild(_xpText);
        _best = PZUI::heading("", 6.f, PZCfg::kColDim);
        _best->setAnchorPoint(Vec2(1.f, 0.5f));
        _best->setPosition(Vec2(rx + rw, 20.f));
        plate->addChild(_best);

        _xpBarW = rw - 6.f;
        _xp = PZUI::bar(_xpBarW, 12.f, pg->levelFraction(), PZCfg::kColAccent2);
        _xp->setPosition(Vec2(rx + rw * 0.5f, 3.f));
        plate->addChild(_xp);
        _shownFrac = pg->levelFraction();
        _xpLevel = pg->level();

        _next = PZUI::heading("", 6.f, PZCfg::kColDim);
        _next->setAnchorPoint(Vec2(0.f, 0.5f));
        _next->setPosition(Vec2(rx, -19.f));
        _nextMaxW = rw;
        plate->addChild(_next);
        y -= plateH + gapY;

        Node* host = _host;

        // Pills are laid out as they are created, so disabling a feature in the plist closes the gap instead of
        // leaving a hole in the corner.
        std::vector<PZUI::Button*> grid;
        if (PZCfg::kEnableDailyReward || PZCfg::kEnableGifts) {
            _daily = PZUI::Button::create(PZCfg::kEnableGifts ? PZCfg::kStrGiftsBtn : PZCfg::kStrDailyBtn, colW, btnH, PZCfg::kColButton, [this, host]() {
                if (PZCfg::kEnableGifts) {
                    PZPopups::showGiftCenter(host, [this]() { refresh(); }, [this]() { onFreeCoins(); });
                } else {
                    PZPopups::showDailyReward(host, [this]() { refresh(); });
                }
            }, 11.f);
            grid.push_back(_daily);
        }
        if (PZCfg::kEnableMissions) {
            _missions = PZUI::Button::create(PZCfg::kStrMissionsBtn, colW, btnH, Color3B(70, 150, 240), [this, host]() {
                PZPopups::showMissions(host, [this]() { refresh(); });
            }, 11.f);
            grid.push_back(_missions);
        }
        if (PZCfg::kEnableGadgets) {
            _gadgets = PZUI::Button::create(PZCfg::kStrGadgetsBtn, colW, btnH, Color3B(150, 105, 220), [this, host]() {
                PZPopups::showGadgets(host, [this]() { refresh(); });
            }, 11.f);
            grid.push_back(_gadgets);
        }
        if (PZCfg::kEnableFreeCoins) {
            _free = PZUI::Button::create(PZCfg::kStrShopBtn, colW, btnH, PZCfg::kColButton, [this, host]() {
                PZPopups::showShop(host, [this]() { refresh(); }, false);
            }, 11.f);
            grid.push_back(_free);
        }
        for (size_t i = 0; i < grid.size(); ++i) {
            grid[i]->setPosition(Vec2(colX, y - btnH * 0.5f - static_cast<float>(i) * (btnH + gapY)));
            addChild(grid[i]);
        }

        // Vibration lives in the bottom-right corner: out of the meta cluster, above the banner, below the cutout.
        _vibe = PZUI::Button::create("", 126.f, 30.f, Color3B(120, 124, 134), [this]() {
            auto* p = PZProgress::shared();
            p->setHapticsEnabled(!p->hapticsEnabled());
            PZHaptics::play(PZHaptics::Kind::Medium);   // instant preview when turning on
            refresh();
        }, 8.f);
        _vibe->setPosition(Vec2(safe.getMaxX() - 16.f - 63.f, safe.getMinY() + 32.f));
        addChild(_vibe);

        if (PZCfg::kEnableNotifications) {
            Node* hostNode = _host;
            _remind = PZUI::Button::create("", 176.f, 30.f, Color3B(120, 124, 134), [this, hostNode]() {
                const bool on = !(PZProgress::shared()->notifEnabled() && PZNotif::status() == PZNotif::Status::Granted);
                PZNotif::setEnabledByPlayer(hostNode, on);
                runAction(Sequence::create(DelayTime::create(0.4f), CallFunc::create([this]() { refresh(); }), nullptr));
            }, 8.f);
            _remind->setPosition(Vec2(safe.getMaxX() - 16.f - 126.f - 8.f - 88.f, safe.getMinY() + 32.f));
            addChild(_remind);
            PZNotif::refreshStatus();
            runAction(Sequence::create(DelayTime::create(1.2f), CallFunc::create([this]() { PZNotif::reschedule(); refresh(); }), nullptr));
        }

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
                             win.height * 0.55f, 1.0f, 1200);
            } else {
                PZ::AdsController::shared()->requestRewarded();
            }
        }, 0.35f, "pz_menu_free_poll");
    }

    void updateFreeLabel() {
        if (!_free) return;
        // The SHOP pill carries a badge while a free-coin video is ready (the video itself lives in the shop).
        if (_freeText.empty()) { _freeText = PZCfg::kStrShopBtn; _free->setText(_freeText); }
        _free->setBadge(PZProgress::shared()->freeCoinsAvailable() ? 1 : 0);
    }

    void onFreeCoins() {
        auto* pg = PZProgress::shared();
        if (!pg->freeCoinsAvailable()) {
            char b[48];
            std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsWait, clockText(pg->freeCoinsSecondsLeft()).c_str());
            PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, b, PZCfg::kColAccent2, winSize().height * 0.55f, 1.2f, 1200);
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
                         winSize().height * 0.55f, 1.0f, 1200);
            return;
        }
        ads->showRewardedWithCallback([this](bool earned) {
            _freePending = false;
            if (!earned) {
                PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, PZCfg::kStrVideoNoAd, PZCfg::kColBad,
                             winSize().height * 0.55f, 1.0f, 1200);
                return;
            }
            const int got = PZProgress::shared()->claimFreeCoins(PZCfg::kFreeCoinsAmount);
            if (got <= 0) return;
            char b[64];
            std::snprintf(b, sizeof(b), PZCfg::kStrFreeCoinsSub, got);
            PZUI::banner(_host, PZCfg::kStrFreeCoinsTitle, b, PZCfg::kColGood, winSize().height * 0.55f, 1.3f, 1200);
            if (_free) {
                const Vec2 world = _free->convertToWorldSpace(Vec2::ZERO);
                PZUI::burst(_host, _host->convertToNodeSpace(world), PZCfg::kColAccent, 20, 100.f, 0.7f, 1200);
            }
            playCoin();
            PZHaptics::play(PZHaptics::Kind::Success);
            refresh();
        });
    }

    Node* _host = nullptr;
    Label* _lv = nullptr;
    Label* _best = nullptr;
    Label* _xpText = nullptr;
    Label* _next = nullptr;
    float _nextMaxW = 140.f;
    DrawNode* _xp = nullptr;
    float _xpBarW = 140.f;
    float _shownFrac = 0.f;
    int _xpLevel = 1;
    PZUI::Button* _daily = nullptr;
    PZUI::Button* _missions = nullptr;
    PZUI::Button* _gadgets = nullptr;
    PZUI::Button* _free = nullptr;
    PZUI::Button* _vibe = nullptr;
    PZUI::Button* _remind = nullptr;
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
    // A welcome-back gift (player was away) takes the first slot; the daily reward follows.
    if (allowDailyPopup && PZProgress::shared()->comebackCoins() > 0) {
        menuLayer->runAction(Sequence::create(DelayTime::create(0.7f), CallFunc::create([menuLayer]() {
            showComebackGift(menuLayer, [menuLayer]() { refreshMenuWidgets(menuLayer, true); });
        }), nullptr));
        return;
    }
    if (allowDailyPopup && PZProgress::shared()->dailyAvailable()) {
        menuLayer->runAction(Sequence::create(DelayTime::create(0.7f), CallFunc::create([menuLayer]() {
            if (PZProgress::shared()->dailyAvailable()) {
                showDailyReward(menuLayer, [menuLayer]() { refreshMenuWidgets(menuLayer, false); });
            }
        }), nullptr));
    }
}

} // namespace PZPopups
