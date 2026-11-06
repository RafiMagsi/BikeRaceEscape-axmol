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
10.1. When player picks up destory all obstacles power-up, the game should stop and do the animations step by step, like zoom on obstacles and destroy it, once all are destroyed zoom will reset and game start playing again (we need to do such use cases more, you can add use cases sections for all these use cases that makes the game more interesting) (mixing with existing blast wave)
11. Add zoom effects, move in, move out, glitter effects and more what we can
12. Make gift system better, more offers 
13. Add local notifications system with messages and gift rewards, the push notifications permission should be asked when it’s better to do, not in the start of the game but when it feels like user like the game, after a few replays, levels or play time, find the best solutions, first show popup for reward reminder then the os permission popup, keep asking every once a time with different message on the reminder popup, add as much as possible local push notifications messages and send with standard time differences
14. Do similar things as above and let’s make this game great

STATUS OF "NEW IMPLEMENTATIONS" (latest pass)
1. Falls with power-ups: FIXED. Powered rides take their flight line from the nearest platform surface (search ahead/behind +
   last known ground) and end themselves if the rider drops out of the line; shield/mercy rescue unchanged.
2. Coins hiding after ~500 m: FIXED (real cause: 32-slot coin/hazard arrays evicting on-screen objects as chunks and guide arcs
   added 40+ at once). Capacity raised, free-slot registration, explicit off-screen cull.
3. Coin counter growth: FIXED (absolute scale targets) + glitter on arrival.
4. Power-up chips: DONE (icon, name, seconds left on the right, timer bar; six chip slots in one canonical order).
5. Similar issues found and fixed: HUD chip/icon mismatch crash (rows sized 7, created 5), GadgetCard origin offset,
   hit-stop never honoured by the runner, jump apex never fed to the generator, jump buffering missing.
6. Gadget cards alignment: FIXED (card content is centred on the node origin; the anchor/hit-test were offset by half a card).
7. Home level bar: REDESIGNED (round level badge, XP counter, framed animated bar, best, next unlock).
8. Revive: hover until a platform arrives, land, 3-2-1 GET READY countdown with the world held, then turbo sprint + grace.
9. Rocket bike: DONE (side pod, shield + magnet + speed, flame/afterimages). 10. Gun: DONE (nose cannon, muzzle flash, recoil).
10.1 Cinematic Blast: DONE (freeze, HUD hides, zoom on each hazard, reticle, explosion, zoom out, shockwave for the rest).
11. Zoom/glitter FX: DONE (camera punch about the rider, cinematic zoom, glitter on pickups/stages/rewards).
12. Gifts: DONE (Gift Center hub, hourly gift, lucky spin, level chest, mission chest, welcome-back gift).
13. Local notifications: DONE (soft ask after engagement, OS prompt only on yes, rotating copy + re-ask pacing, schedule of
    gift/daily/missions/streak/comeback reminders). iOS verified in the simulator; Android compiled (not run on a device).
14. Obstacle placement: relaxed over-aggressive filtering and added a hazard budget (see META_FEATURES.md).
Notes: the Android jump is half as high in pixels as iOS (kPhysicsWorldScale 0.1 vs 0.05); placement now uses the real apex.
Real AdMob rewarded unit IDs are still needed for end-to-end video revive / spin / free coins in release.

## Testing Round 1 - NOTE# Do not update the tasks just add your feedback under the task number before the other task/point starts.
15. Always show revive in gameover make revive time 10 seonds
    > Feedback: DONE (simulator-verified). Game over always shows REVIVE (coins) + WATCH AD REVIVE side by side, once per run; picking either rolls the committed run back (PZProgress::snapshot) and resumes with hover -> 3-2-1 -> blinking grace. Countdown is 10 s (tuning/ReviveCountdown, also the CONTINUE? popup).
    > Fixes found on the way: death inside a physics step is deferred (Box2D SetTransform assertion), shield/helmet/second-chance rescue now prefers a platform with runway, revive no longer parks the rider under the deck (bad learned ground offset).
