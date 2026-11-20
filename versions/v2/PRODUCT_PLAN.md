# Dino Bike Escape — Version 2 Product and Development Plan

Status: proposal, not implementation  
Prepared: 2026-11-20  
Target platforms: iOS and Android  
Engine: Axmol/C++20, Box2D

## 1. Executive decision

Version 2 should turn the game from “an endless runner with many power-ups” into **a comedy chase game players can master, talk about, and challenge friends to replay**.

The recommended product promise is:

> Escape a ridiculous dinosaur traffic chase, survive changing road events, and beat your friends' runs.

Do not start with real-time multiplayer. It is expensive, introduces latency, matchmaking, reconciliation, cheating, backend, and low-population problems, while the current single-player loop still needs more authored variety. First ship deterministic daily runs, ghosts, leaderboards, friend challenges, and shareable result cards. These provide most of the competitive/social benefit with much lower risk. Prototype live multiplayer only after the social version proves demand.

## 2. What already exists

The repository audit shows that v2 must build on, not duplicate, the current feature set:

- One-touch platform-runner gameplay using Axmol and Box2D.
- Coins, obstacles, procedural platforms, score/distance, multiple bikes/characters.
- Magnet, shield, turbo, blast, jetpack, rocket-bike, and gun power-ups.
- Big-air, combo, near-miss, stages, atmosphere tinting, cinematic blast, camera and glitter effects.
- Gadgets, XP/levels, missions, daily rewards, gifts, lucky spin, revive, rewarded-ad hooks, notifications, settings, IAP, and haptics.
- An iOS Game Center bridge already authenticates, opens leaderboards, and submits scores.
- Android Game Center behavior is currently a no-op; no Google Play Games bridge exists.
- There is no cross-platform friend system, ghost system, replay format, share-card flow, server-authoritative leaderboard, analytics taxonomy, or live-event delivery layer.

### Current product weakness

The game has accumulated features, but features alone do not create identity. The run needs recognizable situations, anticipation, player decisions, recurring rivals, and stories worth sharing. More reward popups will not solve that.

## 3. V2 goals and non-goals

### Goals

1. Make the first three runs funny, readable, and immediately different.
2. Create a meaningful event or decision roughly every 15–25 seconds.
3. Give players short, fair challenges that take 1–5 minutes.
4. Make every good run easy to share or use to challenge a friend.
5. Add social competition without requiring both players online.
6. Instrument the funnel so decisions are based on evidence rather than taste.
7. Preserve 60 FPS and keep all new systems configurable and removable.

### Non-goals for the first v2 release

- Open-world movement or a second steering axis.
- PvP combat, chat, clans, or user-generated text.
- A large narrative campaign.
- Mandatory accounts before the first run.
- Synchronous multiplayer in production.
- More currencies, more wheels of fortune, or more notification pressure.

## 4. Product pillars

### Pillar A — The road tells jokes

Comedy must happen during play, not only in menus. Use short, readable setup → reaction → payoff sequences. Humor must never hide collision information or steal control during a dangerous jump.

### Pillar B — A run has chapters

Replace a flat difficulty ramp with authored “road events” selected by a director. Each run becomes a sequence of recognizable situations: traffic, chase, storm, rival, construction, stampede, boss escape.

### Pillar C — Skill is visible

Coins and distance are not enough. Add optional risk lines, stunt scoring, clean sections, close calls, rival overtakes, and clear medals. Players should understand why a run was impressive.

### Pillar D — Friends can participate later

A friend should be able to receive a challenge, play when convenient, see the sender’s ghost, and rematch. This avoids simultaneous availability and empty matchmaking queues.

## 5. Core run redesign

### 5.1 Run rhythm

Use a lightweight `RunEventDirector` with deterministic random seeds. A normal run alternates:

1. **Breather** (5–8 s): readable coins and simple jumps.
2. **Setup** (2–4 s): sign, dialogue bubble, audio sting, or background clue.
3. **Event** (8–18 s): traffic/rival/weather/chase gameplay.
4. **Payoff** (2–4 s): stunt result, reaction, reward, or funny failure.
5. **Recovery** (3–6 s): safe platform and next choice.

