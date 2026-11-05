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
    ax::Label* _label = nullptr;
    ax::Node* _badge = nullptr;
    bool _active = true;
    bool _pressed = false;
};

// Radial spark burst (fades out and removes itself).
void burst(ax::Node* parent, const ax::Vec2& pos, const ax::Color3B& color, int count, float radius, float duration,
           int z = 900);

// Text that floats up and fades away.
void floatText(ax::Node* parent, const std::string& text, const ax::Vec2& pos, const ax::Color3B& color,
               float size = 14.f, int z = 950);

// Big banner that slides in, holds, and slides out (stage changes, new best, ...).
void banner(ax::Node* parent, const std::string& title, const std::string& sub, const ax::Color3B& color,
            float y, float hold = 1.2f, int z = 960);

// Decaying positional shake that restores `node` to `home`.
void shake(ax::Node* node, const ax::Vec2& home, float amplitude, float duration);

// Horizontal progress bar centred at (0,0); value 0..1.
ax::DrawNode* bar(float w, float h, float value, const ax::Color3B& fill);
void redrawBar(ax::DrawNode* node, float w, float h, float value, const ax::Color3B& fill);

bool fileExists(const std::string& name);

} // namespace PZUI
