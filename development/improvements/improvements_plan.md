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
- Simulator build is current (built from this working tree); the QA channel is enabled until the final clean build.
- QA hook pattern: temp Source/Features/PZQA.cpp/.h polls /tmp/pzqa/cmd (play, die, restart, grant <magnet|shield|turbo|blast>, coins N, dist N, noads, bot, jump, tap x y, daily, missions, back); install $S/qa_on.sh, remove $S/qa_off.sh before shipping; re-run cmake after adding files.
- Menu start routes through PZGCharacterMenu when >1 characters exist (start button tag 3 → character select → play). No upgrade scene; GUI_UpgradeButton is remove-ads IAP; PZGCoinShopScene is coin IAP.
- Device-only build-ios-device must not be touched by sim work; BUILD_COMMANDS.md §3.1/3.2 has archive steps.
- Do not commit unless the user explicitly asks; user has not asked this session (prior commits 7b880a7 improvements 2, 3e8b21f improvements 1 exist).
- Game over now: GAME OVER art, BEST/NEW BEST line, one stats panel (distance/coins/best + LEVEL/XP + DOUBLE COINS), original BACK/RESTART art moved to safe.minY+46. Revive offers coins + video; the video button polls readiness (shows VIDEO LOADING... until ready).
- Menu column (implemented): LV/BEST plate 224x50 + four stacked 224x32 pills (DAILY/MISSIONS/GADGETS/FREE COINS, 5px gaps) in the top-right, ending ~36 design units above the START art; vibration chip bottom-right. Badges sit inside the pill's right end.
- HUD plan: power-up chips centred at safe.maxY - 126, combo moved to safe.maxY - 162, mission tracker at (safe.maxX - 130, safe.maxY - 150), gadget strip at (safe.minX + 24, safe.maxY - 122).
- Gadget pool (plist-overridable): magnet lv1 8s, shield lv3 12s, headstart lv5 150m, helmet lv7 1 save; first install auto-equips first unlocked gadget (magnet).
- Jetpack design: duration 6s, height 250*kDeviceScale above ground, coin every 0.30s, destroys obstacles in path, cyan aura/flame, safe assisted landing + 1.2s grace.
- Big Air: airtime >= 0.85s pays +5 coins with burst/ring/haptic (Tick exists).
Work State
Completed
- Improves 1-3 committed (7a2228e): config hub + plist loader, progress/XP/levels/streak/missions/haptics, UI kit restyle,
  run director (stages, combos, near-miss, Magnet/Shield/Turbo/Blast, FX, revive/mercy, results), popups (daily/missions/
  revive/results/menu widgets), PZLayout safe-area system, Android haptics bridge, obstacle fairness.
- Improves 3 layer verified in the iOS Simulator this session: jetpack (lifts to the flight line, gravity 0, smashes
  obstacles, coin ribbon, cyan aura + "JETPACK" banner), gadgets popup (2x2 cards, lock states, equip/persist, menu
  badge count, in-run strip, magnet+shield applied at run start), big-air/bounty/mission hooks, revive popup with
  coin path (100 coins deducted, run resumes with turbo), results panel with XP bar and DOUBLE COINS button.
- Menu meta column rebuilt: level plate + stacked full-width DAILY / MISSIONS / GADGETS / FREE COINS pills, sized
  from PZLayout::safeRect() so the column always clears the big START button; notification badges moved inside the
  pill's right end so they never cover the label.
- Popup z-order fixed: all modals use kZModal (1500) so they draw above the in-run HUD (z ~1000) and the menu
  widgets (z 450). Previously the power-up chips and mission tracker showed through the revive panel.
- Free-coins feedback made visible: its banner/burst used z 30 on the menu layer (behind the background art) -> now
  z 1200. PZUI::banner also gained a navy backing plate so banner text stays readable over gameplay/menu art.
- Game-over overlay hardened: a second gameOverCallBack(true) now reuses the existing overlay instead of adding a
  second one; the replacement used to lose the results panel (presentResults commits only once) and duplicate the
  BACK/RESTART art buttons.