Rules:

- Never overlap two control-changing events.
- Do not spawn a dialogue bubble over the landing zone.
- Every event declares required speed, minimum platform length, safe entry, safe exit, compatible power-ups, and cleanup behavior.
- Event selection uses a shuffle bag with cooldowns, not unrestricted random selection.
- The seed plus input log must reproduce a run for ghosts and QA.

### 5.2 “Other bikes pass by” system

This is high value if it has gameplay meaning. Do not make bikes random foreground decoration that obscures hazards.

Add three kinds of traffic:

- **Ambient riders:** pass in a background lane, wave, crash harmlessly, carry visual jokes. No collision.
- **Rivals:** enter with a name and portrait, follow a deterministic line, and create a short overtake objective.
- **Hazard riders:** telegraph before crossing the player lane; collision rules are explicit and consistent.

Initial rider personalities:

| Rival | Behavior | Joke/payoff | Player lesson |
|---|---|---|---|
| Grandma Turbo | Slow entrance, absurd rocket overtake | Her shopping basket drops bonus coins | Follow coin arcs |
| Delivery Dino | Wobbles with stacked pizza boxes | Perfect landing keeps pizzas; bump scatters toppings | Smooth jumping |
| Cool Kid | Performs a wheelie beside the player | Looks back and hits a sign if beaten | Overtake timing |
| Safety Officer | Follows every warning sign | Gets chased by his own safety cone | Read telegraphs |
| Tiny Triceratops | Very small bike, very loud horn | Horn startles background creatures | Audio cue recognition |

Rival encounter acceptance criteria:

- A 1.5–2.0 second safe introduction precedes competition.
- A progress marker shows PLAYER vs RIVAL without covering the HUD.
- Losing never ends a run; winning grants stunt score/coins and rivalry progress.
- Rider physics are simplified or kinematic; do not add full Box2D bikes for decorative traffic.
- At least one safe cleanup path exists for death, pause, revive, jetpack, and cinematic blast.

### 5.3 Road-event catalog

Build six events for v2 launch, then add one per content update:

1. **Dino Crossing:** warning signs, small dinos cross, a large late straggler creates a jump choice.
2. **Pizza Panic:** chase a delivery rider; collect falling boxes without missing the road.
3. **Construction Chaos:** moving lift, paint rollers, cones, and a worker who rotates the wrong sign.
4. **Meteor Shower:** shadows telegraph impacts; craters become ramps; perfect dodges build a bonus.
5. **Police Chase Mix-up:** the police initially chase the player, then pass and chase a thief dinosaur instead.
6. **T-Rex Boss Escape:** a 30–45 second set piece with roars, bites at telegraphed positions, and a final stunt ramp.

Later candidates: parade, haunted tunnel, low-gravity lab leak, chicken stampede, ice-cream truck chase, alien checkpoint, collapsing bridge.

### 5.4 Risk routes and micro-challenges

Present occasional two-choice layouts that preserve one-touch control:

- Low safe route versus high stunt route.
- Coin trail versus power-up trail.
- Shortcut through breakable signs versus longer clean road.
- Chase rival versus collect a letter set.

The choice must be visible at least 1.5 seconds before commitment. Difficulty should affect reward, not produce blind traps.

Micro-challenges appear inside runs and last 10–25 seconds:

- Land three perfect jumps.
- Stay above the rival until the checkpoint.
- Collect five pizza boxes.
- Pass the speed camera above a target speed.
- Survive a section without a power-up.
- Smash exactly three marked obstacles.
- Spell D-I-N-O from a risky coin line.

Only one micro-challenge is active at once. Failure should show a quick reaction and continue the run.

## 6. Comedy animation and dialogue plan

### 6.1 Animation vocabulary

Create reusable animation components instead of one-off timeline code:

