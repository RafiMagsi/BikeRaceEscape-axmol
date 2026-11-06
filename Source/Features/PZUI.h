//
//  PZUI.h
//  Tiny code-drawn UI kit shared by the meta features (no sprite sheets, themed from PZGameConfig.h).
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZGameConfig.h"
#include <functional>
#include <string>

namespace PZUI {

inline ax::Color4F c4f(const ax::Color3B& c, float a = 1.f) {
    return ax::Color4F(c.r / 255.f, c.g / 255.f, c.b / 255.f, a);
}

// Arcade-font label with system-font fallback. `size` is in design units.
ax::Label* label(const std::string& text, float size, const ax::Color3B& color = PZCfg::kColText);

// Heading style used across the game: bold gold arcade text with a dark outline.
ax::Label* heading(const std::string& text, float size, const ax::Color3B& color = PZCfg::kColAccent);

// Original-game panel: dark navy body, bronze ring with a light-gold inner line and a soft drop shadow.
ax::Node* panel(float w, float h, float radius = 22.f);

// The game's own coin art (falls back to a drawn coin). `size` is the on-screen diameter.
ax::Node* coinIcon(float size);

// Code-drawn icon for a gadget / power-up id (magnet, turbo, shield, jetpack, headstart, rocket, helmet, gun), drawn to
// look like what it does. `size` is the on-screen diameter; unknown ids get a lettered badge.
ax::Node* gadgetIcon(const std::string& id, float size, const std::string& fallbackLetter = "?", bool dim = false);

// Rounded rectangle centred on (0,0) of the returned node.
ax::DrawNode* roundedRect(float w, float h, float radius, const ax::Color3B& fill, float fillAlpha,
                          const ax::Color3B& border, float borderAlpha = 1.f, float borderWidth = 2.f);

// Dimmed full-screen layer that swallows touches; add your panel on top of it.
ax::LayerColor* modalBase(float dim = 0.72f);

// Pressable code-drawn button (centred). Fires `onTap` on release inside the button.
class Button : public ax::Node {
public:
    static Button* create(const std::string& text, float w, float h, const ax::Color3B& color,
                          std::function<void()> onTap, float fontSize = 15.f);
    void setText(const std::string& text);
    void setActive(bool active);          // greyed out + ignores taps when false
    void setSwallow(bool swallow);
    void setBadge(int count);             // small red counter bubble (0 hides it)
    bool active() const { return _active; }

private:
    bool initButton(const std::string& text, float w, float h, const ax::Color3B& color,
                    std::function<void()> onTap, float fontSize);
    void rebuild();

    std::string _text;
    float _w = 0, _h = 0, _fontSize = 15;
    ax::Color3B _color;
    std::function<void()> _onTap;
    ax::DrawNode* _bg = nullptr;
    ax::DrawNode* _glow = nullptr;   // animated frame: a warm ring that breathes around the pill
    ax::Label* _label = nullptr;
    ax::Node* _badge = nullptr;
    bool _active = true;
    bool _pressed = false;
};

// Radial spark burst (fades out and removes itself).
void burst(ax::Node* parent, const ax::Vec2& pos, const ax::Color3B& color, int count, float radius, float duration,
           int z = 900);

// Twinkling four-point sparkles (gold glitter): used for rewards, pickups and celebrations.
void glitter(ax::Node* parent, const ax::Vec2& pos, const ax::Color3B& color, int count, float radius, float duration = 0.7f,
             int z = 900);

// Text that floats up and fades away.
void floatText(ax::Node* parent, const std::string& text, const ax::Vec2& pos, const ax::Color3B& color,
               float size = 14.f, int z = 950);

// Big banner that slides in, holds, and slides out (stage changes, new best, ...). Banners are QUEUED: only one is on
// screen at a time, the next one waits `kGap` seconds after the previous one starts to leave, so a burst of events
// (mission complete + stage up + get ready ...) is never stacked on top of itself. Duplicate requests are merged.
void banner(ax::Node* parent, const std::string& title, const std::string& sub, const ax::Color3B& color,
            float y, float hold = 1.2f, int z = 960);

// Entrance animation: the node slides in from the side (or from below) while scaling up and fading in. The node keeps
// its own scale/position as the target, so it is safe on laid-out widgets and labels with custom scales.
void enterIn(ax::Node* node, float delay = 0.f, bool fromRight = false, float distance = 40.f);

// Runs enterIn() over the children of `parent` one after the other (menus, popup content, HUD rows).
void stagger(ax::Node* parent, float step = 0.05f, float startDelay = 0.03f, int skipFirst = 0);

// Screen transition helpers: fade from / to black. `then` runs when the fade-out has finished.
void fadeInScreen(ax::Node* layer, float duration = 0.28f);
void fadeOutScreen(ax::Node* layer, float duration, std::function<void()> then = nullptr);

// Decaying positional shake that restores `node` to `home`.
void shake(ax::Node* node, const ax::Vec2& home, float amplitude, float duration);

// Horizontal progress bar centred at (0,0); value 0..1.
ax::DrawNode* bar(float w, float h, float value, const ax::Color3B& fill);
void redrawBar(ax::DrawNode* node, float w, float h, float value, const ax::Color3B& fill);

bool fileExists(const std::string& name);

} // namespace PZUI
