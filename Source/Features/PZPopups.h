//
//  PZPopups.h
//  Code-drawn popups and widgets: revive offer, daily reward, missions, game-over results,
//  main-menu widgets. Styling and text come from PZGameConfig.h; placement from PZLayout.h.
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZProgress.h"
#include <functional>

namespace PZPopups {

// "SAVE ME!" offer with countdown: revive for coins and/or a rewarded video. Exactly one callback fires, after
// the popup removed itself. `videoSupported` shows the free option; it starts as VIDEO LOADING... and enables
// itself as soon as the reward ad is ready (the popup polls, so a slow network never hides the option).
ax::Node* showRevive(ax::Node* parent, int cost, int coinsOwned, bool videoSupported,
                     std::function<void()> onCoins, std::function<void()> onVideo, std::function<void()> onDecline);

// Daily reward calendar with a CLAIM button.
ax::Node* showDailyReward(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Today's missions with progress and CLAIM buttons.
ax::Node* showMissions(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Adds stats / XP / mission feedback to the game-over overlay and animates the score. `claimDoubleCoins` is
// invoked after a rewarded video completes; it must grant and return the bonus coins (0 = nothing granted).
void attachResults(ax::Node* gameOverLayer, ax::Label* distanceLabel, const PZRunStats& stats,
                   const PZRunResult& result, std::function<int()> claimDoubleCoins = nullptr);

// Gadget loadout: pick up to kGadgetSlots unlocked gadgets. `onChanged` fires after every equip/unequip.
ax::Node* showGadgets(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Main-menu additions (level plate, DAILY / MISSIONS buttons, vibration toggle). Safe to call again to refresh.
void attachMenuWidgets(ax::Node* menuLayer);
void refreshMenuWidgets(ax::Node* menuLayer, bool allowDailyPopup);

// Runs `fn` on the next frame (safe way to remove a popup from inside its own button callback).
void later(std::function<void()> fn);

} // namespace PZPopups