- `SquashStretchAction`: landing, pickup, UI reward.
- `AnticipationAction`: lean-back before boost or jump.
- `ReactionBubble`: surprise, anger, laugh, dizzy, panic.
- `LookAtAction`: rider looks at rival, hazard, or camera.
- `PropEjector`: helmet, pizza, cone, wheel, fish, sign.
- `ComicImpact`: freeze 40–80 ms, radial lines, shake, sound, then recovery.
- `BackgroundGag`: non-colliding event with visibility and frequency limits.

Prefer Axmol actions, particles, sprite-frame animation, and pooled nodes. Avoid adding a skeletal-animation runtime unless the art pipeline truly needs bone deformation; it raises package size and production cost.

### 6.2 Launch animation list

- Bike coughs twice before a rocket power-up ignites too strongly.
- Rider checks behind, sees the T-Rex, and pedals faster for one second.
- Rival waves, nearly falls, recovers, and pretends nothing happened.
- Helmet spins in the air and lands backward after a shield save.
- A passing rider’s horn makes a background dinosaur jump.
- Perfect landing compresses tires, pops the rider upward, and triggers a small confident nod.
- Failed micro-challenge shows a tiny sarcastic roadside score judge.
- New best: background paparazzi dinosaurs appear for the result pose.
- Revive: mechanic dinosaur rolls the player back in, slaps the bike, and the exhaust falls off once.
- Idle menu: rider cleans the bike; bike immediately gets muddy again.
- Rare 1-in-50 gag: a dinosaur in a business suit waits at a bus stop and checks its watch.

Animation budget: one major gag at a time; background gag cooldown 20–35 seconds; no screen-covering gag during precision input.

### 6.3 Dialogue approach

Use short authored bark lines, not chat and not generated text. Maximum 32 visible characters, one or two lines, 1.5–2.5 seconds, localized through IDs.

Sample pre-run rival lines:

- “Try to keep up. Politely.”
- “My grandma tunes this bike.”
- “Last one there is fossil fuel.”
- “No shortcuts! Unless I find one.”

Sample event lines:

- “Why are the cones moving?”
- “That pizza is still good!”
- “Meteor insurance was extra.”
- “This feels legally questionable.”

Sample result lines:

- Win: “I meant to let you win.”
- Loss: “Rematch? My horn was broken.”
- Crash: “The road attacked first.”
- New best: “Put that on the fridge.”

Dialogue selection rules:

- Never repeat the last five barks.
- Context tags: rival, event, win/loss, near miss, crash type, power-up.
- Disable dialogue independently from SFX and music.
- All dialogue must work without voice-over; voice is optional polish.

## 7. Challenge modes

### 7.1 Daily seeded challenge — v2 launch priority

Every player gets the same seed, event order, power-up placements, and challenge rules for that UTC day. One free ranked attempt plus unlimited practice attempts is fairer than blocking play. Ranked attempts may be configurable after testing.

Daily modifiers:

- One-hit ride (no revive, high score multiplier).
- Pizza delivery (keep cargo intact).
- Rocket rush (frequent rocket, tighter obstacles).
- Coin drought (score comes from stunts).
- Rival gauntlet (three rivals in one run).
- Floor is lava (longer air time and scarce platforms).

Scoring should be server/config versioned: `distance + stuntScore + eventBonuses - collisionPenalties`. Store seed, rules version, content version, platform, and score proof metadata.

### 7.2 Weekly league

Use five non-punitive divisions: Rookie, Rider, Pro, Legend, Fossil. Promote the top band; never demote inactive players more than one division per week. Rewards should be cosmetic/profile-oriented, not power that makes competitive scores unfair.

Do not build this until daily challenge completion and leaderboard reliability are proven.

### 7.3 Achievements

Create 15–20 meaningful achievements at launch, leaving room to grow:

- First Escape, 1 km Club, 10 km Lifetime.
- Three Perfect Landings.
- Beat Every Rival.
- Survive a T-Rex Event.
- Win a Friend Challenge.
- Pizza Arrived Warm.
- Close Call Collector.
- Hidden: Business Dino Spotted.