16. Improve all the menus with proper positions, some screens are too conjested, you can reduce the size of back,retry, button to standard sizes
    > Feedback: DONE (simulator-checked: main menu, character select, pause, game over, shop, gadgets). One standard round-button size for BACK / RETRY / CONTINUE on every screen (tuning/StdRoundButtonPx = 76, applied in PZGBaseMenuScene::load, replacing the old 1.0 / 1.2 / 1.25 / 0.9 mix). Game over is now one clean row: BACK - REVIVE - WATCH AD - RETRY, clear of the stats panel. The legacy COIN STORE screen (3 art packs) is reworked in task 24.
17. Improve all HUD displays, find a better game font if you can with good texture
    > Feedback: DONE. New font Lilita One (SIL OFL, Content/Fonts) everywhere via theme/FontPath + FontScale (1.45) + MinFontPx (14), so a reskin can swap the font without code. HUD/popups/pills/shop/game-over re-checked. The level plate and mission tracker text are still small because they live in fixed art-sized frames; say if you want those frames enlarged.
18. Again check all die/revive/powerup use cases/senarios
    > Feedback: DONE for this round (simulator, automated position log per power-up). Jetpack, rocket, gun, turbo, shield and magnet were each granted and the bike left to fall: the first fall is rescued onto a platform (shield or second chance), a second fall brings up CONTINUE? (10 s, coins or ad), declining gives the game-over screen with REVIVE/WATCH AD, and a game-over revive rolls the run back and resumes. Powered rides land safely on a platform. Not covered: real rewarded-ad completion (no live ad unit).
19. Add all powerups in the gadgets and put price on each, also show gadgets in the gameplay i asked this before, all gadgets should have icons according to their nature.
    > Feedback: DONE. All 8 power-ups are gadgets with a coin price and charges (buy 1; each equipped gadget uses 1 at run start), nature-specific code-drawn icons (PZUI::gadgetIcon), and an in-run icon strip (dim = out of charges / spent). Unlock levels still gate some gadgets. (Prices are re-tuned by task 29.)
20. Show ads as google admob ads policy, so there should be no rejection or policy voilations.
    > Feedback: DONE in code, NOT run against live ads. Google UMP consent on Android + iOS before any ad request; interstitial pacing (first 2 game overs free, 120 s gap, 1.4 s delay, every 2nd game over, cancelled on revive); misleading FREE GAME ribbon (opened a full-screen ad) removed by default (features/EnableFreeGameAd=false); rewarded ads only start from a visible player tap with the coin amount on the button.
    > Still needed from you: real rewarded unit IDs in Content/ads.plist, a Privacy Options button (AdsBridge.showPrivacyOptions exists, no UI yet), and the AdMob/Play Ads + Data-safety declarations.
21. Make a complete shop with powerups and coins. with watch ad get coinds button, put coins on the button (should follow admob policy)
    > Feedback: DONE. SHOP pill on the main menu: POWER-UPS tab (buy charges) and COINS tab (WATCH AD +N COINS with cooldown and an 'optional video' note, plus store packs from IAPSettings). Pack list grows to 6 in task 24.

## Testing Round 2
22.1 There are so many missing sounds, fix all effects sounds, like when blast power up is picked the hitting obstables there is no sound
    > Feedback: DONE (triggers verified in the simulator; not listened to on a device speaker, so tune volumes by ear). Root cause of the silence: the game referenced Sounds_4..7 (shot + 3 obstacle deaths) but those files were never shipped, and nothing played for power-ups at all. Added the 4 missing files and ~30 new effects (Content/Sounds/fx_*.mp3, synthesised with tools/audiogen.py (numpy)): jump, land, power-up chime + one per power-up (magnet, shield, turbo, jetpack, rocket, gun, blast), obstacle explosion/smash, gun shot, shield break, helmet, near miss, combo bonus, big air, stage up, revive, countdown tick/GO hit, crash, level-up, mission, popup open, purchase, error. New module Source/Features/PZAudio.h/.cpp (missing files are skipped silently, rate limited per effect; features/EnableSfx, tuning/SfxVolume).
