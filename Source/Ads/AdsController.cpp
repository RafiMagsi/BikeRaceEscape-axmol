#include "Ads/AdsController.h"

#include "Ads/Providers/AdsProviderMux.h"
#include "PZLegacyCompat.h"

namespace PZ {

AdsController* AdsController::shared() {
    static AdsController instance;
    return &instance;
}

void AdsController::setProvider(std::unique_ptr<IAdsProvider> provider) {
    provider_ = std::move(provider);
    initialized_ = false;
}

void AdsController::setAdsEnabled(bool enabled) {
    adsEnabled_ = enabled;
    if (!adsEnabled_) {
        hideBanner();
    }
}

bool AdsController::initialize(const std::string& appIdOrKey) {
    appIdOrKey_ = appIdOrKey;
    if (!provider_) {
        AXLOGI("AdsController: no provider set; ads disabled by default");
        initialized_ = false;
        return false;
    }
    if (initialized_) return true;
    initialized_ = provider_->initialize(appIdOrKey_);
    AXLOGI("AdsController: initialize provider='{}' result={}", provider_->name(), initialized_);
    return initialized_;
}

void AdsController::loadConfigFromPlist() {
    config_ = AdsConfig::loadFromAdsPlist();

    // Choose provider based on config (for now: AdMob only).
    if (config_.admobEnabled) {
        // Provider must be set by the app bootstrap (keeps SDK optional).
        AXLOGI("AdsController: ads.plist enables AdMob (provider must be set by bootstrap)");
    }
    if (config_.chartboost.enabled) {
        AXLOGI("AdsController: ads.plist enables Chartboost (provider must be set by bootstrap)");
    }
}

void AdsController::onMainMenu() { handleContext(AdsContext::MainMenu); }
void AdsController::onLevelsMenu() { handleContext(AdsContext::LevelsMenu); }
void AdsController::onPauseMenu() { handleContext(AdsContext::PauseMenu); }
void AdsController::onPlaying() { handleContext(AdsContext::Playing); }
void AdsController::onGameOver() { handleContext(AdsContext::GameOver); }
// Interstitial networks that are enabled and configured on this platform, in the order the
// "free game" button prefers them (Chartboost first on iOS: fast full-screen).
std::vector<AdsController::InterstitialNetwork> AdsController::interstitialNetworks() const {
    std::vector<InterstitialNetwork> nets;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if (config_.chartboost.enabled && !config_.chartboost.interstitialLocationIOS.empty()) {
        nets.push_back({false, config_.chartboost.interstitialLocationIOS});
    }
    if (config_.admobEnabled && !config_.admob.interstitialIdIOS.empty()) {
        nets.push_back({true, config_.admob.interstitialIdIOS});
    }
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    if (config_.admobEnabled && !config_.admob.interstitialIdAndroid.empty()) {
        nets.push_back({true, config_.admob.interstitialIdAndroid});
    }
#endif
    return nets;
}

// Shows the first network that has an interstitial cached. Returns true if one was shown.
bool AdsController::showFirstReadyInterstitial() {
    for (const auto& net : interstitialNetworks()) {
        selectAdMobInterstitial(net.admob);
        if (!provider_->isInterstitialReady()) continue;

        AXLOGI("AdsController: showing {} interstitial", net.admob ? "AdMob" : "Chartboost");
        markFullscreenShown();
        showInterstitial();
        nextInterstitialAdMob_ = !net.admob; // let the next automatic ad use the other network
        loadInterstitial({net.placementId}); // cache the next one
        return true;
    }
    return false;
}

void AdsController::cancelMoreGames() {
    if (!moreGamesPending_) return;
    moreGamesPending_ = false;
    ax::Director::getInstance()->getScheduler()->unschedule("pz_more_games_poll", this);
}

void AdsController::onMoreGames() {
    if (!adsEnabled_ || !provider_) return;

    // Explicit button tap: show whichever network already has an ad cached.
    if (showFirstReadyInterstitial()) {
        cancelMoreGames();
        return;
    }
    if (moreGamesPending_) return; // already waiting for an ad to fill

    // Nothing is cached yet (e.g. right after launch): request every enabled network and show the
    // first one that fills. Give up after a few seconds so a late ad never pops up mid-game.
    const auto nets = interstitialNetworks();
    if (nets.empty()) {
        AXLOGW("AdsController: onMoreGames: no interstitial network is enabled/configured");
        return;
    }
    for (const auto& net : nets) {
        selectAdMobInterstitial(net.admob);
        if (!provider_->isInterstitialReady()) loadInterstitial({net.placementId});
    }
    AXLOGI("AdsController: onMoreGames: no ad cached yet; waiting for one to load");

    static constexpr float kPollInterval = 0.25f;
    static constexpr float kTimeout = 10.0f;
    moreGamesPending_ = true;
    moreGamesElapsed_ = 0.0f;
    ax::Director::getInstance()->getScheduler()->schedule(
        [this](float dt) {
            moreGamesElapsed_ += dt;
            if (!adsEnabled_ || !provider_) {
                cancelMoreGames();
            } else if (showFirstReadyInterstitial()) {
                cancelMoreGames();
            } else if (moreGamesElapsed_ >= kTimeout) {
                AXLOGW("AdsController: onMoreGames: no ad filled within {}s; giving up", kTimeout);
                cancelMoreGames();
            }
        },
        this, kPollInterval, false, "pz_more_games_poll");
}

void AdsController::showBanner() {
    if (!adsEnabled_ || !provider_) return;
    provider_->showBanner();
}

void AdsController::loadBanner(const AdRequest& request) {
    if (!adsEnabled_ || !provider_) return;
    provider_->loadBanner(request);
}

void AdsController::hideBanner() {
    if (!provider_) return;
    provider_->hideBanner();
}

void AdsController::loadInterstitial(const AdRequest& request) {
    if (!adsEnabled_ || !provider_) return;
    provider_->loadInterstitial(request);
}

void AdsController::showInterstitial() {
    if (!adsEnabled_ || !provider_) return;
    provider_->showInterstitial();
}

void AdsController::loadRewarded(const AdRequest& request) {
    if (!adsEnabled_ || !provider_) return;
    provider_->loadRewarded(request);
}

void AdsController::showRewarded() {
    if (!adsEnabled_ || !provider_) return;
    provider_->showRewarded();
}

namespace {
std::string rewardedPlacementId(const AdsConfig& cfg) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    std::string id = cfg.admob.rewardedIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    std::string id = cfg.admob.rewardedIdAndroid;
#else
    std::string id;
#endif
#ifndef NDEBUG
    // Debug builds only: fall back to Google's public sample rewarded unit so the flow can be exercised before a
    // real unit exists. Release builds with an empty id simply have no rewarded video (the option is hidden).
    if (id.empty()) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        id = "ca-app-pub-3940256099942544/1712485313";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        id = "ca-app-pub-3940256099942544/5224354917";
#endif
    }
#endif
    return id;
}
} // namespace