Use Game Center achievements on iOS and Play Games achievements on Android; keep an internal platform-neutral achievement service so gameplay code knows nothing about native APIs.

## 8. Social and sharing plan

### 8.1 Shareable result cards — first social release

After a new best, rival win, daily medal, or rare gag, offer a non-blocking SHARE button. Generate a 1080×1920 or 1200×630 card containing:

- Player bike/character and result pose.
- Distance, stunt score, medal, challenge/date.
- Seed/challenge identifier where relevant.
- QR/deep link or store link.
- “Can you beat this?” copy.

Use the native iOS share sheet (`UIActivityViewController`) and Android Sharesheet (`ACTION_SEND`). Do not integrate Facebook/TikTok SDKs just to share an image. Never grant a gameplay reward for completing a share; the OS does not reliably prove meaningful sharing and incentivized sharing is spammy.

### 8.2 Friend challenge — recommended multiplayer

Flow:

1. Player finishes a daily or normal seeded run.
2. Tap **Challenge a Friend**.
3. Game creates a compact challenge token: challenge ID, seed, rules version, target score, expiry, and sender display data.
4. Native share sheet sends a universal/app link.
5. Recipient installs/opens the game directly into a challenge preview.
6. Recipient races the identical seed with the sender’s translucent ghost.
7. Both see result and **Rematch**.

Guest recipients can play before account linking. Expired links should fall back to the current daily challenge rather than a dead screen.

Privacy: use platform nickname/avatar only after consent; otherwise generate a rider name. Do not expose email, contacts, device IDs, or exact activity times.

### 8.3 Ghost racing

Record inputs, not transforms, whenever deterministic replay is reliable:

```text
ReplayHeader {
  formatVersion, contentVersion, physicsVersion, seed,
  bikeId, gadgetIds, rulesHash, durationMs, finalScore
}
InputEvent { tick, action: Press | Release }
Checkpoint { tick, x, y, velocityX, velocityY, stateHash }
```

Use fixed-step simulation for challenge mode. Add checkpoints every 1–2 seconds to detect drift; if a ghost diverges, interpolate visual state or hide it rather than affecting gameplay. Ghosts are non-colliding, muted by default, visually translucent, and can be disabled.

### 8.4 Leaderboards

Ship in this order:

1. Existing iOS all-time distance leaderboard—verify configuration and error handling.
2. iOS daily challenge leaderboard and Game Center challenge integration.
3. Android Play Games Services v2 bridge for sign-in, leaderboards, achievements.
4. Cross-platform leaderboard only when a protected backend and anti-cheat checks exist.

Separate boards by rules/version where physics or content changes affect fairness. Do not compare old and new physics scores indefinitely.

## 9. Multiplayer decision

### Option comparison

| Option | Player value | Engineering cost | Operational risk | Recommendation |
|---|---:|---:|---:|---|
| Share cards + native leaderboards | Medium | Low | Low | Build first |
| Seeded friend challenges | High | Medium | Low–medium | V2 core |
| Ghost racing | High | Medium | Medium | Build after determinism spike |
| Same-device race | Medium | Medium | Low | Optional experiment; small screens are a problem |
| Real-time 1v1 | High if population exists | Very high | Very high | Prototype only after evidence |
| Large multiplayer/lobbies | Unclear | Extreme | Extreme | Reject |

### Real-time prototype gate

Only fund a two-week real-time prototype if all are true for at least four consecutive weeks:

- Daily challenge participation ≥ 20% of DAU.
- At least 8% of finishers initiate a share/challenge.
- At least 20% of opened challenge links produce a completed run.
- D7 retention improves materially versus the pre-social cohort.
- Community feedback repeatedly asks to race at the same time.

Prototype scope: two players, identical seed, no contact/collision, position snapshots only, emotes from a fixed set, bots after a short matchmaking timeout. Never let an empty queue block play.