22.2 Label sounds like Go!, Second Chance and others, add sounds for all these text should be man voice sound as in games (Taken 3)
    > Feedback: DONE with a caveat. Every banner label now announces itself automatically (PZUI::banner -> PZAudio::sayLabel): GO!, SECOND CHANCE!, GET READY!, SHIELD, BLAST!, JETPACK!, ROCKET BIKE!, GUN MODE!, LUCKY HELMET!, NEW BEST!, STAGE n, BIG AIR!, CLOSE CALL!, SMASH!, MISSION COMPLETE!, WELCOME BACK!, plus magnet/shield/turbo pickups and GAME OVER. Voice = deep male, graded (pitch down, bass, compression, slap echo) in Content/Sounds/vo_*.mp3; one voice at a time, never overlapping (features/EnableVoice, tuning/VoiceVolume).
    > Caveat: the voice is generated from the macOS 'Ralph' text-to-speech voice, not a recorded actor, and I cannot copy the real Taken 3 voice. Apple's TTS output may not be redistributable commercially, so for release replace the vo_*.mp3 files with a recorded/licensed voice - same file names, no code change.
23. When bike jumps and comes back to platform the yellow circle effect is not right, make it under the bike or tires fire, or flare at the platfrom under bike. Make it better
    > Feedback: DONE (simulator frame checked). The yellow ring around the bike on landing is gone. Landings now draw on the road under the tyres (PZRunDirector::landingFx): a flat shock ring that races along the road, sparks thrown off both wheels (ballistic arcs), a flame lick at each tyre on hard landings and a warm additive flare on the surface; the dust puffs and landing sound stay. Big Air landings use the same effect at full strength.
24. For coin shop increase the amounts for each coin make list of 6 coins packs
    > Feedback: DONE (simulator visual and config verified). The COINS tab renders six configured packs (1,500 / 3,500 / 6,000 / 12,000 / 25,000 / 50,000), with correct compact labels including 1.5K and 3.5K. Product IDs, values, and enabled state were checked in the shipped iOS configuration.
25. Add animations and transitions in all HUD and screens they should have move in or similar animations, screen transitions should be better, 25. Add animations and transitions in all HUD and screens they should have move in or similar animations, screen transitions should be better, do in sequence animations, like a menu buttons animate 1 by 1, also the buttons frame should have animation.
    > Feedback: DONE (simulator spot-check plus code review). Menu buttons stagger in, button frames pulse, popups animate their contents, and screen transitions fade. The opening adds a letterbox frame, staged HUD entrance, and separately paced GET READY / 3-2-1 beats. Timings are based on the actual voice-clip durations, and countdown audio is preloaded. Restart cancels stale cinematic/camera actions so screens cannot inherit a prior run's animation. Main menu, shop, pause, and intro framing were visually checked; this is not an exhaustive animation pass over every HUD state.
26. Add running bike on main menu and other screens excpt gameover. or add what's better as game on these screens to show active gameplay.
    > Feedback: DONE (simulator spot-check plus code review). The animated riding lane is visible on the main menu and pause overlay, and is omitted from game over and active gameplay. Character select retains its large selectable-bike preview instead of adding a competing rider.
27. The main menu should have bike running and other details, so when a user play first time it should not go away from a static screen. Make it interactive and interesting.
    > Feedback: DONE (simulator spot-check plus code review). The menu lane uses the selected bike, cycles decorative coins and dust, and adds occasional hops; tap-to-jump and the first-run hint are wired. The first-run hint stays out of the cinematic and enters at GO. Recycled decorative coins restore their opacity before reuse.