- Results panel spacing: XP bar fills the row up to the DOUBLE COINS button, which is inset clear of the panel frame.
- docs/IMPROVEMENTS_PLAN.md phases 1-4 checked off with per-item notes; docs/META_FEATURES.md rewritten to document
  the metaFeatures override layer, the new gadget/jetpack/reward features, the code map and the z-order rules.
Active
- Nothing in progress; working tree contains only this session's fixes (menu column, badge inset, modal z-order,
  banner plate, game-over overlay reuse, results spacing) plus the temp QA channel in Source/Features/PZQA.cpp/.h.
Blocked
- Real AdMob rewarded unit IDs are still empty in Content/ads.plist, so the rewarded *completion* path (revive video,
  double coins, free coins) cannot be validated end to end; the loading/ready UI and coin fallback are verified.
- The simulator window's mouse mapping drifts, so UI taps are driven through the temporary QA channel
  (`echo "tap 1174 204" > /tmp/pzqa/cmd`) instead of cliclick.
Next Move
1. Remove the QA channel (scratchpad $S/qa_off.sh), rebuild, and re-run the smoke path (menu -> run -> die -> revive
   -> results -> restart) on the clean build.
2. When real ad unit IDs land: fill Content/ads.plist rewarded IDs, then verify video revive, double coins and free
   coins against live fill on device.
3. Optional remaining audit from the plan: obstacle-placement/fairness pass, pause-freezes-jetpack check, iPad/narrow
   aspect screenshot pass, and a quick perf sweep of the update loop.

Relevant Files
- docs/IMPROVEMENTS_PLAN.md: new Jetpack-Joyride-inspired roadmap; update checkboxes as phases land.
- Source/Features/PZGameConfig.h: all defaults incl. new jetpack/gadget/reward keys, GadgetTemplate/kGadgetPool, strings/colors.
- Source/Features/PZConfig.cpp: metaFeatures loader; new rewards + gadgets.Pool parsing.
- Source/Features/PZProgress.h/.cpp: gadget selection + free-coins cooldown persistence; awardCoins.
- Source/Features/PZRunDirector.h/.cpp: jetpack, gadgets, big air, start grace, revive gating, HUD, results — main integration point.
- Source/Features/PZPopups.h/.cpp: revive polling, gadgets/free coins/double coins UI, menu column, results layout (modals use kZModal).
- Source/PZGGameFieldScene.cpp: gameOverCallBack reuses an existing game-over overlay (tag 10) instead of stacking a second one.
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


NEW IMPLEMENTATIONS:

The issues and improvements
1. When player falls down form platform with powerups sometimes he stays down until the powerup expires and he dies
2. The coins starts hiding after 500 meters, they should not hide just check their entry and exit or may me some other issue but we need to fix it
3. The coin counter text next to Coin icon in left top side of the screen sometimes keeps increasing it’s size, it should not be like that it should animate with glitters or some animation off effect and then stays back to it’s original positions, it should not be too big
4. Find powerups icons properly and show text in the right side of the current powerup running 
5. Find and fix similar issues
6. Gadgets cards are not in the frame of the gadgets fix all similar alignments and positions
7. Improve the level bar on Home Screen
8. Improve revive system, the player should wait for next platform and then respawn and better logic is needed here, find how games respawn and use the best logic
9. Add rocket bike power up, so you have to add rocket to the side of the bike and it’s shield + magnet + speed with special effects of speeding
10. Add gun in front of the bike power up that shoots on all obstacles, and similar power ups
10.1. When player picks up destory all obstacles the game should stop and do the animations step by step, like zoom on obstacles and destroy it, once all are destroyed zoom will reset and game start playing again (we need to do such use cases more, you can add use cases sections for all these use cases that makes the game more interesting)
11. Add zoom effects, move in, move out, glitter effects and more what we can
12. Make gift system better, more offers 
13. Add local notifications system with messages and gift rewards, the push notifications permission should be asked when it’s better to do, not in the start of the game but when it feels like user like the game, after a few replays, levels or play time, find the best solutions, first show popup for reward reminder then the os permission popup, keep asking every once a time with different message on the reminder popup, add as much as possible local push notifications messages and send with standard time differences
14. Do similar things as above and let’s make this game great