## 10. Architecture

Use clean boundaries so game rules are testable without Axmol scenes or native SDKs:

```text
Presentation
  Run HUD / Challenge UI / Result Card / Social Screens
        |
Application
  StartChallenge / FinishRun / CreateShare / SubmitScore / LoadGhost
        |
Domain
  RunRules / EventDirector / Challenge / Replay / Score / Rival
        |
Ports (interfaces)
  ILeaderboard / IAchievement / IShare / IChallengeRepository
  IReplayStore / IAnalytics / IRemoteConfig / IClock
        |
Adapters
  GameKit / Play Games / Native Share / Local JSON
  Optional Firebase or custom HTTPS backend
```

Suggested source layout:

```text
Source/V2/Domain/
Source/V2/Application/
Source/V2/Presentation/
Source/V2/Infrastructure/Apple/
Source/V2/Infrastructure/Android/
Source/V2/Infrastructure/Local/
```

Rules:

- `PZRunDirector` should emit domain events; it must not grow into the network/social service.
- Native SDK calls remain behind adapters.
- All remote/config data has bundled defaults, schema versions, min/max validation, and safe fallback.
- Network failure never prevents a normal offline run.
- Event assets are preloaded or streamed outside precision sections.
- Use dependency injection through small constructor/context objects, not new global singletons.

### Lower-cost packages and platform services

- **Axmol actions/particles/SpriteFrameCache:** reuse for comedy animation and pooling.
- **GameKit:** existing iOS bridge; expand it instead of introducing a third-party social SDK.
- **Google Play Games Services v2:** native Android leaderboard/achievement/friends surface.
- **Native share sheets:** no social SDK dependency.
- **Firebase C++ SDK (optional, only for cross-platform challenges):** Authentication, Firestore/Realtime Database, Remote Config, Analytics, Crashlytics, and App Check reduce backend work. Validate current Axmol/iOS/Android build compatibility in a spike before committing.
- **nlohmann/json (optional):** header-only versioned challenge/replay serialization if the existing Axmol value/JSON facilities become cumbersome. Do not add it merely for simple configuration.
- **zstd or miniz (only if needed):** compress replay input logs after measuring actual payloads.

Avoid Photon/PlayFab/Nakama until live or custom cross-platform requirements justify them. A managed multiplayer dependency before product validation adds integration and operating cost without solving the core fun problem.

## 11. Analytics and experiments

No engagement plan is credible without measurement. Add a platform-neutral analytics port and a debug event inspector before tuning retention.

Minimum event taxonomy:

| Event | Required fields |
|---|---|
| `tutorial_step` | step, success, time_ms, attempt |
| `run_start` | mode, seed, bike, gadgets, player_level |
| `run_end` | mode, distance, duration, death_reason, score, revive_count |
| `road_event_start/end` | event_id, variant, success, duration |
| `micro_challenge_end` | challenge_id, success, progress |
| `rival_end` | rival_id, win, lead_changes |
| `powerup_pickup/end` | type, source, duration, end_reason |
| `daily_start/end` | challenge_id, attempt, medal, rank_band |
| `share_prompt` | context, shown, tapped |
| `share_complete` | context, activity_type_if_available |
| `challenge_link_open` | challenge_id, install/open, expired |
| `friend_challenge_end` | challenge_id, sender_score, result |
| `ghost_desync` | format, physics_version, tick, error |

Primary metrics:

- Tutorial completion and first-run completion.
- Runs per new player in first session.
- D1, D7, and D30 retention by acquisition/version cohort.
- Median session length and runs per DAU.
- Death reason distribution by distance band.
- Event encounter, success, and quit rates.
- Daily challenge participation/completion.
- Share tap rate, challenge-link open rate, and recipient completion.
- Crash-free sessions, low-memory exits, and frame-time percentiles.

Guardrails:

- Ad impressions per session and quit-after-ad rate.
- Revive prompt decline/quit rate.
- Notification opt-in and disable rate.
- No experiment may make hazards less readable or sell competitive power.

