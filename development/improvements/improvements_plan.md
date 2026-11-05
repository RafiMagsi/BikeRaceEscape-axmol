Objective
- Polish the Dino Bike Escape (repo BikeRaceEscape-axmol) meta layer like a professional endless runner, using Jetpack Joyride as inspiration: HUD/power-up alignment above the ad banner, revive via coins OR rewarded video, free/double coins, gadget loadout, a Jetpack power-up, big-air bonuses, more FX, all tunables in Content/CompleteGameInfo.plist, a plan MD in docs/, and iOS Simulator verification — while keeping the project a clean reskin base.
Important Details
- Repo: /Users/rafi/developer/games/remake/BikeRaceEscape-axmol; bundle com.nextfiction.dinobikeescape; Apple team XF5237ER2B; iOS min 15.0; Android minSdk 24; iOS build 9 / 1.3.0, Android versionCode 5 / 1.3.0.
- Engine: own thirdparty/axmol for iOS; Android uses shared ~/development/axmol (media3 pinned 1.2.1). Design res 960x640, FIXED_HEIGHT; on a 19.5:9 device actual getWinSize() is ~1392x640 — use PZLayout::safeRect() and never assume 960 width.
- axmol Director::getSafeAreaRect() returns RenderView::getVisibleRect() (visible rect, not true safe area); PZLayout adds its own margins/banner reserve.
- Config system: metaFeatures at bottom of Content/CompleteGameInfo.plist (~line 7685); PZConfig.cpp maps key = C++ var name minus leading k; missing keys keep defaults in PZGameConfig.h. Sections now: theme/strings/features/tuning/daily/missions/art/rewards/gadgets. Missing-key safety must stay for reskins.
- Rewarded API in Source/Ads/AdsController.h: requestRewarded(), isRewardedAvailable(), showRewardedWithCallback(std::function<void(bool earned)>), handleRewardedFinished(bool). Content/ads.plist has rewardedIdIOS/rewardedIdAndroid as empty strings; AdMob enabled/testMode false; debug fallback sample unit exists. User will supply real IDs later; release with empty id hides option.
- Ads.plist banner configs exist per screen; OnPlaying banner off. PZLayout::bannerReserve() returns 0 if removeAds.
- Physics scale: kPhysicsWorldScale 0.05 iOS vs 0.1 Android (Android jumps half as high in pixels); kDeviceScale() used for spacing/jetpack height.
- Haptics: PZHaptics::Kind {Tick, Light, Medium, Heavy, Success, Warning, Error, Selection}; respects PZProgress::hapticsEnabled().
- UI style must match original: navy panels PZCfg::kColPanel, bronze rings, orange pill buttons with white outline/gloss, PressStart2P (PZCfg::kFontPath).
- Simulator workflow (outside repo): $S=/private/tmp/claude-501/-Users-rafi-developer-games-remake-BikeRaceEscape-axmol/e3eb76ea-655c-414a-801f-6474ebefe9a1/scratchpad; $S/build-sim, $S/build.sh, $S/run.sh [fresh], $S/shot.sh, $S/tap.sh x y (screenshot coords 1400x644; simulator window 785x453, mapping verified); device iPhone 17 Pro Max iOS 26.4 AAAA9D66-DF85-4D0A-9B85-964CED24AF93.
- Installed simulator build is STALE (app binary Nov 5 17:01; source 17:22/17:29) — it predates the vibe button/current UI. Rebuild before judging layout.
- QA hook pattern: temp Source/Features/PZQA.cpp/.h polls /tmp/pzqa/cmd (play, die, restart, grant <magnet|shield|turbo|blast>, coins N, dist N, noads, bot, jump, tap x y, daily, missions, back); install $S/qa_on.sh, remove $S/qa_off.sh before shipping; re-run cmake after adding files.
- Menu start routes through PZGCharacterMenu when >1 characters exist (start button tag 3 → character select → play). No upgrade scene; GUI_UpgradeButton is remove-ads IAP; PZGCoinShopScene is coin IAP.
- Device-only build-ios-device must not be touched by sim work; BUILD_COMMANDS.md §3.1/3.2 has archive steps.
- Do not commit unless the user explicitly asks; user has not asked this session (prior commits 7b880a7 improvements 2, 3e8b21f improvements 1 exist).
- Current game-over screenshot findings: GAME OVER art top, BEST 306 under it (~y298px), COINS +0 left (x375), BACK/RESTART round buttons x553/831 y408, LEVEL/XP right (x1040); layout needs a proper stats panel + double-coins control; revive currently shows coin-only because ad not loaded at death.
- Current menu widgets (stale build): LV/BEST plate, DAILY, MISSIONS at top-right; planned compact column = plate 210x66 + four 210x40 pills (DAILY/MISSIONS/GADGETS/FREE COINS) with 10px gaps, vibration as small bottom-right chip to avoid the big START button.
- HUD plan: power-up chips centred at safe.maxY - 126, combo moved to safe.maxY - 162, mission tracker at (safe.maxX - 130, safe.maxY - 150), gadget strip at (safe.minX + 24, safe.maxY - 122).
- Gadget pool (plist-overridable): magnet lv1 8s, shield lv3 12s, headstart lv5 150m, helmet lv7 1 save; first install auto-equips first unlocked gadget (magnet).
- Jetpack design: duration 6s, height 250*kDeviceScale above ground, coin every 0.30s, destroys obstacles in path, cyan aura/flame, safe assisted landing + 1.2s grace.
- Big Air: airtime >= 0.85s pays +5 coins with burst/ring/haptic (Tick exists).
Work State
Completed
- Prior committed feature layer: config hub + plist loader, progress (best/XP/level/streak/missions/haptics), UI kit restyle, run director (stages, combos, near-miss, Magnet/Shield/Turbo/Blast + obstacle destruction + FX, revive/mercy, results, haptics), popups (daily/missions/revive/results/menu widgets), layout safe-area system, Android haptics bridge, obstacle fairness in PZGGamefieldPlatforms::finalizeChunk, guide coin arcs, menu plist Y remap above banner; simulator + Android native build verified earlier.
- Created docs/IMPROVEMENTS_PLAN.md — Jetpack-Joyride-inspired plan with phases 0–6, design pillars, plist key list, acceptance criteria. Phase 0 checked; later phases unchecked.
- Recon: read PZGGameFieldScene HUD/coins/gameOverCallBack, PZGGameOverScene, PZRunDirector HUD/powerups/death/results, PZPopups revive/results/menu widgets, PZLayout, PZConfig, PZProgress, AdsController, ads.plist, CompleteGameInfo.metaFeatures; confirmed getSafeAreaRect = visible rect; confirmed stale build; confirmed ad-not-loaded cause of coin-only revive.
- Source/Features/PZGameConfig.h extended:
- theme kColJetpack {120,235,255}.
- strings for jetpack, big air, start hint, head start, lucky helmet, revive sub, video no-ad/retry, free/double coins, gadgets, results distance.
- features EnableGadgets, EnableJetpack, EnableFreeCoins, EnableDoubleCoins, EnableBigAir, EnableStartGrace.
- tuning kJetpackDuration 6.0, kJetpackHeight 250, kJetpackCoinEvery 0.30, kJetpackCoinsPerTick 1, kWeightJetpack 0.08 (Magnet .26/Shield .26/Turbo .20/Blast .20), kStartGrace 1.2, kBigAirMin 0.85, kBigAirCoins 5, kGadgetSlots 2, kGadgetMagnetSeconds 8, kGadgetShieldSeconds 12, kGadgetHeadStartM 150, kGadgetHelmetSaves 1, kFreeCoinsAmount 100, kFreeCoinsCooldown 300, kDoubleCoinsMultiplier 2.0; kArtOrbJetpack.
- GadgetTemplate {id,name,desc,icon,unlockLevel,value} + kGadgetPool.
- Source/Features/PZConfig.cpp parses all new theme/strings/features/tuning/art keys, rewards (FreeCoinsAmount/FreeCoinsCooldown/DoubleCoinsMultiplier), gadgets.Pool (Id/Name/Desc/Icon/UnlockLevel/Value).
- Source/Features/PZProgress.h/.cpp extended: gadget API (gadgetSlotCount, gadgetPoolCount, gadgetPoolIndex, gadgetAt, gadgetUnlocked, gadgetSelected, selectedGadgetCount, toggleGadget), rewarded economy (freeCoinsAvailable, freeCoinsSecondsLeft, claimFreeCoins, awardCoins); persistence keys PZP_Gadget%d, PZP_GadgetInit, PZP_FreeCoinsAt; first-install auto-equip magnet; save/load updated.
- Source/Features/PZRunDirector.h/.cpp extended (not yet compiled):
- PowerUp::Jetpack = 4, coinsEarned(), refreshGadgetStrip(), updateJetpackRide(), applyGadgets(), endJetpack(); _rows[5], _gadgetStrip, _jetpackAura, _jetpackT/_jetpackMax/_jetpackLockY/_jetpackCoinClock, _helmetSaves.
- initDirector 5 HUD rows, jetpack aura, gadget strip; layoutHud shifts combo/tracker/gadget strip; onLoaded refresh strip.
- startRun resets jetpack/helmet, start grace, applies gadgets, GO banner + "TAP TO JUMP" for best<=0, Selection haptic.
- Jetpack collect/lift, hover with bob, obstacle destruction, flame bursts/afterimages, coin ticks, safe endJetpack() landing; turbo pickups ignored while flying; spawn suppressed during jetpack; updated weights/HUD arrays.
- Lucky Helmet save before mercy in onDeath; revive video now offered whenever kEnableReviveVideo and calls requestRewarded() (popup must poll readiness).
- Big Air bonus on landing; jump/landing/dust FX skipped during jetpack; presentResults hides jetpack aura/gadget strip.
- Old screenshots (p*.png etc.) are unrelated; use new captures.
Active
- PZPopups not yet modified: still old showRevive(..., bool videoAvailable, ...) behavior, no ad polling/loading state, no DOUBLE COINS button, no gadgets popup, no FREE COINS button, menu column not rebuilt. Need AdsController include.
- attachResults signature still (gameOverLayer, distanceLabel, stats, result); planned extension with std::function<int()> claimDouble (grant via PZRunDirector/scene->addBonusCoins) and layout rework (stats panel, reposition original BACK/RESTART above banner, DOUBLE COINS pill).
- PZRunDirector::presentResults not yet wired for double-coins callback; verify jetpack/pause/game-over interactions.
- Content/CompleteGameInfo.plist metaFeatures not yet updated with new features/tuning/rewards/gadgets/strings/art/theme values.
- No build yet after edits; compile status unknown (watch for missing includes e.g. <algorithm> for std::max, PZLayout include in PZRunDirector, PZProgress string/vector headers).
- Uncommitted working-tree changes: plan doc + all code edits above.
Blocked
- Real AdMob rewarded unit IDs not provided (ads.plist rewarded IDs empty); full video flow cannot be validated with real fill — use debug sample unit and polling UI; user supplies IDs later.
- Simulator ad availability may be unreliable/slow, so video revive button must remain usable with loading state and coin fallback.
Next Move
1. Rewrite the revive popup to accept videoSupported (config) and poll PZ::AdsController::isRewardedAvailable() every ~0.35s, enabling WATCH VIDEO from VIDEO LOADING...; add subtitle/SAVE ME styling and keep coin/skip options.
2. Implement PZPopups: gadgets popup (2x2 data-driven cards, equip up to 2, lock by level, persist + refresh menu), rebuilt compact menu column with GADGETS/FREE COINS/vibration chip, FREE COINS rewarded flow with cooldown toast, DOUBLE COINS results button; extend attachResults + wire callback in PZRunDirector::presentResults.
3. Add all new metaFeatures sections/values to Content/CompleteGameInfo.plist; rebuild simulator ($S/build.sh / cmake), fix compile errors, then QA flows with qa_on.sh (menu, gadgets, free coins, run, jetpack, big air, die/revive video+coins, results, double coins, restart); screenshot-check HUD/game over; update docs/META_FEATURES.md + plan status; run qa_off.sh before finishing.
Relevant Files
- docs/IMPROVEMENTS_PLAN.md: new Jetpack-Joyride-inspired roadmap; update checkboxes as phases land.
- Source/Features/PZGameConfig.h: all defaults incl. new jetpack/gadget/reward keys, GadgetTemplate/kGadgetPool, strings/colors.
- Source/Features/PZConfig.cpp: metaFeatures loader; new rewards + gadgets.Pool parsing.
- Source/Features/PZProgress.h/.cpp: gadget selection + free-coins cooldown persistence; awardCoins.
- Source/Features/PZRunDirector.h/.cpp: jetpack, gadgets, big air, start grace, revive gating, HUD, results — main integration point; not yet compiled.
- Source/Features/PZPopups.h/.cpp: revive polling, gadgets/free coins/double coins UI, menu column, results layout — next major edit.
- Source/Features/PZUI.h/.cpp: panel/button/heading/bar/coin icon kit; Button::rebuild style.
- Source/Features/PZLayout.h/.cpp: safeRect()/bannerReserve()/mapY()/keepInside()/popupCentre()/popupMaxHeight().
- Content/CompleteGameInfo.plist: metaFeatures (~line 7685) must receive new values/sections.
- Content/ads.plist: rewarded IDs empty; banner/interstitial config exists; placeholders to fill later.
- Source/Ads/AdsController.h/.cpp: rewarded readiness/request/show API for revive/free/double coins.
- Source/PZGGameFieldScene.h/.cpp: addBonusCoins, gameOverCallBack (revive intercept), coinPickup, HUD labels; addBonusCoins updates settings + gf_coins.
- Source/PZGMainMenuScene.cpp: attachMenuWidgets/refreshMenuWidgets attach points; start → character select.
- Source/PZGGameOverScene.cpp: overlay load/score label/back/restart tags; results layout host.
- Source/PZGGamefieldPlatforms.h/.cpp: finalizeChunk, groundYAt, alignForRevive (used by endJetpack), riderApex.
- Source/PZGGamefieldBasicRunner.h/.cpp: creates run director, speed multiplier.
- Source/constants.h: kDeviceScale(), UI tags.
- docs/META_FEATURES.md: reskin/feature map — update with new keys/features.
- BUILD_COMMANDS.md: iOS archive/release steps; version table.
- Scratchpad $S/build.sh, run.sh, shot.sh, tap.sh, qa_on.sh, qa_off.sh, qa/PZQA.cpp: simulator/QA harness.