28. If there are too many notifications coming on screen give a gap, like Mission Complete, Get Ready etc. Do a proper logic on it.
    > Feedback: DONE (code review and simulator build). Gameplay/menu banners are deduplicated and serialized with a 0.6-second quiet interval and a bounded queue. Fixed queued-parent lifetime handling so delayed or evicted banners cannot dereference a removed scene node. Simultaneous notification stress was not separately exercised in the simulator.
29. Increase the price of Power Ups start from 1,000 to 20,000 coins.
    > Feedback: DONE (code-checked). The gadget price ladder runs from 1,000 coins for the magnet through 20,000 for the bike cannon, and the gadget shop reads those configured prices. Purchase transactions were not separately exercised in the simulator.
30. Reduce the frequency of coins by 50% exept some predefined set of coins like BIKE shape of coins group.
    > Feedback: DONE (code-checked). Loose authored coins use 0.5 density while contiguous groups of at least five (including BIKE-style shapes) remain intact; random-mode spawns and guide arcs also apply the density rule. Long-run coin-rate balance was not measured in a simulator session.
31. Another bug, when player falls down and revives i see some coins or power ups get's generated at some y postion as the player position when fall in pit
    > Feedback: DONE (code-checked; pit-spawn scenario not independently reproduced). Runtime power-up orbs are positioned relative to nearby platform geometry, with a screen-relative fallback if no platform exists; below-platform orbs are removed instead of remaining unreachable inside a pit.
32. Make start/death scenes dramatic, like when start scene zoomed in to left side bike entry, the bike should enter from the left side of the screen, and message with voice, "Get Ready, 3, 2, 1 Go!" during 3,2,1 the other items of the screen, coin and counter, buttons, powerup top horizontal list will comes animated in the screen, and at Go! the game starts with cheers sounds "wooo" from audience. Death scene same, the zoom in to collion animation, fire and disappointed people "haaaa" sounds, zoom out, show game over animated.
    > Feedback: DONE in implementation (iOS simulator build and start sequence verified). The bike eases in from the left under a letterbox frame; GET READY precedes the 3-2-1 count, the HUD reveals during the count, and GO visibly hands control to gameplay. Repeated restart was checked for stale camera/actions. Death uses a staged crash flash, impact effects, camera push/pull, and frame exit before either the continue offer or ordinary results; it no longer depends on revive eligibility. A lifecycle edge case was fixed so a death during the intro-delay window cancels pending start actions, and repeated HUD hiding preserves the restore list. The simulator reached the results/revive UI, but the death push/pull shot and speaker mix were not isolated well enough to claim visual/audio verification. Generated voice assets retain the licensing caveat under task 22.2.
33. Revive should end when next platfrom starts not at the time of pit, it falls down as soon as the revive grace period ends, also make available the tap immediately at powerup end and platform start.
    > Feedback: DONE (simulator gameplay path verified plus code review). A coin revive waited for a usable runway, settled, completed a paced 3-2-1 countdown, activated revive turbo, and accepted jump input after the boost ended. The revive/fall-rescue edge-case matrix and every other power-up ending were not exhaustively exercised in the simulator; code paths use safe-platform landing and restore grounded/jump state.
## Testing Round 3
34. Blinking (protected) rider must not fall into a pit after a power-up or revive
    > Feedback: DONE (iOS simulator, position log over several gaps with grace held). New PZRunDirector::updateGraceBridge: while the rider is blinking and over a gap it is held at the road level it left from (eased toward the next platform once that is close), gravity returns the moment ground is under it, a tap still jumps, and the blink cannot expire in the middle of a gap. Measured: rider y stayed 318-335 across 5 gaps in 18 s, no fall rescue/teleport.