Do not hard-code “industry benchmark” targets. Establish the current baseline, then require statistically useful improvement against it.

## 12. Delivery roadmap

Each phase is a releaseable slice. Do not begin the next large system until the previous one is measured and stable.

### Phase 0 — Baseline and technical spikes (1–2 weeks)

- Add analytics interface, local debug logger, session/run/death funnel events.
- Capture current retention and gameplay baselines if a production analytics provider is available.
- Add deterministic fixed-step test mode and seeded content generation.
- Build a 60-second replay prototype; verify identical outcomes on repeated runs and both platforms.
- Audit iOS Game Center configuration and score submission.
- Spike Android Play Games Services v2 sign-in and one test leaderboard.
- Spike native share bridge with a generated result-card image.

Exit gate: a seed reproduces placements; replay drift is measured; one share card works on both platforms; no regression in normal offline play.

### Phase 1 — Make the run memorable (3–5 weeks)

- Implement `RunEventDirector` and event contract.
- Ship three events: Dino Crossing, Pizza Panic, Construction Chaos.
- Add ambient traffic and two rivals.
- Add reusable reaction/prop/comic-impact animation components.
- Add contextual bark system with localization IDs.
- Add perfect-landing and event-success feedback.

Exit gate: six distinct event combinations in ten test runs; no unfair event-entry deaths in automated seed sweep; stable frame time on minimum devices.

### Phase 2 — Challenges and mastery (2–4 weeks)

- Daily seeded challenge with 5–7 modifiers.
- Versioned scoring and separate challenge results.
- In-run micro-challenges and medals.
- Platform achievements.
- Daily leaderboard on iOS; Android leaderboard after bridge validation.

Exit gate: all players receive identical daily content; offline result queues and later submits; clock changes do not duplicate rewards.

### Phase 3 — Sharing and friend competition (3–5 weeks)

- Result-card renderer and context-sensitive share moments.
- Universal Links/App Links and challenge landing/deferred-install strategy.
- Challenge token service or platform challenge APIs where sufficient.
- Ghost playback with desync detection.
- Rematch and result comparison UI.
- Privacy, expiry, blocking/reporting only if custom identities become visible.

Exit gate: sender → share → recipient → run → result → rematch works on real iOS and Android devices; expired and offline links degrade safely.

### Phase 4 — Content and live operations (ongoing)

- Add remaining launch events and rivals.
- Remote-config event weights and safe tuning ranges.
- Weekly league only after daily engagement proves useful.
- Seasonal cosmetic themes that do not change collision geometry.
- Monthly event/animation pack with automated seed sweeps.

### Phase 5 — Multiplayer experiment (optional, evidence-gated)

- Position-only 1v1 proof of concept.
- Measure latency, queue time, disconnects, bot substitution, and cost per match.
- Kill the experiment if it is not more fun than a ghost race in blind playtests.

## 13. First backlog, in priority order

### P0 — Required

1. Analytics abstraction and baseline events.
2. Seeded deterministic challenge mode and replay spike.
3. Run-event contract and automated compatibility validation.
4. Three polished road events.
5. Two rivals plus ambient pass-by bikes.
6. Native result-card sharing.
7. Daily challenge and versioned leaderboard.
8. Crash/performance/seed-sweep test coverage.

### P1 — Strong follow-up

9. Ghost playback.
10. Shareable friend-challenge links and rematches.
11. Platform achievements.
12. Three more events and three more rivals.
13. Weekly league after metrics validate it.

### P2 — Experiments

14. Real-time race prototype.
15. Seasonal event packs.
16. Spectator/result highlight replay.

## 14. Side effects and regression checklist

Every feature must be tested against:

