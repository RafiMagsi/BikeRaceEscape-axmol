//
//  V2Share.h
//  Result cards: renders a 1200x630 image of a daily result (score, medal, modifier, the player's own bike) with a
//  RenderTexture, saves it to the writable path and hands it to the native share sheet (iOS UIActivityViewController,
//  Android ACTION_SEND through a FileProvider). No social SDK, and sharing never grants a reward (PRODUCT_PLAN 8.1).
//
#pragma once

#include "PZLegacyCompat.h"
#include "V2/V2Daily.h"
#include <functional>
#include <string>

namespace V2 {
namespace Share {

// Builds the share text for a result ("I scored 1,234 on today's ...").
std::string shareText(const Daily::Result& r);

// Renders the card, saves it as PNG and opens the share sheet. `done(path)` runs once the PNG exists (empty on failure);
// the sheet itself is handed over right after. `host` is only used to keep the render alive on the scene.
void shareDailyResult(ax::Node* host, const Daily::Result& r, std::function<void(const std::string&)> done = nullptr);

// A normal (non-daily) run worth sharing: Crown claimed, rival beaten or a distance record (results headline, PRODUCT_PLAN 24.5).
struct RunCard {
    std::string headline;        // "CROWN CLAIMED"
    int distance = 0;
    std::string moment;          // best mastery moment ("" none)
    std::string next;            // next target ("" none)
};
void shareRun(ax::Node* host, const RunCard& c);

// Platform part (V2ShareApple.mm / Java ShareBridge): shows the sheet for an existing PNG.
void sendImage(const std::string& path, const std::string& text);

} // namespace Share
} // namespace V2