35. Main menu: bigger bike and coins, layered animated background, more alive
    > Feedback: DONE (simulator frames). Bike 74 -> 172 units (about 105 px of visible bike), six 66-unit spinning coins in an arc, bike rides the curb in the artwork (not the water), road dashes stream past, hop pitches the bike. Backdrop is cut along its horizon lines into sky / wall / water / road strips that sway left-right at different speeds (6 / 11 / 17 / 25 px), plus drifting clouds, twinkling stars, shooting stars, flickering lamp glow, lamp reflections and glints on the water, fireflies, and a floating logo (PZPopups MenuParallax / MenuShowcase). Main menu only; the pause overlay keeps the plain backdrop.
36. Gameplay power-ups at the top, tappable, bigger round icons with usage counts; nothing equipped at the start of a run
    > Feedback: DONE (simulator: tap activates magnet, ring drains, count drops to 0, 'NO CHARGES - BUY MORE IN THE SHOP' on an empty tap). Round 78 px buttons (66 / 56 when 4+ / 6+) sit in the top row left of the pause button; the number on the badge is the stored charges; a breathing ring means a tap is available, a draining ring shows the effect running, grey = out of charges. A tap spends one charge and never makes the bike jump. Runs now start bare: applyGadgets no longer consumes charges or starts rides. The buttons show the gadgets picked in the GADGETS menu (up to 3; buying a charge still auto-fills an empty slot), and that screen now says 'PICK UP TO 3 - TAP THEM DURING A RUN'. Blocked while another ride is active, during the countdown, cinematics, pause and revive.
37. Top-level banner / HUD text unreadable
    > Feedback: DONE. Main-menu level plate rebuilt: 290x100, 'LEVEL' badge, XP counter 12 / best 9.5 / next-unlock 9.5 (was 6 -> clamped to 14 px), 16-unit XP bar, pills 28 high with 14-size captions. In-run mission tracker, power-up chips and combo text enlarged the same way. Not changed: the global minimum font (tried 18, it overlapped text in the gadget cards, so reverted to 14); smaller captions elsewhere (popup sub-lines) are still 14 px.

Re-check of 24-33 (this pass): 24 six packs and Android's six consumables confirmed in IAPSettings / BillingBridge (docs/IAPs.md updated); 31/33 revive and power-up endings now share the grace bridge above, so no code path drops a blinking rider into a pit. 25-30 and 32 were code-reviewed earlier and not re-measured here (coin density, banner queue under stress and the death push/pull audio still need a by-ear / long-run pass on device).

ADS CHECK (iOS simulator, real unit IDs, 2026-11-06): UMP + Google Mobile Ads start, rewarded ad loaded, interstitial loaded, banner loaded (956x66, 'Test mode' badge because the simulator is a test device). Fixed on the way: a banner/interstitial requested while the consent flow was still open was dropped and never retried (no banner on first launch); now deferred and sent once consent allows ads. Content/ads.plist now has the iOS rewarded unit (ca-app-pub-8966545526103390/5633843231) and testMode also covers rewarded. Chartboost returned 'no ad found' in the simulator (expected; AdMob is the fallback).
    > Still needed before store submission: (a) Android rewarded unit ID (rewardedIdAndroid is empty, so video revive / spin / free coins have no ad on Android), (b) AdMob Privacy & messaging: the UMP consent form is not configured for the app ('no form(s) configured for the input app ID'), required for EEA/UK users, (c) a real-device check of Android banner/interstitial (not run here), (d) Privacy Options button still has no UI.

## Testing Round 4
38. Remove Ads / Reminders chips; settings; menu arrangement
    > Feedback: DONE (simulator). Main menu: Restore / Kids Bike / Coin Store stacked down the left, logo centred, Play moved a little lower, and one centred chip row REMOVE ADS | REMINDERS | SETTINGS (vibration moved into Settings; the art 'REMOVE ADS' banner is hidden on the main menu, still art on pause/game over). New SETTINGS popup: sound effects, music, vibration, reminders, privacy options (persisted: PZ_SoundOn / PZ_MusicOn; the audio switches live in the SimpleAudioEngine shim and PZAudio).