- Pause/resume, background/foreground, call interruption.
- Death during event, revive during event, game-over revive, restart.
- Shield, magnet, turbo, blast, jetpack, rocket, gun, and gadgets.
- Event cleanup when a node is destroyed or a scene changes.
- Fixed-height safe areas, narrow phones, tablets, and banner/no-banner layouts.
- iOS/Android physics-scale differences already present in the project.
- Offline startup, network loss mid-submit, duplicate callbacks, retry queues.
- Old save migration and config missing-key defaults.
- Ads disabled, remove-ads purchased, rewarded ad unavailable.
- 30/60 FPS frame pacing and low-memory asset recovery.
- Localization expansion, large numbers, accessibility settings, sound-off mode.

Automated checks:

- Simulate at least 1,000 seeds per event pack; assert a reachable safe route and valid cleanup.
- Replay golden input logs after physics/content changes.
- Schema round-trip tests for challenge tokens and replay files.
- Idempotency tests for score/reward submission.
- Performance budgets for active nodes, particles, draw calls, and frame time.

## 15. Content production template

Every new road event must provide:

```text
Event ID and version
One-sentence player fantasy
Setup / gameplay / payoff timeline
Entry and exit safety rules
Required and optional art/audio
Collision and scoring rules
Compatible/incompatible power-ups
Pause/death/revive/restart cleanup
Difficulty variants
Dialogue tags
Analytics events
Accessibility/readability notes
Performance budget
QA seeds and acceptance tests
```

This prevents clever prototypes from becoming fragile production code.

## 16. Release criteria

V2 is ready only when:

- First run reaches gameplay quickly and explains control without a modal wall.
- A new player encounters at least two understandable memorable events in five minutes.
- Daily challenge is deterministic and fair across supported devices.
- Social features remain optional and normal play works offline.
- No competitive reward depends on watching an ad or buying power.
- New share/challenge flows pass privacy and child-safety review for the game’s target audience.
- Crash-free and frame-time metrics are no worse than the previous production version.
- Existing power-ups, revive, menus, saves, IAP, ads, and notifications pass regression QA.

## 17. Research basis

