//
//  PZPopups.h
//  Code-drawn popups and widgets: revive offer, daily reward, missions, game-over results,
//  main-menu widgets. Styling and text come from PZGameConfig.h.
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZProgress.h"
#include <functional>

namespace PZPopups {

// "CONTINUE?" offer with countdown. Exactly one of the callbacks fires, after the popup removed itself.
ax::Node* showRevive(ax::Node* parent, int cost, int coinsOwned, std::function<void()> onRevive,
                     std::function<void()> onDecline);

// Daily reward calendar with a CLAIM button.
ax::Node* showDailyReward(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Today's missions with progress and CLAIM buttons.
ax::Node* showMissions(ax::Node* parent, std::function<void()> onClosed = nullptr);

// Adds best / coins / XP-level / mission feedback to the game-over overlay and animates the score.
void attachResults(ax::Node* gameOverLayer, ax::Label* distanceLabel, const PZRunStats& stats,
                   const PZRunResult& result);

// Main-menu additions (level plate, DAILY and MISSIONS buttons). Safe to call again to refresh.
void attachMenuWidgets(ax::Node* menuLayer);
void refreshMenuWidgets(ax::Node* menuLayer, bool allowDailyPopup);

// Runs `fn` on the next frame (safe way to remove a popup from inside its own button callback).
void later(std::function<void()> fn);

} // namespace PZPopups