39. Pause / game over / character screens vs the banner
    > Feedback: DONE. Root cause: the layout reserved 104 units for the banner but a real adaptive banner plus the home-indicator inset is ~126 (tuning/BannerReserve is now 128), so buttons overlapped it. Game-over buttons and stats panel moved up; character screen BACK now sits on the same row as PLAY; pause lane bike runs behind the buttons.
40. Main menu bike on the pavement, tilted with the floor, no black/yellow lane line; coins lower
    > Feedback: DONE. The pavement band of the backdrop is lifted so a strip of it shows above the banner; the bike rides there (no water), leans with the slope, and the dashed lane strip is gone.
41. Round buttons (play / back / restart / continue / pause) had a ragged outer edge
    > Feedback: DONE. The 9 button frames in animation_s0/1/2.png were clipped 2.5 px inside their circle with a feathered edge (tools: clip script kept out of the repo; originals are in git).
42. Jetpack also pulls coins in (magnet) for its whole duration. No blinking / transparency at the start of a run (start protection is now invisible). Coin packs are 5,000 / 15,000 / 40,000 / 100,000 / 250,000 / 500,000 (store prices to be set in the consoles).
43. AdMob policy wording on rewarded entry points
    > Feedback: DONE. Every rewarded button now says WATCH AD and carries a play icon: 'WATCH AD: 2X COINS', 'WATCH AD: CONTINUE', 'WATCH AD REVIVE', 'WATCH AD +N COINS', 'WATCH AD: SPIN AGAIN', 'WATCH AD: GIFT'. Privacy options entry exists in Settings (Google UMP, iOS + Android). The UMP consent FORM itself still has to be created in the AdMob console (Privacy & messaging) - it cannot be done from code.

44. PLAY button centred on the XP plate / pill column; menu bike tilted left-up / right-down and as low as the banner allows
    > Feedback: DONE (simulator). PLAY shares the column centre; the bike tilts 3.4 degrees (wheels stay on the pavement, lowest point sits right above the ad banner).

45. Run commands without device ids
    > Feedback: DONE. tools/run.sh finds the iPhone / booted simulator / Android device itself (one target = used, several = numbered list, none = pick a simulator to boot). BUILD_COMMANDS.md section 0 documents it; the hard-coded device id is gone from the docs.
46. Run start: the bike no longer drops in from above
    > Feedback: DONE (simulator frames). The rider is parked hidden off-screen with gravity off until the intro starts, then rides in from the left.
47. Revive / power-up end / fall rescue: no more teleporting
    > Feedback: DONE (simulator position log: jetpack end 484 -> 386 -> 329 over ~0.45 s). PZRunDirector::settleRider glides the rider onto the platform and, when the platform is not under it, slides the world in at the same time (PZGGamefieldPlatforms::findReviveTarget + shiftWorld). Used by rescueRider (revive, shield / mercy / helmet saves) and by turbo / jetpack endings. The rider is untouchable during the glide; a jump tap hands control back immediately.
48. BEST overlapping XP on the menu plate; BEST in the game HUD
    > Feedback: DONE. The XP text shrinks to leave room for BEST; in the run HUD BEST sits on the coin / distance line, between the two counters.
49. Upload readiness
    > Feedback: versions bumped to 1.4.0 (iOS build 10, Android versionCode 6). See the release notes at the end of this file section / BUILD_COMMANDS.md for the artifacts that were produced.

50. Out of charges -> shop (monetisation)
    > Feedback: DONE (simulator). Choosing a gadget with 0 charges in the in-run picker opens the SHOP (power-ups tab) over the held run; closing it resumes the run and refreshes the counts. The shop gadget cards were re-laid out so name / status / price no longer overlap.
51. In-run GADGETS button + picker with proper icons
    > Feedback: DONE (simulator). The top bar now has one round GADGETS button (2x2 board of the real magnet / shield / rocket / turbo icons, badge = total charges, ring breathes; red ring when everything is at zero). It opens a picker with all eight gadgets (nature-specific icons, charges, LV lock); tap one and it applies to the rider at once, the run is held while the picker is open. Menu GADGETS screen now just sells charges (no equip slots).