- Apple describes Game Center leaderboards, achievements, matchmaking, real-time/turn-based play, and saved games: [Game Center overview](https://developer.apple.com/game-center/).
- Apple’s current challenges are leaderboard-based, time-limited competitions that can notify friends, show scores, and support rematches: [Manage challenges](https://developer.apple.com/help/app-store-connect/configure-game-center/manage-challenges) and [Game Center challenges](https://developer.apple.com/documentation/appstoreconnectapi/game-center-challenges).
- Apple recommends short, individually completed, skill-based challenges with clear measurement and gives 1–5 minutes as the useful duration: [Game Center design guidance](https://developer.apple.com/design/human-interface-guidelines/game-center).
- Google Play Games achievements and leaderboards provide the Android-native progression/competition layer: [Achievements](https://developer.android.com/games/pgs/achievements) and [server-side leaderboards](https://developer.android.com/games/pgs/android/server-access).
- Native sharing avoids unnecessary social SDK dependencies: [Apple sharing](https://developer.apple.com/documentation/uikit/collaborating-and-sharing-copies-of-your-data) and [Android sharing](https://developer.android.com/training/sharing/send).
- Unity’s analytics definitions support tracking DAU, session length, sessions per DAU, and retention rather than guessing whether engagement improved: [Analytics dashboards](https://docs.unity.com/en-us/analytics/dashboards/dashboards).
- Research across several games finds that challenge affects retention but is mediated by player segments, social context, time, and money—supporting measurement and segmentation instead of a universal difficulty assumption: [Challenge and Retention in Games](https://escholarship.org/uc/item/6k3357qx).
- If a cross-platform backend is introduced, Firebase provides Auth, databases, Remote Config, Analytics, Crashlytics, and game-oriented services; App Check adds app/device attestation but does not eliminate all abuse: [Firebase for Games](https://firebase.google.com/docs/games/setup) and [App Check](https://firebase.google.com/docs/app-check).

## 18. Final recommendation

Build **events + rivals + daily seeded challenges + sharing + ghosts**. That is the coherent v2.

Do not build a full live-service economy or real-time multiplayer yet. The game first needs memorable situations and evidence that players want to compete. If friend challenges create repeated play, live racing becomes a justified next bet. If they do not, real-time networking would only make an underused feature more expensive.

## 19. Engineering review and implementation status (2026-11-20)

### 19.1 What the code audit changes in this plan

The plan's direction (events, rivals, daily seeds, ghosts, sharing) stands. Reading the actual runner changes the **order** and adds prerequisites:

1. **Determinism is not free.** Level content is generated by the legacy chunk pipeline with unseeded `rand()` / `CCRANDOM` (15+ call sites in `PZGGamefieldPlatforms.cpp`) and Box2D is stepped with a **variable** `dt` (`world->Step(dd, 3, 3)`). Daily seeds, replays and ghosts are therefore impossible until (a) the generator takes a seeded RNG and (b) challenge mode steps physics at a fixed rate. This is now an explicit Phase 0 exit gate, ahead of any leaderboard work.
2. **Events must not be PZGObstacles.** The obstacle pipeline is plist/atlas-art driven and placed by chunk templates; bolting events into it would make them unreproducible and unfair. Events are **kinematic actors** owned by a separate director (see 19.2), which asks the run director whether a hit is absorbed (shield / turbo / jetpack / grace / mercy) and only then reports a lethal hit.
3. **Spawn on demand, never in advance.** Platforms have gaps, so a long scripted stretch of flat road almost never exists. Events spawn each actor at the right screen edge only when the road there is flat, continuous and free of lethal obstacles, and skip or abort when it is not. This replaces "the event declares its own safe entry" with something that is enforced by code.
4. **Rival race needs a fair currency.** The player cannot out-run a rival (the world speed is the same). The race is therefore a meter: the rival drifts away, and the player closes the gap with coins, near misses, long jumps and air time. Tunables live in one place (`V2RoadEvents.h`).
5. **Scope trims.** Cross-platform backend, weekly leagues and real-time multiplayer stay out until the Phase 3 metrics exist (unchanged). Ambient background riders are kept (cheap, no collision) but share a cooldown with other gags.

### 19.2 Built so far (Phase 1 slice, in the repo)

| Piece | Where |
|---|---|
| Seeded RNG, shuffle bag (no repeats), bark book (never the same line within five), analytics port with ring buffer | `Source/V2/V2Core.*` |
| `RoadEventDirector`: scheduler (quiet opening, breathers, shuffle bag), kinematic actors that follow the road and hop gaps, hit-test, bubbles, rival marker, cleanup / world-shift hooks | `Source/V2/V2RoadEvents.*` |
| Events: **Dino Crossing** (telegraph sign, three small dinos, big straggler), **Pizza Panic** (delivery rider drops collectable boxes), **Cool Kid race** (overtake meter + HUD marker) | same |
| Ambient riders that pass you on a back lane (no collision) | same |
| Sprite atlas generator (28 frames: dino walk cycles, three rider variants with wheel animation, sign, pizza box, impact star) | `tools/make_v2_atlas.py` -> `Content/Images/HD/v2_events.png/.plist` (SpriteFrameCache) |
| Hooks into the run: host callbacks (absorb / lethal / award), world-shift listener, cleanup on restart, death, results and revive | `PZRunDirector`, `PZGGamefieldPlatforms` |

Verified on the iOS simulator with a QA bot that can read hazards: sign telegraph, dinos, delivery rider and boxes, rival pass and meter all render and complete; shield absorbs a dino hit and knocks it away; analytics lines show seed / start / hit / end.

### 19.3 Next steps, in order

1. Seed the platform generator and add a fixed-step challenge mode; golden replay test (Phase 0 gate).
2. Remaining events (Construction Chaos, Meteor Shower), second rival, perfect-landing feedback, localisation IDs for barks.
3. Daily seeded challenge and share cards (Phase 2 / 3 as written above).
4. Real-device frame-time and memory pass with events active; tune `V2RoadEvents.h` from playtest data.
