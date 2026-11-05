// TEMPORARY simulator QA channel (copied in/out of Source/Features by the assistant). Polls /tmp/pzqa/cmd.
#include "PZQA.h"
#include "PZPopups.h"
#include "PZRunDirector.h"
#include "PZGGameFieldScene.h"
#include "PZGGamefieldPlatforms.h"
#include "PZGMainMenuScene.h"
#include "PZGCharacterMenu.h"
#include "PZSettingsController.h"
#include "PZProgress.h"
#include "constants.h"
#include "Features/PZLayout.h"
#include <typeinfo>
#include <functional>
#include "Ads/AdsController.h"
#include <fstream>
#include <sstream>
#include <cstdio>

USING_NS_AX;

namespace {
bool g_installed = false;
int g_key = 0;
bool g_bot = false;
bool g_waitBlast = false;
float g_botHold = 0.f;

Menu* findMenu(Node* n) {
    if (auto* m = dynamic_cast<Menu*>(n->getChildByTag(kBaseMenuItemTag))) return m;
    return nullptr;
}
void activateTag(Node* layer, int tag) {
    if (auto* m = findMenu(layer)) {
        if (auto* it = dynamic_cast<MenuItem*>(m->getChildByTag(tag))) it->activate();
    }
}
template <class T> T* findLayer(Node* scene) {
    if (!scene) return nullptr;
    for (auto* c : scene->getChildren()) if (auto* t = dynamic_cast<T*>(c)) return t;
    return nullptr;
}

void botTick(float dt) {
    auto* gf = PZGGameFieldScene::shared();
    if (g_waitBlast && gf && gf->run && !gf->gameOver && gf->platformLayer()) {
        int n = 0;
        auto* pl = gf->platformLayer();
        const float right = Director::getInstance()->getWinSize().width;
        for (int i = 0; i < pl->obstacleSlots(); ++i) {
            auto* o = pl->obstacleAt(i);
            if (!o || o->dead || !o->destroyCharacter || !o->sprite) continue;
            const Rect b = o->worldBox();
            if (b.getMinX() > gf->character->loc.x + 60.f && b.getMaxX() < right) ++n;
        }
        if (n >= 2) { g_waitBlast = false; gf->run->grantByName("blast"); }
    }
    if (!g_bot || !gf || !gf->character || gf->gameOver || gf->gamePause) return;
    auto* ch = gf->character;
    if (g_botHold > 0.f) { g_botHold -= dt; if (g_botHold <= 0.f) ch->jumpEnd(); return; }
    if (ch->jump_counter != 0) return;
    auto* pl = gf->platformLayer();
    if (!pl) return;
    const float k = kDeviceScale();
    const float speed = gf->gf_speed * gf->speedMultiplier() * 64.f * k;   // units / s
    const float look = std::max(140.f, speed * 0.34f);
    const float rx = ch->loc.x;
    bool need = false;
    float gy;
    if (!pl->groundYAt(rx + look * 0.9f, gy) && pl->groundYAt(rx, gy)) need = true;
    for (int i = 0; i < pl->obstacleSlots() && !need; ++i) {
        auto* o = pl->obstacleAt(i);
        if (!o || o->dead || !o->destroyCharacter || !o->sprite) continue;
        Rect b = o->sprite->getBoundingBox(); b.origin += o->getPosition();
        if (b.getMinX() > rx && b.getMinX() < rx + look && b.getMinY() < ch->loc.y + 120.f) need = true;
    }
    if (need) { ch->jump(); g_botHold = 0.55f; }
}

void run(const std::string& line) {
    std::istringstream is(line);
    std::string cmd, arg;
    is >> cmd >> arg;
    Scene* scene = Director::getInstance()->getRunningScene();
    AXLOGI("[QA] {} {}", cmd, arg);
    auto* gf = PZGGameFieldScene::shared();
    if (cmd == "play") {
        if (auto* m = findLayer<PZGMainMenuScene>(scene)) activateTag(m, 3);
        else if (auto* c = findLayer<PZGCharacterMenu>(scene)) activateTag(c, 4);
    } else if (cmd == "die") {
        if (gf) gf->gameOverCallBack(true);
    } else if (cmd == "restart") {
        if (gf) gf->gameRestart();
    } else if (cmd == "grant") {
        if (gf && gf->run) {
            gf->run->grantByName(arg);
        }
    } else if (cmd == "jetpack") {
        if (gf && gf->run) gf->run->grant(PZRunDirector::PowerUp::Jetpack);
    } else if (cmd == "pos") {
        std::ofstream out("/tmp/pzqa/out", std::ios::app);
        if (gf && gf->character) {
            auto* ch = gf->character;
            out << "loc " << ch->loc.x << "," << ch->loc.y
                << " jumps " << ch->jump_counter << " jump_enabled " << ch->jump_enabled;
            if (ch->p_body) out << " body_y " << ch->p_body->GetPosition().y
                                << " vy " << ch->p_body->GetLinearVelocity().y
                                << " grav " << ch->p_body->GetGravityScale();
            out << "\n";
        }
        out.close();
    } else if (cmd == "coinsinfo") {
        std::ofstream out("/tmp/pzqa/out", std::ios::app);
        int live = 0, onscreen = 0;
        auto* p = gf ? gf->platformLayer() : nullptr;
        if (p) {
            const float w = Director::getInstance()->getWinSize().width;
            for (int i = 0; i < p->coinSlots(); ++i) {
                PZGCoin* c = p->coinAt(i);
                if (!c) continue;
                ++live;
                if (c->getPosition().x > 0 && c->getPosition().x < w) ++onscreen;
            }
        }
        out << "coins live " << live << " onscreen " << onscreen
            << " dist " << (gf ? gf->gf_distance : 0.f) << "\n";
        out.close();
    } else if (cmd == "coins") {
        auto* sc = PZSettingsController::shared();
        sc->coins = std::atoi(arg.c_str());
        if (gf) gf->gf_coins = sc->coins;
    } else if (cmd == "dist") {
        if (gf) gf->gf_distance += std::atof(arg.c_str());
    } else if (cmd == "noads") {
        PZSettingsController::shared()->removeAds = true;
        PZ::AdsController::shared()->setAdsEnabled(false);
        PZ::AdsController::shared()->hideBanner();
    } else if (cmd == "notif") {
        if (auto* m = findLayer<PZGMainMenuScene>(scene)) PZPopups::showNotificationOffer(m);
    } else if (cmd == "obstdie") {
        // Drop the nearest lethal obstacle ahead onto the rider (reproduces "killed by an obstacle").
        if (gf && gf->platformLayer() && gf->character) {
            auto* pl = gf->platformLayer();
            PZGObstacle* best = nullptr; float bx = 1e9f;
            for (int i = 0; i < pl->obstacleSlots(); ++i) {
                auto* o = pl->obstacleAt(i);
                if (!o || o->dead || !o->destroyCharacter || !o->sprite) continue;
                if (o->getPositionX() > gf->character->loc.x && o->getPositionX() < bx) { bx = o->getPositionX(); best = o; }
            }
            if (best) best->setPosition(ccp(gf->character->loc.x, best->getPositionY()));
        }
    } else if (cmd == "waitblast") {
        g_waitBlast = true;
    } else if (cmd == "bot") {
        g_bot = (arg != "off");
    } else if (cmd == "jump") {
        if (gf && gf->character) gf->character->jump();
    } else if (cmd == "jumpend") {
        if (gf && gf->character) gf->character->jumpEnd();
    } else if (cmd == "layout") {
        auto* d = Director::getInstance();
        const Size win = d->getWinSize();
        const Rect safe = PZLayout::safeRect();
        std::ofstream out("/tmp/pzqa/out", std::ios::app);
        out << "win " << win.width << "x" << win.height
            << " safe [" << safe.origin.x << "," << safe.origin.y << " " << safe.size.width << "x" << safe.size.height
            << "] banner " << PZLayout::bannerReserve() << "\n";
        if (scene) {
            for (auto* c : scene->getChildren()) {
                out << "  scene child " << typeid(*c).name() << " tag " << c->getTag()
                    << " pos " << c->getPosition().x << "," << c->getPosition().y << "\n";
                if (auto* m = dynamic_cast<Menu*>(c->getChildByTag(kBaseMenuItemTag))) {
                    for (auto* it : m->getChildren()) {
                        out << "    menu item tag " << it->getTag() << " pos " << it->getPosition().x << "," << it->getPosition().y
                            << " size " << it->getBoundingBox().size.width << "x" << it->getBoundingBox().size.height << "\n";
                    }
                }
            }
        }
        out.close();
    } else if (cmd == "tree") {
        std::ofstream out("/tmp/pzqa/out", std::ios::app);
        std::function<void(Node*, int)> walk = [&](Node* n, int depth) {
            for (auto* c : n->getChildren()) {
                out << std::string(depth * 2, ' ') << (c->getName().empty() ? "?" : c->getName())
                    << " cls=" << typeid(*c).name() << " tag=" << c->getTag()
                    << " z=" << c->getLocalZOrder() << " vis=" << c->isVisible()
                    << " pos=" << c->getPosition().x << "," << c->getPosition().y << "\n";
                if (auto* it = dynamic_cast<MenuItemSprite*>(c)) {
                    out << std::string(depth * 2 + 2, ' ') << "  item tag=" << it->getTag()
                        << " pos=" << it->getPosition().x << "," << it->getPosition().y << "\n";
                }
                if (depth < 3) walk(c, depth + 1);
            }
        };
        if (scene) walk(scene, 0);
        out.close();
    } else if (cmd == "gadget") {
        PZProgress::shared()->toggleGadget(arg);
        AXLOGI("[QA] gadgets selected={}", PZProgress::shared()->selectedGadgetCount());
    } else if (cmd == "daily") {
        if (auto* m = findLayer<PZGMainMenuScene>(scene)) PZPopups::showDailyReward(m);
    } else if (cmd == "gadgets") {
        if (auto* m = findLayer<PZGMainMenuScene>(scene)) PZPopups::showGadgets(m);
    } else if (cmd == "shop") {
        if (auto* m = findLayer<PZGMainMenuScene>(scene)) PZPopups::showShop(m, nullptr, arg == "coins");
    } else if (cmd == "missions") {
        if (auto* m = findLayer<PZGMainMenuScene>(scene)) PZPopups::showMissions(m);
    } else if (cmd == "tap") {
        std::string a2; is >> a2;
        const Size win = Director::getInstance()->getWinSize();
        const float dx = std::atof(arg.c_str()) / 1400.f * win.width;
        const float dyGl = win.height - std::atof(a2.c_str()) / 644.f * win.height;
        auto* rv = Director::getInstance()->getRenderView();
        const float vx = dx * rv->getScaleX() + rv->getViewPortRect().origin.x;
        const float vy = (win.height - dyGl) * rv->getScaleY() + rv->getViewPortRect().origin.y;
        intptr_t id = 0; float xs[1] = {vx}, ys[1] = {vy};
        rv->handleTouchesBegin(1, &id, xs, ys);
        rv->handleTouchesEnd(1, &id, xs, ys);
    } else if (cmd == "back") {
        Director::getInstance()->popScene();
    }
}
}

void PZQA::install() {
    if (g_installed) return;
    g_installed = true;
    Director::getInstance()->getScheduler()->schedule([](float) {
        std::ifstream f("/tmp/pzqa/cmd");
        if (!f) return;
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(f, line)) if (!line.empty()) lines.push_back(line);
        f.close();
        std::remove("/tmp/pzqa/cmd");
        for (auto& l : lines) run(l);
    }, &g_key, 0.25f, false, "pzqa");
    Director::getInstance()->getScheduler()->schedule([](float dt) { botTick(dt); }, &g_bot, 0.f, false, "pzqabot");
}