52. Menu bike `tuning` keys checked
    > Feedback: DONE. Temporarily changing MenuBikeHeight / MenuBikeTilt / MenuCoinSize in the plist changed the menu bike and coins on the next launch, so the tuning section is read correctly. NOTE: MenuBikeLift in the plist is currently -10 (your value) and was kept. Values: tuning/MenuBikeHeight, MenuBikeTilt, MenuBikeSlope, MenuBikeLift, MenuBikeSpeed, MenuBikeHop, MenuCoinSize, MenuCoinLift, MenuCoinArc.

53. Voices inaudible in game
    > Feedback: DONE. Cause: the voice grade pushed pitch down and boosted the bass, so 80-95 % of the energy sat below 400 Hz, which phone speakers cannot play (the sound effects live at 400 Hz-3 kHz, which is why only they were heard). tools/audiogen.py voice now high-passes at 320 Hz, boosts 1-3 kHz, compresses and normalises each line to about -11 dB mean; all 24 vo_*.mp3 regenerated (same Ralph voice, same licensing caveat as 22.2). Not listened to on a device speaker by me: please check.
54. Gadgets button icon: zig-zag lightning bolt with a glow (pulsing), replaces the 2x2 icon board.
55. Test vs live ads is now automatic: debug builds force Google sample units, release builds use the live ids (see BUILD_COMMANDS.md "Ads: test vs live").

56. iPad layout pass (menus + HUD)
    > Feedback: DONE (iPad Pro 13 simulator). Added PZLayout::uiScale() (1.0 on phones, 0.66 on 4:3 tablets where the design width is only ~850 units). Menus: right column, chip row, left art buttons, logo, Play, bike and coins shrink and re-space; REMOVE ADS now sits alone in the top-left corner. Character screen: COIN STORE / kid / upgrade buttons shrink. Gameplay HUD: coin icon + counter, distance, BEST, pause, gadget button, power-up chips, mission tracker and combo all scale; the gadget button sits left of the pause button and the tracker below both (no overlaps). Shop grid uses 4 columns. The earlier FIXED_WIDTH experiment (world positioning, floating debris / bushes, empty top) was reverted: gameplay keeps the original 640-high design space.
57. Voices removed: big air, power-up pickups (magnet / shield / turbo / jetpack / rocket / gun / blast / helmet), close call. "MISSION COMPLETE" is now announced once: missions already announced live in the run are not repeated on the game-over screen.

## ##################################
## Agent comments
Latest fix (revive killer obstacle): the obstacle that caused a death, or the next one, could still be waiting when control
returned. Revive/mercy/shield/helmet rescues and powered-ride landings now explode all hazards around the rider (visibly, no
reward), the rider is untouchable during the hover and the 3-2-1 countdown, duplicate death signals are ignored while a revive
is pending, the hover has a 2.5 s cap, and the CONTINUE? offer lasts 8 s (`tuning/ReviveCountdown`). Full matrix in
docs/META_FEATURES.md ("Death / revive use cases").


ADMOB:

IOS
Reward ID: 
ca-app-pub-8966545526103390~4717282854
ca-app-pub-8966545526103390/5633843231

intertitial:
ca-app-pub-8966545526103390~4717282854
ca-app-pub-8966545526103390/3450330423

MainMenu Banner:
ca-app-pub-8966545526103390~4717282854
ca-app-pub-8966545526103390/6076493765

Android
Banner
ca-app-pub-8966545526103390~3276839875
ca-app-pub-8966545526103390/1144454373

Intertitial:
ca-app-pub-8966545526103390~3276839875
ca-app-pub-8966545526103390/3534813999

Reward:
ca-app-pub-8966545526103390~3276839875
ca-app-pub-8966545526103390/5655160008