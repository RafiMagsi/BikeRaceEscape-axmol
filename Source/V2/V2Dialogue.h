//
//  V2Dialogue.h
//  Start-of-run rival dialogue (PRODUCT_PLAN 21.6): short, skippable, shown only before the run while the world is frozen.
//  Sequences, speakers, lines, expressions, portrait sets and the once / session / daily policy are all data in
//  CompleteGameInfo.plist (v2.presentation.dialogueSequences). Presentation only: it never touches score, RNG, physics or replay
//  input; ticks do not advance behind it. One cleanup path (finish) restores everything on complete, skip or scene exit.
//
#pragma once

#include "PZLegacyCompat.h"
#include <functional>
#include <string>

namespace V2 {

struct DialogueContext {
    bool replay = false;          // replay tests never show dialogue
    bool dailyRun = false;        // a ranked or practice daily run is about to start
    int dateKey = 0;
};

class DialogueDirector : public ax::Node {
public:
    static DialogueDirector* create();

    // Chooses the highest-priority eligible sequence. "" when nothing should be shown.
    static std::string pick(const DialogueContext& ctx);

    // Shows `id` over `host`. `onDone` runs exactly once when the sequence completes or is skipped (not when the scene exits).
    bool show(const std::string& id, ax::Node* host, std::function<void()> onDone);
    void cancel();                // scene exit / interruption: remove everything, no callback
    bool active() const { return _root != nullptr; }

    void onExit() override;

private:
    void showLine();
    void advance();
    void finish(bool skipped);

    ax::Node* _root = nullptr;
    ax::Sprite* _spr[2] = {nullptr, nullptr};
    ax::Label* _name = nullptr;
    ax::Label* _text = nullptr;
    ax::Label* _hint = nullptr;
    std::string _id;
    int _line = 0;
    float _skipX0 = 0.f, _skipY0 = 0.f, _skipX1 = 0.f, _skipY1 = 0.f;
    bool _reduced = false;
    std::function<void()> _done;
};

} // namespace V2