void AdsController::requestRewarded() {
    if (!provider_ || !initialized_) return;
    const std::string id = rewardedPlacementId(config_);
    if (id.empty()) return;
    provider_->loadRewarded({id});
}

bool AdsController::isRewardedAvailable() const {
    return provider_ && initialized_ && provider_->isRewardedReady();
}

namespace {
double nowSeconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}
} // namespace

bool AdsController::fullscreenAllowed() const { return nowSeconds() - lastFullscreenAt_ >= minGapSeconds_; }
void AdsController::markFullscreenShown() { lastFullscreenAt_ = nowSeconds(); }

void AdsController::cancelPendingInterstitial() {
    ax::Director::getInstance()->getScheduler()->unschedule("pz_delayed_interstitial", this);
}

void AdsController::showRewardedWithCallback(std::function<void(bool)> callback) {
    if (!isRewardedAvailable()) {
        if (callback) callback(false);
        requestRewarded();
        return;
    }
    rewardedCallback_ = std::move(callback);
    cancelPendingInterstitial();
    markFullscreenShown();
    provider_->showRewarded();
}

void AdsController::handleRewardedFinished(bool earned) {
    auto cb = std::move(rewardedCallback_);
    rewardedCallback_ = nullptr;
    requestRewarded();     // cache the next one
    if (cb) cb(earned);
}

void AdsController::selectAdMobInterstitial(bool admob) {
    if (auto* mux = dynamic_cast<AdsProviderMux*>(provider_.get())) {
        mux->useBannerProviderForInterstitial(admob);
    }
}

