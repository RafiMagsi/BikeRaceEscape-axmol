//
//  PZPopups.h
//  Code-drawn popups and widgets: revive offer, daily reward, missions, game-over results,
//  main-menu widgets. Styling and text come from PZGameConfig.h; placement from PZLayout.h.
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZProgress.h"
#include "V2/V2Daily.h"
#include <functional>

namespace PZPopups {

// "SAVE ME!" offer with countdown: revive for coins and/or a rewarded video. Exactly one callback fires, after
// the popup removed itself. `videoSupported` shows the free option; it starts as VIDEO LOADING... and enables
// itself as soon as the reward ad is ready (the popup polls, so a slow network never hides the option).
ax::Node* showRevive(ax::Node* parent, int cost, int coinsOwned, bool videoSupported,
                     std::function<void()> onCoins, std::function<void()> onVideo, std::function<void()> onDecline,
                     float countdown = -1.f);   // countdown < 0 = PZCfg::kReviveCountdown

// Daily reward calendar with a CLAIM button.
ax::Node* showDailyReward(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Today's missions with progress and CLAIM buttons.
ax::Node* showMissions(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Adds stats / XP / mission feedback to the game-over overlay and animates the score. `claimDoubleCoins` is
// invoked after a rewarded video completes; it must grant and return the bonus coins (0 = nothing granted).
void attachResults(ax::Node* gameOverLayer, ax::Label* distanceLabel, const PZRunStats& stats,
                   const PZRunResult& result, std::function<int()> claimDoubleCoins = nullptr,
                   std::function<void()> onReviveCoins = nullptr,   // both set: REVIVE (coins) + WATCH AD buttons are shown
                   std::function<void()> onReviveVideo = nullptr);

// Gadget loadout: pick up to kGadgetSlots unlocked gadgets. `onChanged` fires after every equip/unequip.
ax::Node* showGadgets(ax::Node* parent, std::function<void()> onClosed = nullptr);

// In-run gadget picker (opened from the top-bar GADGETS button). Exactly one callback fires after the popup closed:
// onChoose(id) for a tapped unlocked gadget (stock may be 0: the caller opens the shop), or onCancel.
ax::Node* showGadgetPicker(ax::Node* parent, std::function<void(const std::string&)> onChoose, std::function<void()> onCancel);

// Daily seeded run (Version 2): today's modifier, ranked / practice start, medals, share. `onStart` runs after the
// popup closed and Daily::begin() chose the mode (the caller starts the normal Play flow).
ax::Node* showDailyChallenge(ax::Node* parent, std::function<void()> onStart);
// Shown after a daily run: score, medal, reward, SHARE. `coinsGranted` is what this result paid (ranked, first time).
ax::Node* showDailyResult(ax::Node* parent, const V2::Daily::Result& r, bool ranked, int rankedScore, int coinsGranted,
                          std::function<void()> onClosed = nullptr);

// Settings: sound effects, music, vibration, reminders and the Google privacy-choices form.
ax::Node* showSettings(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Main-menu additions (level plate, DAILY / MISSIONS buttons, vibration toggle). Safe to call again to refresh.
// True while a popup (shop, settings, ...) is open on `parent`; closeTopModal removes it (Android back key).
bool hasModal(ax::Node* parent);
bool closeTopModal(ax::Node* parent);

void attachMenuWidgets(ax::Node* menuLayer);
void refreshMenuWidgets(ax::Node* menuLayer, bool allowDailyPopup);

// Live gameplay furniture for the menu screens: the player's bike runs along the bottom of the screen, hops, kicks up
// dust and sweeps up coins. `interactive` also lets a tap on the lane make it jump (main menu); other screens get the
// same running bike without stealing touches. Never used on the game-over screen.
void attachMenuShowcase(ax::Node* menuLayer, bool interactive = false);

// Shop: power-up charges for coins (tab 1) and coins from a rewarded video / store packs (tab 2).
ax::Node* showShop(ax::Node* parent, std::function<void()> onClosed = nullptr, bool startOnCoins = false);

// Gift Center: daily reward, hourly gift, lucky spin, video gift, level chest and mission chest in one place.
// `onVideoGift` runs (after the hub closes) when the VIDEO GIFT card is tapped.
ax::Node* showGiftCenter(ax::Node* parent, std::function<void()> onClosed, std::function<void()> onVideoGift);
ax::Node* showLuckySpin(ax::Node* parent, std::function<void()> onClosed = nullptr);
ax::Node* showComebackGift(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Pre-permission "remind me?" offer (the OS dialog only appears if the player says yes). Copy rotates per ask.
ax::Node* showNotificationOffer(ax::Node* parent);

// Runs `fn` on the next frame (safe way to remove a popup from inside its own button callback).
void later(std::function<void()> fn);

} // namespace PZPopups