void AdsController::handleContext(AdsContext ctx) {
    const int i = static_cast<int>(ctx);
    if (!adsEnabled_ || !provider_) return;

    // A pending "free game" ad must not appear after the player has left the main menu.
    if (ctx != AdsContext::MainMenu) cancelMoreGames();

    const auto& s = config_.setups[i];

    // Banner (AdMob): shown on menu contexts, hidden while playing.
    {
        std::string bannerId;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        bannerId = config_.admob.bannerIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        bannerId = config_.admob.bannerIdAndroid;
#endif
        if (config_.admobEnabled && s.bannerAdMobWeight > 0 && !bannerId.empty()) {
            bannerCounters_[i] += 1;
            if (bannerCounters_[i] >= s.bannerShowAfterCount) {
                loadBanner({bannerId});
                showBanner();
            }
        } else {
            hideBanner();
        }
    }

    // Interstitial schedule: alternates AdMob / Chartboost when both are weighted.
    // Show only when cached (showing before ready causes a white loading screen on Chartboost),
    // then cache the next one.
    const bool admobInterstitialOn = config_.admobEnabled && s.interstitialAdMobWeight > 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    const bool chartboostInterstitialOn = config_.chartboost.enabled && s.interstitialChartboostWeight > 0;
#else
    const bool chartboostInterstitialOn = false; // Chartboost is only wired on iOS.
#endif
    if ((admobInterstitialOn || chartboostInterstitialOn) && s.interstitialShowAfterCount > 0) {
        interstitialCounters_[i] += 1;
        if (interstitialCounters_[i] >= s.interstitialShowAfterCount) {
            interstitialCounters_[i] = 0;

            bool useAdMob = admobInterstitialOn && (!chartboostInterstitialOn || nextInterstitialAdMob_);
            auto requestFor = [&](bool admob) {
                AdRequest r;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
                r.placementId = admob ? config_.admob.interstitialIdIOS : config_.chartboost.interstitialLocationIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
                r.placementId = admob ? config_.admob.interstitialIdAndroid : std::string();
#endif
                return r;
            };

            // Prefer the scheduled network; fall back to the other one if it is not cached.
            selectAdMobInterstitial(useAdMob);
            bool ready = provider_->isInterstitialReady();
            if (!ready && admobInterstitialOn && chartboostInterstitialOn) {
                selectAdMobInterstitial(!useAdMob);
                if (provider_->isInterstitialReady()) {
                    useAdMob = !useAdMob;
                    ready = true;
                } else {
                    selectAdMobInterstitial(useAdMob);
                }
            }
            const AdRequest req = requestFor(useAdMob);
            AXLOGI("AdsController: interstitial tick (ctx={}, network={}, ready={})", i, useAdMob ? "AdMob" : "Chartboost", ready);
            // Pacing: grace period for new players, minimum gap, and a short delay at game over so the ad can never land
            // on a tap that was meant for the game-over buttons.
            const bool atBreak = ctx == AdsContext::GameOver;
            if (atBreak) ++breaksSeen_;
            if (ready && (!atBreak || breaksSeen_ > graceBreaks_) && fullscreenAllowed()) {
                nextInterstitialAdMob_ = !useAdMob;
                const bool admobPick = useAdMob;
                auto fire = [this, admobPick](float) {
                    if (!adsEnabled_ || !provider_) return;
                    selectAdMobInterstitial(admobPick);
                    if (!provider_->isInterstitialReady() || !fullscreenAllowed()) return;
                    markFullscreenShown();
                    showInterstitial();
                };
                if (atBreak && delaySeconds_ > 0.f) {
                    ax::Director::getInstance()->getScheduler()->schedule(fire, this, 0.f, 0, delaySeconds_, false,
                                                                          "pz_delayed_interstitial");
                } else {
                    fire(0.f);
                }
            }
            if (!req.placementId.empty()) loadInterstitial(req); // cache the next one
        }
    }

    // Proactive caching on Main Menu so GameOver can show instantly.
    if (ctx == AdsContext::MainMenu) {
        for (const bool admob : {true, false}) {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
            if (!admob) continue;
#endif
            if (admob ? !config_.admobEnabled : !config_.chartboost.enabled) continue;
            AdRequest r;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
            r.placementId = admob ? config_.admob.interstitialIdIOS : config_.chartboost.interstitialLocationIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            r.placementId = admob ? config_.admob.interstitialIdAndroid : std::string();
#endif
            if (r.placementId.empty()) continue;
            selectAdMobInterstitial(admob);
            if (!provider_->isInterstitialReady()) loadInterstitial(r);
        }
    }

    // Rewarded schedule is supported by config, but should generally be triggered by explicit UI events.
    // We keep this for compatibility with older configs that auto-show rewarded at certain contexts.
    if (s.rewardedAdMobWeight > 0 && s.rewardedShowAfterCount > 0) {
        rewardedCounters_[i] += 1;
        if (rewardedCounters_[i] >= s.rewardedShowAfterCount) {
            AdRequest req;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
            req.placementId = config_.admob.rewardedIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            req.placementId = config_.admob.rewardedIdAndroid;
#endif
            if (!req.placementId.empty()) {
                loadRewarded(req);
                showRewarded();
            }
            rewardedCounters_[i] = 0;
        }
    }
}

} // namespace PZ
