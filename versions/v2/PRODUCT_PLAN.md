# Dino Bike Escape — Version 2 Product and Development Plan

Status: implementation in progress; entertainment review added after commits `03e8cd6`–`4e6fdde`
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

### 19.3 Phase 0 / 1 progress (2026-11-20, second pass)

- **Seeded content.** `Source/PZRandom.*` is a dedicated content RNG; `PZGGamefieldPlatforms.cpp` and `PZGObstacle.cpp` draw only from it (cosmetic effects keep `std::rand`). `PZRunDirector::setChallengeSeed(seed)` makes a run reproducible; 0 = normal run (time-based seed).
- **Fixed-step mode.** With a challenge seed the runner simulates in exact 1/60 s slices (`PZGGamefieldBasicRunner::update`), independent of frame rate.
- **Determinism check (debug builds).** `[V2:sig] seed= dist= hash=` logs a hash of every content decision when the run passes 150 m. Simulator result: seed 12345 gave the same hash on 4 of 4 runs (including after another seed in between); seed 777 gave a different one. Not yet covered: input replay and cross-platform identity (iOS vs Android floats), which is what the golden-replay spike must prove.
- **Three more events.** *Construction Chaos* (sign, cone clusters, rolling paint drum, worker gag), *Meteor Shower* (growing shadow telegraph, falling meteor, smoking rock hazard, close-dodge bonus; rocks land at least 0.85 s ahead of the rider) and *Grandma Turbo* (slow entrance, rocket overtake, dropped coin trail). Six events now share one shuffle bag.
- **Smooth landing.** Crossing a gap in the air and landing with road ahead gives a squash-and-stretch on the rider and a small bonus; three in a row show PERFECT xN.
- **Art.** Atlas grew to 47 frames (grandma rider, meteor, rocket flame, worker, cone, drum, rock, coin, work sign).
- **Bugs found by the simulator and fixed:** a null dereference when Grandma left the screen, an event that ended early because its actor had been removed, actors spawned while iterating the actor list, and meteors that landed on the rider.

### 19.4 Daily challenge and share card (built, 2026-11-20)

- **Daily run** (`Source/V2/V2Daily.*`, DAILY RUN pill on the main menu): seed = hash of the UTC date + rules version (identical on every device); modifier of the day rotates through STANDARD, ONE LIFE (no free second chances, x1.5), EVENT RUSH (events twice as often, x1.25) and COIN DROUGHT (40% coins, x1.3). Score = `(distance + 2 x coins + 25 x events cleared) x modifier`, medals at 500 / 1,200 / 2,500.
- **Fairness rules:** one ranked attempt per UTC day (the first run; leaving or restarting it before the end still counts it), unlimited practice; no gadgets, no revives, no ads-for-advantage in daily runs; power-up orbs use their own RNG stream so they cannot change the level. Participation reward (20 / 40 / 80 / 150 coins by medal) is claimed once per date, so clock changes or repeats cannot pay twice. Results are kept in a local upload queue (`pendingUploads()`) for a future leaderboard adapter; rules version is stored with every score.
- **Share card** (`Source/V2/V2Share.*`): a 1200 x 630 card drawn with a RenderTexture (score, medal, modifier, the player's own bike, "CAN YOU BEAT THIS?", link) saved as PNG and handed to the native sheet (iOS `UIActivityViewController`, Android `ACTION_SEND` through a FileProvider, no storage permission). Sharing never grants a reward. Verified on the simulator: card PNG written and the iOS share sheet opens with the image and text. Android code is written but not built or run on a device.
- **Analytics events:** `daily_start`, `daily_end`, `share_prompt` (local ring buffer / log only).
- **Not built yet:** a leaderboard upload, friend challenge links with ghosts, and server-side score validation. Daily scores are only as trustworthy as the device until a server checks them, so do not attach prizes to ranks yet.

### 19.5 Replay test (built, 2026-11-20)

**What exists** (`Source/V2/V2Replay.*`): every fixed-step run (daily / challenge) records `{seed, rules version, input events by tick, a state checkpoint every 30 ticks, death tick}`. A recording can be played back into a fresh run: inputs are injected on the same ticks (live input is ignored while a replay plays) and each checkpoint is compared bit-for-bit (rider position / velocity, distance, speed, hash of every content random draw). The daily ranked run saves its replay to `replay_daily_<yyyymmdd>.v2r` in the writable path. A golden file `Content/golden_replay.v2r` (seed 7070, 1,496 ticks, 30 inputs, 49 checkpoints, recorded on the iOS simulator) ships with the app; **debug builds show a REPLAY TEST button** on the main menu (then press Play on the character screen) that plays it and shows REPLAY EXACT / REPLAY DRIFT with the numbers (also logged as `[V2:replay] RESULT`). Run it on an Android device to measure cross-platform drift: that part is still open.

**Result on the iOS simulator:** 6 of 6 replays bit-exact, including 3 with random 0-40 ms stalls injected into every frame, and the bundled file through the real button.

**What it took (each was a real bug or a hidden real-time dependency):**
1. Tick 0 now starts at GO. The world used to simulate for ~0.3 s of real time before the intro froze it, so the number of pre-GO ticks depended on frame timing. In fixed-step runs the world is now frozen from the first frame (one zero-time update builds the road) and the tick clock starts at GO.
2. The frozen / paused world no longer banks time in the fixed-step accumulator (it burst out as up to 6 slices at the first live frame).
3. The hit-stop is re-read for every slice, so a frame stall cannot run slices that a smooth frame rate would run after the freeze.
4. The road-event hit test used the rider sprite's bounding box, which follows the bike's real-time animation frame. It now uses a fixed box around the body position.
5. Undefined behaviour: `PZGObstacle` never initialised `dead`, `deleteReady`, `body`, `gameSpeed`, `counter`, `damage_counter` (only ever set to true / assigned on some paths). Same for `PZGCoin` and `PZGPlatfrom`. All members now have default values. This affects normal runs too (an obstacle could behave as dead or move on its own depending on heap contents).
6. The death tick is the first lethal physics signal, not the frame in which game over is processed (that is deferred in real time).
7. **Power-up orbs in reproducible runs are limited to magnet and shield.** Turbo, blast, jetpack, rocket and gun use real-time tweens, cinematics and hit-stops; replays that picked one up diverged (all replays of two different seeds split at the first pickup, ~11 s). If daily runs should offer more power-ups, those effects have to be made tick-based first.

**Limits:** not proven across platforms; one golden recording only (a bot run with a death at tick 1,496); revive, mercy rescues and any run that takes a real-time rescue are not replayable (a replay ends at the first death signal); replay files are not yet signed or validated by a server.

### 19.6 Next steps, in order

1. Run the golden replay on an Android device (and an older iPhone) and record the drift numbers here. If Android is not bit-exact, define a tolerance and checkpoint on quantised state instead.
2. Server-side validation: replay a submitted `{seed, inputs}` headless to check the claimed score before it enters a leaderboard.
3. Leaderboard: Game Center daily board first, Play Games on Android after a bridge spike.
4. Friend challenge link + ghost playback from a replay file; make turbo / jetpack / rocket tick-based if they should appear in daily runs.
5. Real-device frame-time / memory pass with events active; tune from playtests.

## 20. Entertainment and engagement review (2026-11-20)

This section reviews gameplay only. Graphics quality is deliberately excluded.

### 20.1 Verdict

**Will the current v2 work improve engagement? Probably yes, but it is not yet enough to prove strong retention.**

The old run was mechanically repetitive. The new event scheduler, rivals, collection objectives, daily seed, medals, and share result create anticipation and give the player more reasons to start another run. That is a real improvement. The implementation also protects fairness better than a typical first pass.

The weakness is depth. Six event names do not currently equal six different gameplay experiences:

| Event | Current entertainment value | Honest assessment |
|---|---|---|
| Dino Crossing | Medium | Good telegraph and escalating straggler, but still mainly four themed jump obstacles. |
| Pizza Panic | High | Best event so far: a character, moving source, collection target, success threshold, and payoff. |
| Cool Kid Race | High | Strongest skill remix: normal actions now feed an understandable race meter. Needs variants and persistent rivalry. |
| Construction Chaos | Medium-low | Readable variety, but cones and barrel still reduce to jump timing; the worker is mostly decoration. |
| Meteor Shower | Medium | Good anticipation and close-dodge reward. Four similar strikes will become predictable quickly. |
| Grandma Turbo | Medium-high initially | Funny entrance and coin payoff, but little meaningful competition; novelty will fade after several encounters. |

Current entertainment score, based on the code and existing simulator verification:

- Technical event foundation: **8/10**.
- Immediate novelty for returning players: **7/10**.
- Meaningful decisions inside a run: **4/10**.
- Variety after a week: **4/10**.
- Competitive/social loop: **3/10** because sharing exists but challenges, ghosts, and leaderboard submission do not.
- Measurability: **3/10** because analytics currently ends at a local log/ring buffer.

This means the direction is correct, but store marketing should not imply that the game is already multiplayer or socially competitive.

### 20.2 What was done properly

#### Event pacing and safety

- Quiet opening, one event at a time, post-event breathers, and a shuffle bag are the correct foundations.
- Events check for continuous, flat, obstacle-free road and skip unsafe spawns instead of forcing a broken setup.
- Signs, shadows, dialogue, and introductions give the player advance warning.
- Kinematic traffic is the right choice; decorative riders do not need full bike physics.
- Fixed rider hit boxes avoid animation-frame-dependent collisions.
- Events clean up on death, restart, results, revive/world shift, pause-like control locks, and hazard clearing.
- Shield, grace, turbo, jetpack, rocket, mercy, and revive interactions were considered instead of being left as side effects.

#### Event identity

- Pizza Panic and Cool Kid Race change the purpose of normal jumping instead of only replacing obstacle art.
- Meteor shadows and a close-dodge bonus reward observation and bravery.
- Grandma's slow setup followed by absurd acceleration has a clear comedic setup and payoff.
- Ambient riders make the world feel occupied without adding unfair collisions.
- Short contextual barks are better than modal dialogue and the recent-line history reduces immediate repetition.

#### Skill feedback

- Smooth-landing detection and streak feedback make an existing action feel more intentional.
- Near misses, air time, coins, and clean jumps contributing to the rival race is a smart reuse of existing mechanics.
- Event success is optional; losing an event generally does not end the entire run unless the player hits an actual hazard.

#### Daily foundation

- Seeded content, fixed-step simulation, separate RNG streams, versioned rules, offline result queue, and replay checkpoints are good engineering decisions.
- Daily mode removes bought gadgets and ad-based advantage.
- Practice remains available after the ranked attempt.
- Share cards use native sharing and do not reward spam.

#### Regression work

- Initialising previously undefined obstacle, coin, and platform state was an important normal-game fix, not just replay work.
- Golden replay tests with artificial frame stalls are valuable evidence that timing changes did not silently alter a run on iOS.

### 20.3 Initial gaps identified before the latest implementation

This is the original review backlog and is retained for decision history. Several items were subsequently implemented. Use the current audit in section 20.7 and the ordered work in sections 20.8–20.10 as the authoritative status and execution plan.

#### P0 — Fix before presenting v2 as a major entertainment update

1. **Add real choices, not more obstacle skins.** At least three events need a visible risk/reward decision. Examples:
   - Dino Crossing: safe jump over the small dinos or stay low and collect a risky letter trail before the big straggler.
   - Construction: safe cone route or break through marked barrels for a shortcut bonus.
   - Meteor: follow the safe edge or bait a meteor into destroying a marked obstacle for a larger reward.

2. **Give every event two or three variants.** The shuffle bag prevents immediate repeats but not learned repetition. Six fixed scripts will become familiar in one or two sessions. Variants should change timing and objective, not merely spawn count.

3. **Add difficulty tiers.** Each event needs `intro`, `standard`, and `expert` parameters selected from distance and demonstrated skill. Never make telegraphs shorter than the tested reaction-time floor. Difficulty should change patterns, speed, and optional reward lines.

4. **Reconsider one blind ranked attempt.** The first run is partly a knowledge test because event order and timing are unknown. One mistake consumes the only ranked result and can create anxiety instead of engagement. Recommended launch rule: **best of three ranked attempts**, then unlimited unranked practice. Reward only once. If one attempt is retained, call it “Daily First Ride,” not a serious competition.

5. **Make daily modifiers change decisions.** Current modifiers mostly change event frequency, coin density, revive availability, or a score multiplier. Add rule modifiers such as:
   - Perfect Landing Day: landing streak drives score.
   - Pizza Express: every event drops cargo; missed cargo reduces a delivery meter.
   - Dino Tag: stay close to a rival without passing too early.
   - Smash Route: marked hazards are targets while unmarked hazards must be avoided.

6. **Connect analytics to a real provider before tuning.** Local logs cannot answer whether events improve retention. At minimum send run start/end, event start/outcome, skip reason, hit reason, daily attempt, modifier, share tap, and frame-time bucket. Distinguish `success`, `failed`, `skipped_no_road`, `aborted_death`, and `cancelled_scene`; currently a technical skip can look like player failure.

7. **Playtest scoring and medals.** `25 × eventsCleared` is small next to distance, while event coin awards are counted again through `2 × coins`. This can make event value confusing and may overweight coin-paying events. Show a score breakdown and tune from recorded runs. Medal thresholds should be validated per modifier because Coin Drought and Event Rush do not produce equivalent score distributions.

8. **Move v2 tuning out of hard-coded constants.** First-event time, breathers, race drift/gain, event counts, waits, collection thresholds, rewards, daily score weights, medals, and modifier values should use a versioned v2 configuration section with safe defaults and validation. This is required for cheap playtest tuning and future reskins.

#### P1 — Needed for lasting entertainment

9. **Add a chapter climax.** Implement the T-Rex escape as a 30–45 second set piece after several normal events. A run needs a memorable peak, not an endless flat sequence of equally weighted encounters.

10. **Give rivals memory.** Track encounters and wins per rival, change their bark after losses, unlock a rematch milestone, and let the player see “2–1 vs Cool Kid.” This creates a relationship without building multiplayer.

11. **Make Grandma interactive.** Let the player draft behind her rocket for a temporary speed/score charge, choose to follow her dangerous coin trail, or beat her to a checkpoint. The current collect-six-coins requirement is not enough after the joke is known.

12. **Make Construction a timing puzzle.** The worker should flip a sign or activate a lift that changes the route. The player reads the worker animation and chooses an early or late jump. Otherwise it remains ordinary obstacles with construction names.

13. **Vary Meteor outcomes.** A meteor should sometimes open a shortcut, destroy an obstacle, leave a temporary ramp, or trigger a chain reaction. Four shadow → rock repetitions are readable but shallow.

14. **Add micro-challenges inside normal runs.** Start with only three: three smooth landings, collect five event objects, and clear an event without a power-up. One at a time, 10–25 seconds, no run-ending penalty. These create short goals between major events.

15. **Improve ambient gags.** Current ambient riders mostly pass and sometimes say “BEEP BEEP.” Add a small pool of non-blocking setup/payoff behaviors: wave then wobble, lose a harmless prop, get startled by a horn, or reappear later with the prop fixed. Keep the 20–35 second cooldown.

16. **Make event completion clear.** Show the objective at entry (`PIZZA 0/5`, `CATCH COOL KID`, `DODGE 0/4`) and a compact result at exit. Dialogue alone is not reliable instruction during movement.

17. **Restore power-up variety only after tick-based conversion.** Limiting deterministic runs to magnet and shield is correct for replay safety, but daily runs may feel less exciting than normal runs. Convert one high-fantasy power-up—preferably rocket—to tick-based behavior and add a Rocket Rush daily only after replay tests remain exact.

#### P2 — Social follow-up, not the next task

18. Complete Game Center daily submission and the Android Play Games bridge.
19. Turn the shared URL into a real challenge token/deep link; the current card is promotion, not a friend challenge.
20. Render a non-colliding ghost from the recorded input/replay only after Android replay drift is measured.
21. Do not begin live multiplayer until asynchronous challenge metrics pass the gates in section 9.

### 20.4 Required entertainment loop

Every event should satisfy this five-part test:

1. **Notice:** the player understands that something different is coming.
2. **Choice:** the player can choose a safe action or a harder, more rewarding action.
3. **Skill:** timing or route execution changes the result.
4. **Reaction:** a rival/world character visibly reacts to the result.
5. **Consequence:** the run changes briefly—bonus, shortcut, rivalry state, combo, or later callback.

Current coverage:

| Event | Notice | Choice | Skill | Reaction | Consequence |
|---|---:|---:|---:|---:|---:|
| Dino Crossing | Yes | No | Yes | Partial | Coins only |
| Pizza Panic | Yes | Partial | Yes | Yes | Score/reward |
| Cool Kid Race | Yes | Yes | Yes | Yes | Score/reward |
| Construction | Yes | No | Yes | Partial | Coins only |
| Meteor | Yes | Partial | Yes | No | Close-dodge reward |
| Grandma | Yes | Partial | Yes | Yes | Coin reward |

Before calling an event “finished,” it should pass at least four columns, including **Choice** or a strong event-specific objective.

### 20.5 Previous gameplay sprint checkpoint

This was the recommended implementation sprint. Its current status is:

| Item | Status | Remaining proof/work |
|---|---|---|
| Objective HUD owned by `RoadEventDirector` | Implemented | Verify all events, tiers, aspect ratios, and cleanup paths |
| Two variants for Dino, Construction, and Meteor | Implemented | Visually play through all six patterns and balance them |
| Explicit safe/risky play in those events | Partial | Bonuses and optional lines exist; route choices remain shallow |
| Best-of-three ranked daily with attempts remaining | Implemented | Add automated rule, rollover, reward, and migration tests |
| Three micro-challenges | Implemented | Validate frequency, clarity, conflict handling, and reward balance |
| Versioned `v2` tuning in `CompleteGameInfo.plist` | Implemented | Verify all values, defaults, and version migration |
| Outcome/skip analytics | Implemented locally | Connect a production provider and add run/performance/share events |
| Ten fresh-player observation sessions | Not started | Run without coaching and record comprehension and voluntary retries |

The original sprint exit criteria remain valid:

- At least 8/10 new testers can state each event objective after seeing it once.
- At least 7/10 notice and intentionally take one risk route.
- No tester reports an untelegraphed event death.
- Each tester sees at least four distinct event variants in a 10-minute session.
- Event failure does not cause more than a short interruption or obscure the next hazard.
- Analytics can compare death, completion, skip, and replay rates per event/variant.
- Testers voluntarily start another daily attempt; do not count a prompted retry.

### 20.6 Release claim

After the current code, it is fair to say:

> “Runs now feature six road events, passing riders, rival races, daily seeded challenges, medals, and shareable results.”

It is **not** yet fair to say:

- “Multiplayer” or “race your friends.”
- “Every run tells a different story.”
- “Deep daily competition.”
- “Live events,” unless a real scheduled/configured event is active.

The product becomes genuinely entertaining when the events demand different thinking, not merely display different actors.

### 20.7 Current implementation audit — 2026-11-21

This section supersedes the earlier sprint-status notes. Percentages are engineering estimates, not retention data.

| Area | Estimated complete | Honest assessment |
|---|---:|---|
| Event framework, cleanup, and fairness | 90% | Strong foundation; requires complete device play-throughs |
| Event content and variety | 65% | Three events have variants; the other repeatable events remain shallow |
| Daily challenge | 80% | Feature-complete enough for testing, but scoring, rewards, migration, and edge cases are not validated |
| Comedy and world activity | 70% | Passing riders and callback gags help; the content pool will still repeat |
| Social and friend challenges | 25% | Native sharing exists, but no real challenge token, ghost, or cross-platform leaderboard |
| Analytics and validation | 35% | Local structured logging exists; production measurement and player evidence do not |
| Cross-platform release QA | 30% | Changed source compiles, but Android and real-device behavior are not proven |

Overall estimates:

- **Gameplay feature implementation:** approximately 75%.
- **Verified and balanced gameplay:** approximately 50%.
- **Full v2 plan, including social follow-up:** approximately 58%.

#### Implemented correctly in code

- Versioned `V2Config` reads the `v2` plist section, applies defaults, and clamps unsafe values.
- Daily mode offers best-of-three ranked attempts, keeps the best result, allows unlimited practice, and pays only the initial medal or a later medal upgrade.
- Six daily modifiers exist: Standard, One Life, Event Rush, Coin Drought, Perfect Landing, and Pizza Express.
- Daily results show a score breakdown, and event-generated coins are excluded from normal coin scoring to prevent double counting.
- Event results distinguish `success`, `failed`, `skipped_no_road`, `aborted_death`, and `cancelled_scene`.
- Intro, standard, and expert tiers are selected from run progress.
- Dino, Construction, and Meteor have two pattern variants each.
- Objective, risk-line, close-dodge, and compact result feedback exist.
- Three micro-challenges use existing gameplay signals: smooth landings, event-object collection, and clearing the next event without a power-up.
- Cool Kid stores wins and losses, changes barks, shows the record, and awards every third win.
- The T-Rex climax is scheduled separately from the normal event bag after enough successful events.
- Ambient riders can wobble, lose a harmless cone, react to the player, and later return with the cone fixed.
- Lucky meteors, road-event streak bonuses, golden pizza slices, and Grandma jackpot coins add small surprise rewards.
- Local JSONL analytics rotate at 256 KB and record event, daily, risk, and micro-challenge activity.

Compilation and configuration checks passed for the changed V2 gameplay sources and `CompleteGameInfo.plist`. That proves basic integration, not entertainment quality or release safety.

#### Implemented but still shallow

- Expert tiers mostly increase speed and timing pressure. They need event-specific decisions or patterns, not only tighter numbers.
- Pizza Panic, Cool Kid Race, and Grandma have no equivalent two-variant structure yet.
- The T-Rex sequence has a memorable presentation but relies heavily on ten telegraphed rock dodges. Its chapters need mechanically different phases.
- Grandma's high jackpot coins add jump timing, but the planned drafting and route decision are missing.
- Construction has a flagger but not the full lift/sign timing puzzle.
- Meteor can turn one strike into a coin arc but does not open shortcuts, create ramps, or trigger chain reactions.
- Rival memory is limited to Cool Kid; there is no broader rival roster or meaningful rivalry progression.
- Local analytics cannot measure retention, conversion, or population-level event balance.

#### Known defect

The T-Rex payoff dialogue is attached to the T-Rex actor immediately before that actor is removed. On the next update, the dialogue can lose its owner and fade almost instantly. Put the payoff on a persistent HUD/world layer or delay actor removal long enough for the line to be read.

### 20.8 Release-blocking validation plan (P0)

Complete these tasks in order. Do not call v2 release-ready until all exit criteria pass.

1. **Full event play-through matrix**
   - Play every variant at intro, standard, and expert tiers.
   - Complete and deliberately fail every objective.
   - Test shield, turbo, jetpack, rocket, death, revive, pause/control lock, restart, and scene exit during each event.
   - Confirm telegraphs, collision windows, objective text, result text, cleanup, and normal road resumption.

2. **Repair and validate the T-Rex climax**
   - Fix the payoff-dialogue lifetime defect.
   - Finish the complete 10-stomp sequence on success and failure paths.
   - Confirm cleanup on death, restart, and scene change.
   - Replace repeated rocks with at least three mechanically distinct phases before describing it as a complete set piece.

3. **Restore deterministic replay evidence**
   - Re-record `golden_replay.v2r` for rules version 2.
   - Run it with normal frames and injected stalls.
   - Verify identical checkpoints on iOS and Android before enabling ghosts or competitive replay claims.

4. **Automate daily-mode rules**
   - Test three ranked attempts, unlimited practice, best-score retention, medal upgrades, reward-once behavior, forfeits, and date rollover.
   - Test all six modifiers and per-modifier medal thresholds.
   - Test migration from existing saved daily keys so an update cannot reset attempts or duplicate rewards.

5. **Balance using session evidence**
   - Record objective success, event failure, death, close dodge, reward, score source, medal, retry, and run duration.
   - Tune event rewards and medal thresholds from observed distributions, not developer-only runs.
   - Treat each modifier separately; Event Rush, Coin Drought, and Perfect Landing should not share assumed score curves.

6. **Device and layout QA**
   - Test low-end target devices, long runs, app background/foreground, memory pressure, and audio interruption.
   - Test objective and micro-challenge plates on supported aspect ratios and safe areas.
   - Add frame-time buckets to analytics and set an explicit performance budget.

7. **Fresh-player observation**
   - Run at least ten sessions without explaining events beforehand.
   - Capture whether players understand each objective, see the risk route, understand failure, and voluntarily retry.
   - Use the sprint exit criteria in section 20.5; do not substitute team opinion for player evidence.

### 20.9 Entertainment-depth sprint (P1)

Start only after the P0 validation loop is functioning.

1. Add two meaningful variants to Pizza Panic, Cool Kid Race, and Grandma. A variant must change the decision or timing pattern, not only the number of objects.
2. Give expert tiers new route or objective patterns while preserving the tested telegraph floor.
3. Split the T-Rex climax into distinct phases, for example: pursuit, bite requiring duck/jump timing, collapsing road choice, and a final ramp escape.
4. Add Grandma drafting with a visible charge meter and a safe-versus-risky decision.
5. Let Meteor open a shortcut, leave a temporary ramp, or destroy a marked obstacle through a chain reaction.
6. Complete Construction's animation-reading puzzle with an early/late route choice.
7. Expand rival progression only after Cool Kid engagement is proven; do not add a large roster that repeats the same race logic.

Entertainment-sprint exit criteria:

- Every repeatable event passes at least four parts of the Notice → Choice → Skill → Reaction → Consequence test.
- Every repeatable event has at least two meaningfully different patterns.
- Expert play changes decisions instead of merely accelerating the same script.
- The T-Rex phases are recognizable from player action without relying on instruction text.
- No new mechanic breaks normal runs, daily scoring, deterministic replay, cleanup, or power-up interactions.

### 20.10 Social follow-up (P2)

Build social features in increasing cost order:

1. Complete Game Center daily submission and the Android Play Games bridge.
2. Put a versioned challenge token in the shared URL with date, seed, rules version, and modifier.
3. Open the token through a deep link and show a clear friend-challenge entry screen.
4. Add a non-colliding replay ghost only after cross-platform deterministic tests pass.
5. Measure challenge creation, challenge open, completion, retry, and sender/receiver return rate.
6. Consider real-time multiplayer only if asynchronous challenges produce repeat play and the team can support authoritative networking, matchmaking, latency handling, cheating controls, and live operations.

Native share cards alone are promotion, not multiplayer and not a friend challenge. Do not market them as such.

### 20.11 Dependency and architecture guidance

- Keep gameplay rules, presentation, persistence, platform services, and analytics behind separate interfaces. `RoadEventDirector` should not directly own platform leaderboard or share implementations.
- Keep authoritative tuning in the versioned `v2` configuration and include the rules/config version in replay and challenge data.
- Reuse the existing Axmol scheduler, action system, particle system, and platform share/game-service adapters before adding dependencies.
- For real multiplayer, evaluate a maintained service such as Nakama, Photon, or Epic Online Services before writing matchmaking and relay infrastructure. This evaluation is future work, not permission to add an SDK now.
- For production analytics, use a thin internal event interface so Firebase Analytics, GameAnalytics, or another provider can be selected without coupling gameplay code to its SDK.
- Every external SDK must be checked for binary size, supported platforms, privacy/consent requirements, offline behavior, maintenance status, and impact on deterministic gameplay.

### 20.12 Next execution checkpoint

The next milestone is **Gameplay Validation Complete**, not “more features.” It is complete only when:

- all event/tier/outcome combinations have a recorded QA result;
- the T-Rex defect and complete sequence are verified;
- rules-v2 golden replay passes on iOS and Android;
- daily rule tests pass, including save migration;
- ten fresh-player sessions meet the section 20.5 comprehension and retry gates;
- scoring and medal changes are supported by recorded session data;
- no regression is found in ordinary non-v2 runs.

After this checkpoint, recalculate the percentages in section 20.7 and choose either the entertainment-depth sprint or the social follow-up from evidence. Do not run both simultaneously.

## 21. Visual entertainment, screens, and cinematic presentation plan

### 21.1 Goal and non-goals

The visual update should make the game feel like a professionally directed arcade bike escape, not a collection of unrelated effects. The desired fantasy is:

> Ride fast through a dangerous, cinematic sewer world where rival bikers, distant dinosaurs, environmental surprises, and short character moments make each run feel like an episode.

This pass must improve anticipation, personality, spectacle, and visual clarity without changing the proven bike, platform, coin, or core HUD language.

Non-goals for this pass:

- Do not redesign bike physics or normal controls.
- Do not turn decorative dinosaurs into hidden hazards.
- Do not add dialogue screens during active gameplay.
- Do not replace the bike, coins, platforms, parallax backgrounds, or core HUD merely to make everything new.
- Do not add real-time multiplayer before the asynchronous plan in section 20.10 is validated.
- Do not generate final assets before the style baseline, collision baselines, asset manifest, and one obstacle vertical slice are approved.

### 21.2 Research summary and product lessons

The useful lesson from successful bike games is not “add more graphics.” They combine a readable core action with visible identity, short goals, competition, and content variety:

| Reference | Relevant product pattern | Apply here | Do not copy |
|---|---|---|---|
| [Bike Race](https://play.google.com/store/apps/details?id=com.topfreegames.bikeracefreeworld) | Simple controls, crazy worlds, stunts, many bikes, offline play, friend challenges, and multiplayer are presented as one clear fantasy | Keep one-tap readability; make worlds and rival moments memorable; later use challenge links and ghosts | Its exact tracks, bikes, UI, or visual identity |
| [Hill Climb Racing 2](https://play.google.com/store/apps/details?id=com.fingersoft.hcr2) | Ranked cups, teams, events, leaderboards, and customization give repeated runs context | Frame daily runs and rivals as visible competition; make rewards and records legible | Large metagame scope before the run itself is validated |
| [Mad Skills Motocross 3](https://apps.apple.com/us/app/mad-skills-motocross-3/id1508275130) | Strong side-view physics, many designed tracks, recurring content, multiplayer events, and deep visual customization | Protect the side-view silhouette and landing readability; later test cosmetic rider identity | Realistic simulation complexity or hundreds of tracks now |
| [Trials Rising](https://www.ubisoft.com/en-us/games/trials-rising) | Over-the-top locations, spectacle, character/bike customization, progression, competition, and optional ghosts | Use large background set pieces, expressive rivals, short camera reveals, and optional future ghosts | Long forced cinematics, visual clutter, or expensive 3D scope |
| [Bike Unchained 3](https://play.google.com/store/apps/details?id=com.redbull.bike.downhill.racing) | Downhill and jump modes are marketed as distinct fantasies, supported by bike/rider presentation | Give each event a distinct visual promise and result identity | Adding modes that do not change decisions |

Research conclusion:

- Spectacle attracts attention, but recognizable goals and rivalry create repeat play.
- The player must read the road, bike posture, obstacle silhouette, and landing surface before noticing decoration.
- Background dinosaurs are appropriate because they strengthen the game's unique identity without creating another rules system.
- Dialogue should establish who the rival is and why this run matters, then get out of the way.
- Fast restart is more valuable than a long cinematic. Repeated presentation must be skippable or abbreviated.
- Ghosts and social comparison are useful later, but only after replay determinism and platform services work.

### 21.3 Current visual audit

#### Preserve as primary style references

Use these existing elements as the art-direction reference set for all future generated graphics:

1. **Bike and rider:** crisp side silhouette, realistic material highlights, readable wheels, dark gear, controlled color accent.
2. **Platforms and ramps:** photo-painted dirt/wood/stone materials, visible top surface, strong ground contact, believable wear.
3. **Parallax backgrounds:** dark blue sewer/canal atmosphere, naturalistic light, depth, haze, and subdued distant contrast.
4. **Coins:** warm gold, high contrast, instantly collectible, with a clean rim and strong highlight.
5. **HUD:** dark navy/metal framing, warm orange/gold action accents, large readable arcade typography.

These are the visual source of truth. New art should look as though it shipped with them.

#### Replace or revise

- Legacy `Obstacles_*` art is often faded, muddy, inconsistently lit, and weak against the road.
- The current `v2_events.png` did not follow the established bike, platform, parallax-background, HUD, or main-game style. Its bright flat-vector characters and props conflict with the semi-real painted bike and sewer world. It is implementation placeholder art only and **the complete atlas must be replaced** before the visual-entertainment update is considered finished. Do not preserve its rendering style when generating replacements; preserve only stable frame identities, gameplay roles, dimensions/anchors where required, and initially the animation timing.
- Some interface art has an older glossy-button treatment. Keep the recognizable color language but simplify future panels and improve spacing rather than mixing additional styles.
- Background images are atmospheric but repeated layers can feel static. Add compatible depth motion and scenic events instead of replacing them wholesale.

#### Visual style definition

Call the target style **cinematic painted arcade realism**:

- side-on 2D readability;
- realistic materials and lighting simplified into strong silhouettes;
- dark cool environment with warm gameplay accents;
- exaggerated scale and comedy, but not flat children's-vector art;
- slightly stylized proportions for characters and dinosaurs;
- restrained texture detail at gameplay size;
- no photoreal human faces pasted onto illustrated bodies;
- no thin outlines, tiny props, or low-contrast transparent hazards.

### 21.4 Visual priority rules

Every frame must preserve this attention order:

1. Immediate landing surface and lethal obstacle.
2. Player bike posture and jump arc.
3. Event objective or rival position.
4. Collectibles and optional risk route.
5. HUD state.
6. Background spectacle and comedy.

If a dinosaur, particle effect, dialogue element, foreground prop, or camera move competes with levels 1–3, reduce or remove it.

Use the following visual languages consistently:

| Meaning | Visual treatment |
|---|---|
| Lethal/physical hazard | Solid silhouette, high local contrast, grounded shadow, warm warning accent when needed |
| Collectible/reward | Gold light, small sparkle, upward motion, no red warning shape |
| Decorative background | Lower contrast and saturation, atmospheric blue/green tint, slower parallax, no collision shadow on the road |
| Objective/rival | Character accent color plus compact label or marker |
| Cinematic-only element | Letterbox or portrait framing; never visually identical to active gameplay |

### 21.5 Decorative dinosaur system

Large dinosaurs should become the game's signature background entertainment. They must remain cosmetic and cannot affect physics, scoring, event RNG, replay checkpoints, or road generation.

#### Scenic beats

Start with five reusable beats:

1. **Distant walker:** a huge long-neck silhouette crosses behind buildings or trees over 8–12 seconds.
2. **Wall peek:** a T-Rex head rises behind a far wall, watches the rider, then slowly lowers.
3. **Bridge crossing:** a dinosaur runs across a distant bridge while dust falls behind it.
4. **Tunnel eye:** a large eye opens in a dark background tunnel, follows the rider briefly, then blinks.
5. **Tiny chase callback:** several tiny dinosaurs chase a much larger harmless rider silhouette in the far background.

The first production slice should implement only Distant Walker and Wall Peek. Add the others after readability and performance tests.

#### Placement and behavior rules

- Place dinosaurs behind gameplay platforms and never between the rider and a hazard.
- Keep their feet, body, and shadows in a clearly distant plane; do not let them appear grounded on the playable road.
- Apply atmospheric tint and 25–55% less contrast than gameplay actors.
- Avoid orange/red warning flashes, meteor shadows, exclamation signs, or other hazard vocabulary.
- Allow at most one large scenic beat at a time.
- Default cadence: first eligible beat after 25–40 seconds, then a cosmetic seeded cooldown of 60–120 seconds.
- Suppress scenic beats during the first tutorial, dialogue transition, T-Rex climax, death, revive, blast cinematic, and dense road event telegraphs.
- A scenic beat may react to a normal gameplay signal—for example looking toward a rocket—but must not consume gameplay RNG or change timing.
- Provide `Off`, `Reduced`, and `Full` spectacle levels. Reduced mode uses static or fade-only versions.
- Repeat protection: do not show the same beat twice consecutively and do not show more than two large beats in a five-minute run.

### 21.6 Start-of-run dialogue screens

Dialogue screens exist only before the run begins. Mid-run personality remains limited to short non-modal barks. This protects pace and road readability.

#### When to show dialogue

- First encounter with a named rival.
- First daily run of the day.
- Rival rematch after a loss or every third rivalry milestone.
- First time entering a new chapter or seeing the T-Rex climax setup.
- Rare story callback unlocked by previous run outcomes.

Do not show a full dialogue screen on every normal restart. After the first viewing, use a one-line abbreviated card or skip it entirely.

#### Screen format

- Duration target: 3–7 seconds when allowed to auto-advance.
- Two to four short lines total; maximum eight words per line where possible.
- One speaker at a time, with clear name and color identity.
- Tap anywhere to advance; hold or dedicated button to skip the sequence.
- Remember viewed dialogue by stable ID and version.
- Never hide the Skip affordance on repeat viewing.
- Freeze the world before the run begins; do not create physics bodies or advance fixed simulation ticks behind the screen.
- Restore camera, HUD visibility, input lock, audio ducking, and scene transform through one cleanup path on complete, skip, app interruption, or scene exit.

#### Professional close-up direction

- Use waist-up or helmet-and-shoulders portraits, not scaled gameplay sprites.
- Frame the player on the left facing inward and the rival on the right facing inward.
- Use a subtle sewer/environment strip behind the portraits, heavily defocused or simplified so faces dominate.
- Give each named rider four expressions: neutral, confident, surprised, and frustrated.
- Preserve helmet, jacket, gloves, and bike color from the gameplay identity.
- Use a slow 2–3% push-in on the current speaker only in Full motion mode; Reduced mode uses a crossfade and highlight.
- Use eye-line, pose, and panel angle to communicate attitude. Do not rely on large text or emoji.
- Dialogue panel uses the existing dark navy/metal HUD family with warm gold focus accents.
- Text must remain readable over every portrait; use an opaque panel or controlled gradient rather than shadow alone.

#### Initial dialogue set

Keep writing short, competitive, and playful:

| Trigger | Rider | Rival | Purpose |
|---|---|---|---|
| First Cool Kid encounter | “Road looks busy.” | “Only behind me.” | Introduce rivalry and confidence |
| Rematch after player loss | “Same road. Different finish.” | “I saved you second place.” | Acknowledge memory |
| Rematch after player lead | “Keeping up?” | “I was studying your mistakes.” | Let rival react to player success |
| First T-Rex chapter | “Tell me that was thunder.” | “Thunder doesn't have teeth.” | Set up spectacle without explaining mechanics |
| Daily challenge | “One route. Three real tries.” | “Then make the first one count.” | Explain stakes with character |

Final writing requires localization review. Store text by dialogue ID; do not bake text into portrait art.

### 21.7 Camera and motion direction

The project already has `cameraTo`, `cameraPunch`, neutral reset, cinematic bars, HUD staging, start ride-in, blast focus, and death focus. Extend that system instead of adding competing scene transforms.

#### Camera shot library

| Shot | Use | Maximum behavior |
|---|---|---|
| Establishing wide | Before run/dialogue | 0.5–0.8 second ease showing road, rider, and background identity |
| Rider close-up | Intro or rival entrance while world is frozen | 1.20–1.35 scale, rider kept off-center to preserve direction of travel |
| Rival two-shot | Start dialogue | Portrait screen or frozen-world composition; never during live hazards |
| Scenic reveal | Safe road only | Small pan/zoom toward a distant dinosaur, then reset before control returns |
| Landing punch | Existing skill feedback | Brief 1.03–1.08 scale pulse; no sustained shake |
| Climax reveal | T-Rex start | One controlled wide-to-close transition with explicit input lock and cleanup |
| Death focus | Existing failure feedback | Preserve cause and rider in frame; return quickly to result/restart |

Rules:

- Gameplay camera changes must never hide the next landing surface or move touch targets.
- Do not rotate the world camera.
- Do not stack shake, zoom, parallax acceleration, flash, and letterboxing on one beat.
- Camera arbitration must allow only one owner: dialogue, intro, climax, blast, death, revive, or gameplay punch.
- Every cinematic path must reset scale and position exactly to neutral.
- Reduced Motion disables scenic pans, portrait push-ins, large zooms, sustained shake, and exaggerated parallax; replace them with fades, tint, and audio/haptic cues. This follows Apple's guidance to reduce automatic zooming, scaling, and peripheral/parallax motion for motion-sensitive players: [Motion guidance](https://developer.apple.com/design/human-interface-guidelines/motion) and [Reduced Motion criteria](https://developer.apple.com/help/app-store-connect/manage-app-accessibility/reduced-motion-evaluation-criteria).

### 21.8 Graphics production instructions for future asset generation

These instructions are for the later graphics-generation phase. Do not generate final assets until the relevant requirement and acceptance test below is approved.

#### Required reference package

For every generation task, provide crops of:

- current blue/red bike and rider;
- current dirt and wood ramps/platforms;
- one full sewer background with foreground road;
- current coin and one HUD panel/button;
- the exact old obstacle/event sprite being replaced;
- a gameplay screenshot showing expected on-screen size.

Do not use `v2_events.png` as the primary style reference. It is a layout/identity reference only until reskinned.

#### General generation brief

Use the following base instruction, then add the asset-specific requirement:

> Create a side-view 2D game asset in cinematic painted arcade realism, matching the supplied dark sewer background, realistic dirt-bike rider, worn platforms, warm gold coins, and navy/orange HUD. Preserve a bold readable silhouette at mobile gameplay size, realistic material texture with simplified detail, cool environmental fill light and a controlled warm upper-left key light. Do not use flat vector art, thick cartoon outlines, photoreal pasted faces, text, logos, excessive saturation, bloom, or a baked background.

#### Obstacle generation rules

- Transparent background and at least 8% clear padding around the visible object.
- Strict side view or the same three-quarter angle as the source obstacle; never change perspective between frames.
- One clear ground-contact plane and a separate soft contact-shadow sprite when needed.
- Damage/death variants preserve mass, orientation, and recognizable silhouette.
- Avoid transparent smoke or dust inside the base sprite; effects belong in separate sprites/particles.
- Do not invent spikes or threatening shapes on nonlethal decoration.
- Deliver a high-resolution master plus the exact target-size export.

#### Dialogue portrait generation rules

- Waist-up portrait, transparent background, inward-facing composition.
- Same helmet, gear, body proportions, and accent color across every expression.
- Four expressions generated from one approved neutral master, not four independent character redesigns.
- Hands and handlebars appear only when composition requires them; avoid malformed partial bikes.
- Lighting direction and edge treatment must match across player and rival portraits.
- No text or speech bubbles in the image.

#### Dinosaur generation rules

- Large readable silhouette first; detail must survive atmospheric tint and scale reduction.
- Anatomically believable but slightly exaggerated for humor and clarity.
- Neutral/cool colors so dinosaurs do not look like rewards or warnings.
- Separate body parts only when an animation plan exists; otherwise use a stable short frame sequence.
- Separate dust, eye blink, and shadow effects from the body sprites.
- Produce a scale sheet showing the dinosaur behind the existing bike/platform before approval.

#### Acceptance checklist for every generated asset

- Matches the five approved reference categories.
- Reads at 100%, 75%, and 50% target size.
- Has clean alpha with no white/dark fringe.
- Uses the approved anchor, source canvas, and light direction.
- Does not alter physics, platform joins, telegraph timing, or UI layout.
- Fits the atlas budget and passes on-device texture/memory review.
- Includes source prompt, reference list, seed/version when available, master image, and final export so it can be reproduced.

### 21.9 Obstacle and hurdle refresh — first production priority

The faded hurdles are the first art task because they directly affect moment-to-moment quality and hazard readability.

#### Requirement O1 — inventory and baseline

- Build an asset manifest for every `Obstacles_*` frame: frame name, gameplay ID, lethal/nonlethal, animation, source size, trimmed size, anchor, scale, offset, collision vertices, death art, and scenes/modes that use it.
- Capture each hurdle in gameplay at normal and maximum speed.
- Record current collision outlines over the sprite and save deterministic replay checkpoints before replacement.
- Grade each asset for silhouette, contrast, material match, scale, ground contact, alpha edge, and animation quality.

#### Requirement O2 — collision-safe vertical slice

Refresh one common obstacle first, preferably a barrier or barrel that appears early.

Important code constraint: `PZGObstacle::initPhysics` subtracts `sprite->getContentSize()` when converting configured collision points and uses the sprite size as the whole collision box when points are absent. Trimming or changing source size can therefore change physics even when configuration vertices are untouched.

Before replacing art:

- preserve original untrimmed source canvas, content size, anchor, offset, and scale; or
- refactor collision conversion to use explicit logical source dimensions stored in data, then migration-test every existing obstacle.

The safer first slice is source-canvas compatibility. Decoupling collision dimensions is the correct long-term architecture but must be a separate tested change.

Vertical-slice exit criteria:

- new art is immediately clearer in the full background;
- collision debug overlay matches the baseline;
- identical replay input produces identical collision outcome and checkpoint hashes;
- near-miss detection, destruction, flash, scale randomization, and death art still work;
- no alpha fringe, atlas bleeding, ground float, or unintended size increase appears;
- two target devices hold the frame-time and memory budgets.

#### Requirement O3 — obstacle family refresh

After the slice passes, replace in families so style remains coherent:

1. Road barriers, cones, barrels, and warning hardware.
2. Rocks, rubbish, tires, and broken platform debris.
3. Mechanical/industrial hazards and moving obstacles.
4. Destruction frames, debris, impact flashes, dust, and contact shadows.

Each family receives one light/material sheet and one gameplay contact sheet before atlas packing.

### 21.10 Parallax background and platform presentation

The existing backgrounds and platforms remain the reference. Improve depth, variation, and integration rather than replacing their identity.

#### Parallax stack

Use a documented layer stack:

| Layer | Typical content | Relative movement | Contrast |
|---|---|---:|---:|
| Sky/atmosphere | clouds, haze, distant light | 0.02–0.06 | Very low |
| Far silhouette | skyline, trees, giant distant dinosaur | 0.08–0.15 | Low |
| Mid environment | buildings, bridges, sewer walls | 0.18–0.32 | Medium-low |
| Scenic action | wall-peek dinosaur, distant riders | 0.20–0.38 | Medium-low |
| Near environment | fences, pipes, foliage | 0.45–0.65 | Medium |
| Gameplay road | platforms, ramps, hazards | 1.00 | Highest gameplay contrast |
| Optional foreground | rare blurred post/leaf sweep | 1.10–1.25 | Low opacity; never obscures road |

Requirements:

- Every repeating layer must tile without a visible seam at all supported aspect ratios.
- Avoid repeating unique landmarks so frequently that motion becomes obvious.
- Create two or three modular variants per mid/near layer before making an entirely new biome.
- Platforms keep a clear bright-enough top edge and darker side face.
- Platform art cannot conceal gaps or imply a solid surface outside collision bounds.
- Add small material callbacks—wet reflection, loose pebble, pipe steam—through cosmetic systems, not physics bodies.
- Foreground sweeps are disabled in Reduced spectacle mode and during dense hazards.

### 21.11 V2 event-art reskin

Replacing `v2_events.png` is mandatory, not optional polish. The current atlas established event identities and code integration, but it failed the main-game style requirement. After hurdle replacement and the style bible are proven, replace the placeholder V2 atlas in this order:

1. Warning signs, cones, barrels, shadows, meteor, rocks, and objective props.
2. Small and large dinosaurs.
3. Delivery, Cool Kid, Grandma, worker, and ambient rider gameplay sprites.
4. Flames, dust, impact stars, reward effects, and variant-specific props.
5. Dialogue close-ups only after each gameplay identity is approved.

Rules:

- Preserve existing frame names for the first reskin to avoid simultaneous code and art migration.
- Preserve animation frame count and timing initially. Improve animation only in a separate change with replay and cleanup tests.
- Use the existing bike, platform/ramp, sewer parallax, coin, and HUD references for every replacement batch. Never use the current V2 atlas as the style target.
- Rider sprites must share wheel diameter, road contact, direction, lighting, and scale with the player bike.
- Event-specific colors identify actors but must not overpower coin gold or hazard warnings.
- Grandma can be exaggerated and funny while still using the same material and lighting language.
- The T-Rex gameplay actor and decorative background dinosaurs must be visually distinct in depth and contrast so players know which one matters.

### 21.12 Screen-by-screen presentation requirements

#### Main menu

- Keep the logo and primary Play action dominant.
- Use a living but low-motion sewer background: water shimmer, one distant light flicker, rare dinosaur silhouette.
- Show the selected bike/rider at useful scale.
- Surface Daily Challenge and Friends/Challenges as secondary cards only when those systems work.
- Avoid a grid of equal-weight buttons.

#### Run setup / dialogue

- Show rider and rival close-ups, run type, modifier/objective, and one primary Start action.
- Normal run can skip this screen unless a story/rival trigger exists.
- Daily setup must clearly show three ranked attempts, current modifier, best score, medal thresholds, and Practice.

#### Gameplay

- Preserve the current bike, coins, platforms, and core HUD.
- Integrate objectives into the existing hierarchy; no additional permanent top bar.
- Temporary event HUD should enter, update, and leave as one component.
- Decorative spectacle yields to road readability automatically.

#### Pause/settings

- Add Motion: Full/Reduced and Spectacle: Full/Reduced/Off.
- Explain that spectacle changes only decoration, not difficulty or score.
- Preserve large touch targets and safe-area placement.

#### Results

- Lead with distance/score and medal, then the most interesting event or rivalry result.
- Show one clear next action: Retry, Next Ranked Attempt, or Share Challenge.
- Use a frozen action snapshot or approved character pose, not an unrelated decorative image.
- Do not claim friend competition until a real challenge token or leaderboard result exists.

All screens must adapt to safe areas and multiple aspect ratios. Apple's game guidance recommends adaptive menus, safe-area-aware controls, and customization for motion intensity: [Designing for games](https://developer.apple.com/design/human-interface-guidelines/designing-for-games/) and [Game controls](https://developer.apple.com/design/human-interface-guidelines/game-controls).

### 21.13 V2 presentation architecture

Do not continue adding visual logic directly to `PZRunDirector`. It already coordinates gameplay, power-ups, camera, HUD, revive, and V2 signals. The presentation expansion needs clean ownership.

#### Proposed modules

```text
Source/V2/Presentation/
  V2PresentationConfig.h/.cpp     typed visual/camera/dialogue/scenic tuning
  V2PresentationDirector.h/.cpp   lifecycle coordinator and arbitration
  V2ScenicDirector.h/.cpp         cosmetic background beats and repeat protection
  V2DialogueDirector.h/.cpp       start-only dialogue selection, state, skip, cleanup
  V2CinematicDirector.h/.cpp      exclusive camera/input/HUD cinematic ownership
  V2AssetCatalog.h/.cpp           stable IDs to frames, portraits, sounds, fallbacks
  V2PresentationEvents.h          immutable signals from gameplay to presentation
```

```text
Content/
  CompleteGameInfo.plist          authoritative V2 registry, tuning, dialogue, scenic and asset bindings
Content/Images/HD/
  v2_obstacles.png/.plist
  v2_scenic.png/.plist
  v2_riders.png/.plist
  v2_portraits.png/.plist
  v2_ui.png/.plist
```

Atlas plists contain generated frame rectangles required by Axmol; they do not own gameplay or presentation definitions. Generated manifests and migration reports belong in the tooling/work directory and are derived from `CompleteGameInfo.plist`, not loaded as competing runtime configuration. Do not require all proposed atlases immediately. Split only when measured loading, memory, or ownership benefits justify it.

#### Ownership rules

- Gameplay publishes immutable events such as `RunWillStart`, `RivalSelected`, `SafeScenicWindow`, `RoadEventWillStart`, `ClimaxWillStart`, `PlayerDied`, and `RunEnded`.
- Presentation observes events but cannot mutate score, physics, road generation, daily attempts, replay input, or gameplay RNG.
- `V2ScenicDirector` uses a dedicated cosmetic RNG stream. Cosmetic decisions are not part of competitive replay state.
- `V2DialogueDirector` selects only pre-run sequences and records viewed IDs through a versioned persistence interface.
- `V2CinematicDirector` is the single authority for camera transform, input lock, HUD visibility, world freeze, letterbox, and cleanup while a cinematic owns the screen.
- Existing `PZRunDirector` camera functions become the low-level adapter or move behind the cinematic interface; there must not be two systems writing scene position and scale.
- Every asset lookup has a fallback so a missing optional scenic asset cannot crash or block a run.
- Presentation configuration is read once into validated typed structures. No scattered string keys or unbounded values in feature code.

#### Runtime layer contract

Use named layer hosts rather than arbitrary Z values throughout feature code:

```text
BackgroundSky
BackgroundFar
BackgroundScenic
BackgroundMid
GameplayBehind
GameplayWorld
GameplayEffects
GameplayFront
ScreenHUD
CinematicOverlay
ModalUI
```

The adapter may map these names to current Axmol nodes/Z orders. The contract prevents a decorative dinosaur from accidentally rendering over the rider or HUD.

#### Cinematic ownership state

Only one of these states may own camera/input at a time:

```text
None -> PreRunDialogue -> Intro -> Gameplay
Gameplay -> Climax -> Gameplay
Gameplay -> Blast -> Gameplay
Gameplay -> Death -> Results
Gameplay -> Revive -> Gameplay
```

Invalid transitions fail safely by declining the new cosmetic/cinematic request. Death, scene exit, and app interruption always win and force common cleanup.

### 21.14 Asset pipeline and cost control

The repository already uses Axmol-compatible `.plist` sprite atlases. Axmol's own documentation supports loading sprite frames from plist atlases and identifies TexturePacker/Zwoptex-style tools: [SpriteFrameCache reference](https://axmol.dev/manual/latest/d8/dff/classax_1_1_sprite_frame_cache) and [Axmol sprite-sheet tools](https://git.mitgai.net/axmolengine/axmol/wiki/Sprite-Sheets-Tools-and-Formats).

Recommended pipeline:

1. Store approved high-resolution masters outside runtime atlases.
2. Export target-size transparent PNG frames through a repeatable script/preset.
3. Pack with TexturePacker CLI if already licensed; otherwise evaluate Axmol-listed `spright` or another maintained free packer using a small compatibility test.
4. Use trim only when logical source size and anchor are explicitly preserved. Do not auto-trim legacy collision-dependent obstacles blindly.
5. Extrude atlas edges and add sufficient padding to prevent sampling bleed.
6. Generate a manifest report containing dimensions, frame names, atlas occupancy, file size, and missing references.
7. Optimize losslessly first. Evaluate GPU texture formats only after confirming Axmol/platform support and measuring quality, load time, binary size, and memory. KTX 2/Basis can reduce transfer and GPU memory in supported pipelines, but it is not an automatic migration: [Khronos KTX](https://www.khronos.org/ktx/).
8. Keep source and generated runtime assets separate; generated atlases should be reproducible and should not be edited by hand.

Do not add Spine, Rive, or another animation runtime for the initial pass. Short sprite sequences and existing Axmol actions are cheaper and sufficient. Re-evaluate a skeletal runtime only if portrait/body animation volume becomes large enough to justify SDK size, tooling, licenses, platform risk, and another runtime dependency.

#### Repository helper pipeline

Use `tools/art_pipeline.py` and `tools/ART_PIPELINE.md` as the supported workflow. This consolidates repetitive geometry, atlas, and prompt context so later changes need fewer manual steps and fewer AI tokens.

| Command | Purpose | Mutation behavior |
|---|---|---|
| `trace` | Trace PNG alpha into the exact five-point, bottom-left, absolute-pixel collision format consumed by `PZGObstacle` and optionally render a review overlay | Read-only unless plist target and `--apply` are both explicit |
| `compare` | Compare source canvas, alpha bounds, exact mask, and alpha intersection-over-union between baseline and replacement | Read-only; strict flags can fail CI |
| `atlas` | Export frame rectangles, logical source sizes, offsets, alpha bounds, and occupied pixels as JSON | Read-only unless an output report path is supplied |
| `brief` | Generate a compact reusable art brief containing exact frame metadata and the locked main-game style direction | Read-only |

The supported pipeline uses the repository-local OpenCV environment created by `tools/setup_art_env.sh`. The older scripts in `reskin-automation/automation/` are historical experiments, not the supported collision pipeline: their center-relative output does not match the current `PZGObstacle::initPhysics` coordinate contract. Do not use their output for gameplay fixtures. `tools/art_pipeline.py` keeps OpenCV's contour/convex-hull quality but converts to the game's absolute bottom-left coordinates, produces a review overlay, and requires explicit `--apply` before changing the plist.

Alpha tracing is a starting proposal, not an automatic gameplay decision. Review every overlay: smoke, antennae, cloth, tiny debris, and decorative protrusions usually should not enlarge a lethal fixture. Apply collision remapping only when the art silhouette genuinely changes and replay/device testing is scheduled in the same step.

### 21.15 Performance, accessibility, and quality budgets

Set budgets before producing the full art set:

- Maintain the existing target frame rate on the lowest supported iOS and Android devices.
- Measure p50, p95, and worst frame time during large dinosaur beats, dialogue transitions, event starts, and atlas loading.
- No runtime texture upload during an active hazard; preload the next required set or use an explicit safe transition.
- Do not assume PNG file size equals GPU memory. Track decoded texture dimensions and pixel format.
- Set per-atlas and peak presentation-memory budgets after baseline measurement; do not invent a number without device evidence.
- Keep draw-call growth bounded by atlas/layer grouping and verify batching rather than assuming it.
- Add Reduced Motion and Spectacle settings before shipping new full-screen camera or parallax behavior.
- Important cues must use at least two of color, shape, text, audio, or haptic feedback.
- Dialogue supports localization expansion and never embeds text in images.
- Test 4:3, 16:10, 16:9, and tall phone safe areas; no critical text or controls outside the safe region.

### 21.16 Sequenced implementation requirements

This is the authoritative step-by-step order. Finish each gate before starting the next major step.

#### Step 0 — Baseline and style lock

- Create gameplay screenshots, collision overlays, frame-time captures, memory captures, and an art inventory.
- Approve the five reference categories and cinematic-painted-arcade-realism style sheet.
- Identify every place the old obstacle and V2 atlases are loaded.

**Gate:** approved reference board, manifest schema, baseline measurements, and no unresolved ownership/licensing issue.

#### Step 1 — One collision-safe hurdle refresh

- Generate and integrate one refreshed common hurdle using section 21.9.
- Preserve logical canvas/anchor/collision behavior.
- Test normal run, daily run, near miss, destruction, replay, and low-end device performance.

**Gate:** clearer art with identical gameplay outcome and no regression.

#### Step 2 — Complete hurdle families

- Refresh road hardware, debris, industrial hazards, and destruction/effect families.
- Update the art manifest and comparison sheets for each family.

**Gate:** every legacy hurdle passes contrast, collision, alpha, ground-contact, replay, and device checks.

#### Step 3 — Presentation architecture foundation

- Add typed presentation config, event signals, named layer hosts, asset catalog, presentation coordinator, and cinematic ownership.
- Adapt existing camera/HUD/input functions behind one authority without changing visible behavior.
- Add Full/Reduced motion and Full/Reduced/Off spectacle configuration.

**Gate:** existing intro, blast, death, and revive flows pass unchanged; forced interruption always restores neutral camera/input/HUD state.

#### Step 4 — Start dialogue vertical slice

- Produce one approved player portrait and one Cool Kid portrait with four consistent expressions each.
- Implement first encounter, rematch-after-win, and rematch-after-loss sequences.
- Add skip, viewed-state versioning, localization-ready data, audio ducking, and interruption cleanup.

**Gate:** first view feels professional, repeat view is fast, skip is immediate, and no simulation tick advances behind dialogue.

#### Step 5 — Background dinosaur vertical slice

- Implement Distant Walker and Wall Peek through `V2ScenicDirector`.
- Use dedicated cosmetic RNG, safe-window suppression, repeat protection, and accessibility variants.

**Gate:** 8/10 testers notice at least one beat, no tester mistakes it for a hazard, and road-event performance/readability does not worsen.

#### Step 6 — Parallax and environmental depth pass

- Modularize far/mid/near layers, remove obvious seams/repeats, and add restrained ambient callbacks.
- Preserve gameplay platform top-edge clarity and collision truth.

**Gate:** all aspect ratios tile correctly; Reduced mode is stable; performance budget passes a ten-minute run.

#### Step 7 — V2 event-art reskin

- Reskin props/hazards first, then dinosaurs, riders, effects, and portraits.
- Preserve frame names/timings until the art-only regression pass succeeds.

**Gate:** every V2 event is visually coherent with the bike/platform/background reference set and retains deterministic outcomes.

#### Step 8 — Camera and screen polish

- Add approved establishing, rival, scenic, and climax shots through the single cinematic owner.
- Polish main menu, daily setup, pause/settings, and results using section 21.12.
- Keep restart and repeat dialogue fast.

**Gate:** camera never hides a hazard, every path resets exactly, screen layouts pass safe areas, and Reduced Motion replaces problematic movement.

#### Step 9 — Entertainment validation

- Run readability tests at normal and maximum speed.
- Run ten fresh-player sessions and five returning-player sessions.
- Measure dialogue skip, spectacle notice, hazard confusion, retry time, event completion, and voluntary replay.
- Compare against the pre-graphics baseline; visual novelty is not success if failure confusion or restart time increases.

**Gate:** no gameplay regression, measurable improvement in perceived polish/variety, and no significant increase in unintended deaths or time-to-retry.

#### Step 10 — Social presentation only after system readiness

- Add challenge-entry and ghost presentation only after section 20.10 services and replay gates pass.
- Do not create mock social screens that imply unavailable multiplayer.

**Gate:** challenge links work end to end on both platforms and every competitive result carries compatible rules/config versions.

### 21.17 Definition of “great bike game” for this project

The graphics update is successful only when the game delivers all of the following together:

- The bike and landing surface remain readable instantly.
- Obstacles look intentional, dangerous when appropriate, and physically grounded.
- The world produces memorable but non-disruptive surprises.
- Named rivals have recognizable visual identity and remember outcomes.
- Start-of-run dialogue gives a reason to care, then gets out of the way.
- Camera motion strengthens important moments without stealing control or causing confusion.
- Daily and social screens clearly explain stakes and next actions.
- Art, UI, events, dinosaurs, portraits, and effects appear to belong to one product.
- Reduced-motion and low-spectacle players retain full gameplay information.
- Every art change preserves physics, replay behavior, cleanup, performance, and ordinary non-v2 runs.

More art is not the target. A coherent, readable, surprising, and repeatable ride is the target.

### 20.13 Implementation update after the 20.7–20.12 review (compiled, NOT play-tested)

- **T-Rex payoff defect (20.7) fixed:** the T-Rex now stays alive ~2 s after the last hazard so the line can be read, then is removed.
- **T-Rex in three phases (20.9 #3, partial):** PURSUIT (4 boulders) → LOG KICKS (3 fast rolling logs) → TWIN STOMPS (2 double strikes with an EDGE RUN coin trail). A bite / collapsing-road / ramp phase is not built.
- **Second variants for Pizza, Cool Kid and Grandma (20.9 #1):**
  - Pizza "double order": boxes drop in pairs, 6 of 9 needed.
  - Cool Kid "wheelie duel": air time is worth 2.4x but he drifts away 1.6x faster.
  - Grandma "slipstream": she lingers ahead; riding on the ground in her slipstream for 1.2 s pays a bonus (a visible percentage in the objective plate; no separate meter graphic).
  Variants unlock from the standard tier (150 m); QA can force one with `forceRoadEvent("pizza_panic:1")`.
- Not done: expert-only patterns (20.9 #2), meteor shortcut/ramp/chain reaction, Construction lift puzzle, rival roster, and every P0 validation item in 20.8 (golden replay, daily tests, play-through matrix, fresh-player sessions). Do those before adding more features.

### 20.14 P0 validation progress (2026-11-21)

| 20.8 item | Status | Evidence / what remains |
|---|---|---|
| 1 Full event play-through matrix | **Not done** | Needs a human or a survivable bot per variant/tier; only a forced Dino stampede and the T-Rex start were seen. |
| 2 T-Rex repair | Partly | Payoff line lifetime fixed; three phases built; the full 11-hazard sequence has not been played through on success and failure paths. |
| 3 Deterministic replay | **Done on the iOS simulator** | `Content/golden_replay.v2r` re-recorded for rules v2 (797 ticks, includes the first Dino Crossing event, modifier ONE LIFE). Result: `checked=26 exact=1 endTick=797/797`, both at normal frame rate and with a ~15 fps stall. Android and real-device runs are still missing. |
| 4 Automated daily rules | **Done** | `Daily::selfTest()` (`Source/V2/V2DailyTest.cpp`) runs 27 checks: attempts, best retention, reward-once and medal upgrades, forfeit, six modifiers and ordered thresholds, event coins not double counted, date rollover, legacy-save migration. Result: 27/27 PASS in the simulator. |
| 5 Balance from session data | Not done | Needs player sessions. |
| 6 Device / layout QA, frame-time buckets | Not done | |
| 7 Fresh-player observation | Not done | |

Findings from this pass (all fixed):
- **Replays ignored the daily modifier.** A run recorded under ONE LIFE replayed with standard event timing, so the replay survived a Dino hit the recording died on (`endTick 828/797`, timeout). Replay files now carry the modifier index (`V2R fmt rules seed endTick ended modifier`; older files read as "none") and playback re-applies it (coin density, event breather, pizza boost, no-mercy).
- **Legacy daily saves.** A rules-v1 save (7-field `_r`, boolean `_c`) was unreadable by the new code and could pay the reward again. `Daily` now migrates a date once (`_v` = 2): attempts used = 1, reward treated as paid.
- Replay results are now also written to the analytics file (`replay_result`).

How to repeat: the self-test and the replay were driven by a temporary in-app hook that is not in the repo. To re-run, call `V2::Daily::selfTest()` from a debug hook and `PZRunDirector::startReplayTest()` (default is the bundled golden file) then start a run; read `replay_result` in `v2_analytics.jsonl`.

## 22. The mission: turn social-media attention into players who stay

### 22.1 Mission statement

The mission is not to make a trailer that receives clicks. The mission is to make a bike game that earns a second run, a return tomorrow, and a recommendation to a friend.

> Social videos create the first visit. The game must create the next visit.

We will compete through a distinctive combination of readable bike action, surprising dinosaur-scale spectacle, memorable riders, fast retries, daily competition, and an evolving world. We will not win by copying the number of modes or assets in larger competitors. We will win only if the core ride is more immediately understandable, more surprising, and more enjoyable to repeat.

This section supersedes any earlier plan that treats a feature as complete merely because it compiles or appears once on screen.

### 22.2 Acquisition-to-entertainment contract

Every promise shown in a social video must have an honest in-game payoff:

| Social hook | Required in-game payoff | Failure to avoid |
|---|---|---|
| Giant dinosaur behind the rider | A safe, readable scenic dinosaur beat appears during normal play, with multiple variants over time | Trailer-only scene or dinosaur mistaken for a hazard |
| Rival biker dialogue | A short professional pre-run exchange, visible rivalry record, and changed rematch line | Long unskippable story with no gameplay consequence |
| Funny passing rider | Passing riders perform setup → reaction → callback behaviors without blocking the road | Random clutter that repeats every minute |
| T-Rex escape | Distinct phases and a memorable finish, not ten repetitions of the same falling object | Spectacle with shallow play or unfair deaths |
| Daily challenge | Three real ranked attempts, visible modifier, score breakdown, fair rewards, and working comparison | A menu label without meaningful competition |
| Challenge a friend | A real versioned challenge link, compatible seed/rules, result comparison, and later a verified ghost | Native share sheet marketed as multiplayer |

The first-session pacing target is:

1. **0–10 seconds:** player sees the bike, world identity, and one clear Play action.
2. **10–25 seconds:** optional short rider/rival setup, immediately skippable.
3. **Within 30 seconds of control:** player performs a satisfying jump/landing and receives clear feedback.
4. **Within 45–75 seconds:** player sees one understandable event, rival, or background spectacle.
5. **Within 90–150 seconds:** player receives a payoff—event result, rivalry change, micro-challenge completion, or surprising callback.
6. **After failure:** useful result and retry input are available quickly; presentation must not punish another attempt.

Do not force all entertainment into the first minute. Show enough identity to prove the social promise, then preserve discoveries for later runs.

### 22.3 Retention and entertainment scorecard

Before the social campaign, capture a baseline build. After each release candidate, compare the same metrics and player-observation questions.

#### Product metrics

- First Play tap from the main menu.
- Time from launch to player control.
- First-run duration and cause of exit.
- Percentage reaching the first road event.
- Percentage starting a second run without being prompted.
- Runs per first session and median time between retries.
- Daily Challenge open, ranked attempt, second attempt, and next-day return.
- Event start, success, failure, technical skip, death, and voluntary replay by event/variant/tier.
- Dialogue viewed, advanced, skipped, and abandoned by dialogue ID.
- Scenic beat shown, noticed proxy, suppressed reason, and performance bucket.
- Share tap, share completion, challenge open, challenge completion, and sender/receiver return when those systems exist.
- D1 and D7 retention by acquisition source and app version.

#### Player evidence

Ask fresh players after the session, without leading them:

- What was the goal?
- Which moment do you remember?
- What caused your death?
- Did anything feel unfair or confusing?
- Which rider or event would you want to see again?
- Why did you retry or stop?
- Would you send this challenge to a friend?

Initial release gates are relative to the measured baseline, not invented industry numbers:

- Time to control must not increase.
- Unexplained deaths and “I do not know what happened” answers must decrease.
- Voluntary second-run rate, median runs per first session, and first-event completion must improve.
- At least 8/10 observed new players can describe the core action and one memorable event.
- At least 6/10 voluntarily begin another run during the test window.
- No social-media promise may rely on a scene or mode unavailable in the tested build.

If attention rises but voluntary retries and returns do not, the mission is failing. Do not respond by producing more advertisements; fix the game loop.

### 22.4 Entertainment operating principles

1. **Readable first, surprising second.** A joke or dinosaur never hides the landing surface.
2. **Interaction beats decoration.** At least some events must change player decisions; reskins alone do not count.
3. **Setup and payoff.** Comedy needs a callback, rivalry needs memory, and spectacle needs a finish.
4. **Fast repeat.** Repeat dialogue abbreviates, cinematics skip, and retry is never buried.
5. **Controlled novelty.** Use shuffle bags, cooldowns, variants, and tier rules instead of random visual noise.
6. **Visible consequence.** The world or rival reacts to player success and failure.
7. **Honest competition.** Daily and friend results carry the same seed, modifier, rules version, and content version.
8. **Content is data.** New entertainment must be registered and tunable through `CompleteGameInfo.plist`, not scattered through C++.

## 23. CompleteGameInfo-driven V2 content architecture

### 23.1 Non-negotiable source-of-truth rule

`Content/CompleteGameInfo.plist` is the authoritative registry for all new V2 content and presentation behavior.

Every new sprite sheet must be registered in `CompleteGameInfo.plist` before code may load or use it. Every new animation, actor, portrait set, scenic dinosaur, dialogue sequence, event-art binding, UI skin, effect set, and content group must be reachable from that registry through a stable semantic ID.

The engine still needs a companion atlas `.plist` beside each atlas texture because Axmol's `SpriteFrameCache` reads packed frame rectangles from it. That atlas plist is storage metadata only. It must not become a second gameplay/content configuration source.

The division of responsibility is:

| File | Owns | Must not own |
|---|---|---|
| `CompleteGameInfo.plist` | Atlas registry, semantic IDs, animation frame lists/timing, anchors/logical sizes, roles, dialogue, scenic definitions, presentation tuning, preload groups, feature enablement, content/rules versions | Packed pixel rectangles generated by the atlas tool |
| Atlas `.plist` | Generated frame rectangles, trim offsets, source sizes, rotation flags, texture filename | Gameplay rules, event selection, dialogue, timing policy, content unlocks |
| Atlas texture `.png` | Runtime pixels only | Names, rules, timing, collision policy |
| C++ | Typed loaders, validation, orchestration, gameplay implementation | Raw new atlas filenames, raw frame-name sequences, authored dialogue lines, or per-asset tuning constants |

This satisfies “everything works from `CompleteGameInfo.plist`” without duplicating generated atlas geometry into two files that can drift apart.

### 23.2 Current violations to migrate

The current implementation does not yet meet this rule:

- `V2RoadEvents.cpp` hard-codes `v2_events.plist` when loading the event atlas.
- It checks and creates raw frames such as `sign_dino_0` and `rider_cool_0` directly.
- Animation bases and frame counts such as `rider_cool` with four frames are passed in code.
- `V2Core.cpp::BarkBook` contains authored dialogue strings in C++.
- `tools/make_v2_atlas.py` writes a fixed filename and knows content identities independently of the game registry.

These are technical debt from the prototype. Do not repeat the pattern for new graphics. Migrate them before adding the production replacement atlases, portraits, or background dinosaur sheets.

### 23.3 Proposed plist schema

Keep the existing top-level `v2` dictionary and add versioned nested presentation/content data. Exact key spelling is locked when the typed loader is implemented.

```text
v2
  rulesVersion
  contentVersion
  presentation
    schemaVersion
    atlases[]
      id
      plist
      texture
      required
      preloadGroup
      memoryClass
    animations
      <animationId>
        atlasId
        frames[]
        fps
        loop
        anchor
        logicalSize
        role
        collisionPolicy
    actors
      <actorId>
        idleAnimation
        moveAnimation
        reactionAnimations
        portraitSet
        accentColor
        scale
        groundOffset
    obstacleArt
      <obstacleId>
        animationId
        deathAnimationId
        collisionSource
        collisionInset
        gameplayObstacleIndex
    scenicBeats
      <beatId>
        actorId
        layer
        cooldown
        duration
        minRunSeconds
        suppressDuring[]
        motionVariant
        reducedMotionVariant
    portraitSets
      <portraitSetId>
        neutral
        confident
        surprised
        frustrated
    dialogues
      <dialogueId>
        trigger
        oncePolicy
        priority
        speakers[]
        lines[]
        backgroundId
        musicDuck
    uiSkins
      <skinId>
        panelFrame
        buttonFrames
        colors
        fonts
    contentGroups
      <groupId>
        atlases[]
        animations[]
        requiredFor[]
```

Example conceptual registration:

```text
atlases:
  - id: v2_event_art
    plist: v2_events_production.plist
    texture: v2_events_production.png
    required: true
    preloadGroup: gameplay_v2

animations:
  cool_kid_ride:
    atlasId: v2_event_art
    frames: [rider_cool_0, rider_cool_1, rider_cool_2, rider_cool_3]
    fps: 10
    loop: true
    role: cosmetic_rider
```

The final file remains a valid Apple plist, not YAML. The text above documents structure only.

### 23.4 Typed runtime architecture

Add these responsibilities to the planned V2 presentation architecture:

```text
V2ContentRegistry
  loads and validates v2.presentation once
  resolves semantic IDs
  exposes immutable typed definitions

V2AtlasRegistry
  loads/unloads registered atlas groups
  confirms atlas metadata matches registered texture
  prevents duplicate frame ownership

V2AssetCatalog
  creates sprites/animations from semantic IDs
  applies registered anchor, scale, and ground offset
  supplies explicit fallback assets

V2DialogueCatalog
  supplies localized dialogue definitions by ID
  contains no authored fallback conversation in C++

V2ContentValidator
  runs in tools and debug startup
  validates every cross-reference and reports all errors together
```

Feature code should request:

```text
assets.createActor("cool_kid")
assets.createAnimation("cool_kid_ride")
dialogues.find("cool_kid_first_encounter")
scenic.start("distant_walker")
```

Feature code must not request:

```text
SpriteFrameCache::addSpriteFramesWithFile("v2_events.plist")
Sprite::createWithSpriteFrameName("rider_cool_0")
spawn("rider_cool", 4, ...)
```

The semantic API is the stable contract. Artists may repack or rename physical sheets through a controlled migration without rewriting event logic.

### 23.5 Content validation requirements

The build-time validator must fail when:

- a registered atlas plist or texture is missing;
- atlas metadata names a different texture than the registry;
- a frame referenced by an animation, portrait, UI skin, or effect does not exist;
- two required atlases define the same frame name;
- an animation is empty, has an unsafe FPS, or mixes incompatible logical sizes without an explicit allowance;
- an actor references a missing animation or portrait set;
- a dialogue references a missing speaker, portrait expression, localization key, or trigger;
- a scenic beat uses a gameplay collision policy or an invalid render layer;
- a gameplay obstacle replacement has no explicit collision policy;
- a source canvas, anchor, collision mapping, or logical size changes without an acknowledged migration;
- a required content group cannot be preloaded within its budget;
- content or rule changes were made without increasing the appropriate version.

Debug builds must print one complete actionable report. Release builds must never crash midway through a run because optional cosmetic content is missing. They should disable the invalid optional beat, use an explicit fallback where safe, log the failure, and keep gameplay deterministic. Required gameplay content should fail CI and release packaging instead of silently shipping broken.

### 23.6 Tooling changes required

Extend `tools/art_pipeline.py` in the implementation phase with registry-aware commands:

| Planned command | Purpose |
|---|---|
| `register-atlas` | Add or update an atlas entry in `CompleteGameInfo.plist` with explicit ID, preload group, required flag, plist, and texture |
| `register-animation` | Validate frames in an atlas and write the semantic animation definition |
| `validate-content` | Validate every atlas/frame/actor/dialogue/scenic/UI cross-reference and version rule |
| `extract-frame` | Export a named frame on its logical source canvas for repainting and collision review |
| `contact-sheet` | Generate a labeled visual sheet for an atlas or semantic content group |
| `migration-report` | Compare old and new registries, atlas metadata, anchors, source sizes, collision mappings, and memory estimates |

All write commands must be dry-run by default, print the exact proposed changes, require `--apply`, preserve a recoverable backup or work through version control, and run `plutil -lint` plus `validate-content` afterward.

The generated compact art brief must read style and asset requirements from the same registry where possible. This reduces repeated prompt context and AI token use while preventing the style rule from being forgotten.

### 23.7 Versioning and replay compatibility

- `rulesVersion` changes when gameplay, scoring, collision, event timing, or deterministic outcomes change.
- `contentVersion` changes when required art bindings, animation definitions, dialogue triggers, or presentation content change.
- Replay and friend-challenge data store both versions.
- Pure pixel repainting that preserves frame geometry and behavior may retain `rulesVersion` but still updates `contentVersion` for diagnosis.
- Collision remapping always changes `rulesVersion` and requires a new golden replay.
- A challenge may compare results only when its compatibility policy accepts both versions.
- Analytics events include app version, rules version, content version, event ID, variant, and acquisition source when available.

### 23.8 Authoritative implementation sequence

This merges the entertainment, graphics, and data-driven requirements into one execution order.

#### Phase 0 — Measure the current game

- Capture launch-to-control, first-run, retry, event, performance, and retention baselines.
- Run fresh-player observation before changing presentation.
- Save current atlas, collision, replay, memory, and screenshot evidence.

**Exit:** we know what must improve and can detect regression.

#### Phase 1 — Build the CompleteGameInfo content registry

- Lock the `v2.presentation` schema and content/rules version policy.
- Implement typed registry, atlas loader, semantic asset catalog, validation, and fallbacks.
- Add registry-aware tooling with dry-run/apply behavior.

**Exit:** a small test atlas, animation, actor, and dialogue load exclusively through semantic IDs from `CompleteGameInfo.plist`; invalid references fail validation.

#### Phase 2 — Migrate existing V2 prototype content

- Register the current placeholder `v2_events` atlas temporarily.
- Move frame sequences, frame counts, FPS, scales, ground offsets, and actor bindings out of C++.
- Move BarkBook dialogue into registered/localizable dialogue data.
- Remove hard-coded atlas and raw frame loads from `V2RoadEvents`.

**Exit:** existing V2 behavior and deterministic replay remain identical, while C++ contains no raw V2 atlas filename or authored frame sequence.

#### Phase 3 — Refresh faded hurdles safely

- Follow section 21.9 and use the supported alpha/collision tools.
- Ensure every replacement and collision policy is represented in `CompleteGameInfo.plist`.
- Keep art-only and collision changes separately reviewable.

**Exit:** hurdles are clearer, collision/replay evidence passes, and registry validation is clean.

#### Phase 4 — Replace the complete V2 event atlas

- Replace every `v2_events.png` placeholder sprite using the bike/platform/background/HUD references.
- Register all new sheets, animations, actors, effects, and content groups before integration.
- Preserve semantic IDs even if physical atlas layout changes.

**Exit:** no flat placeholder art remains; all event visuals match the main game and load from `CompleteGameInfo.plist` definitions.

#### Phase 5 — Add dialogue portraits and pre-run screens

- Register portrait sheets, expressions, speakers, dialogue sequences, triggers, once/repeat policy, and presentation tuning.
- Implement short professional close-up exchanges with skip and interruption cleanup.

**Exit:** no dialogue line, portrait frame, or sequence timing is authored in C++.

#### Phase 6 — Add decorative dinosaur spectacle and parallax improvements

- Register scenic atlases, actors, beat definitions, cooldowns, suppression rules, layers, and reduced-motion variants.
- Add Distant Walker and Wall Peek first; validate readability before expanding.

**Exit:** new players see the social-media promise, no player mistakes decoration for a hazard, and no scenic content affects replay or gameplay RNG.

#### Phase 7 — Finish entertainment depth

- Complete expert event patterns, T-Rex phase variety, Construction choice, Meteor consequences, and rival progression.
- Keep all art/timing/reward bindings data-driven while gameplay algorithms remain typed and testable.

**Exit:** each repeatable event passes Notice → Choice → Skill → Reaction → Consequence and has at least two meaningful variants.

#### Phase 8 — Screens, camera, daily, and fast-retry polish

- Finish menu, daily setup, results, settings, cinematic arbitration, Reduced Motion, and spectacle settings.
- Protect launch-to-control and retry speed.

**Exit:** all screens pass safe areas and interruption tests; camera never hides hazards; repeat presentation is abbreviated.

#### Phase 9 — Retention validation before marketing scale

- Run the fresh/returning player sessions and compare section 22.3 metrics with baseline.
- Fix confusion, unfairness, repetition, and slow restart before buying or scaling traffic.
- Verify every planned social video against the release candidate.

**Exit:** voluntary retry and observed entertainment improve without increasing confusion or technical failure.

#### Phase 10 — Launch content and learn

- Release social videos in measured groups with campaign/source tags.
- Compare acquisition quality, first-session behavior, D1/D7 return, and challenge activity.
- Ship small registered content additions based on evidence, not panic feature accumulation.

**Exit:** the content cadence is sustainable, measurable, and does not bypass registry or QA rules.

### 23.9 Definition of victory

Victory is not one launch spike. Victory is a repeatable system where:

- social videos truthfully show experiences players reach in the game;
- new users understand the bike quickly and remember a specific entertaining moment;
- failure feels deserved and retry is immediate;
- players voluntarily ride again and return for daily/rival progress;
- every new asset is coherent with the main-game style;
- every new sprite sheet and content binding is registered through `CompleteGameInfo.plist`;
- content can be replaced, tuned, validated, and versioned without hard-coded asset edits throughout C++;
- analytics and player observation decide the next update;
- ordinary runs, replay determinism, device performance, and accessibility remain protected.

The mission is winnable, but confidence is not evidence. We win by building the loop, measuring behavior, fixing what players actually feel, and refusing to market features that are not ready.

### 23.10 Tooling serves the mission

OpenCV is the supported contour and convex-hull engine for collision remapping because it has already produced strong results for this project and reduces manual boundary work. It is installed only in the repository-local `.venv-art` environment through pinned `tools/requirements-art.txt`; it is not linked into or shipped with the game.

The library does not make a collision automatically correct. The release contract remains:

- OpenCV proposes the alpha-derived convex boundary.
- The pipeline converts it to the exact absolute, bottom-left, five-point coordinate format consumed by `PZGObstacle`.
- A human reviews the overlay and removes decorative protrusions from lethal geometry where necessary.
- Art and collision changes are compared with the baseline, deterministic replay, near-miss behavior, and real gameplay.
- The new collision is written to `CompleteGameInfo.plist` only through an explicit reviewed `--apply` operation.
- A collision remap increments `rulesVersion`, re-records the golden replay, and runs device/play tests.

If a future method measurably produces safer boundaries or faster reviewed iteration than OpenCV, it may replace the tracing implementation behind the same command and output contract. The mission is fair, entertaining play and fast reliable content production—not loyalty to a particular library.

### 23.11 Implementation status of sections 21–23 (2026-11-21)

**Built and verified on the iOS simulator**

- **Content registry (23.1–23.5, phases 1–2):** `Source/V2/V2Content.{h,cpp}` reads `v2.presentation` (atlases, animations, dialogues, scenicBeats, contentGroups, contentVersion) from `CompleteGameInfo.plist`. `V2RoadEvents` no longer names an atlas file or a raw frame: it asks for semantic animation ids (`rider_cool`, `flame`, ...) and the registry supplies the frame list and fps. `BarkBook` and every ambient / T-Rex bubble line now come from `presentation.dialogues`; `V2Core.cpp` holds no authored text. The golden replay is still exact after the migration (`checked=26 exact=1 endTick=797/797`).
- **Tooling (23.6, partial):** `tools/v2_registry.py` with `validate-content` (atlas/texture/frame/animation/dialogue/scenic cross-references, duplicate frame ownership), `register-atlas` (dry-run by default, `--apply`, runs `plutil -lint` and the validator) and `contact-sheet`. `Content::validate()` runs the same checks in-app once per process and logs one report. Not built: `register-animation`, `extract-frame`, `migration-report`, and the "version not bumped" check.
- **Versions (23.7):** `v2.contentVersion` = 1. Replay files now carry the modifier and the content version (`V2R fmt rules seed endTick ended modifier content`); analytics `v2_run_seed` and `replay_result` carry `rules` and `content`. App version / acquisition source are not in the events yet.
- **Scenic dinosaurs (21.5, first slice):** `Distant Walker` and `Wall Peek` through `V2::ScenicDirector` (`V2Scenic.{h,cpp}`): cosmetic RNG seeded from the clock (never the run seed), first beat after 25–40 s, 60–120 s cooldown, max 2 beats per 5 minutes, never the same beat twice in a row, suppressed during road events, the T-Rex climax, control locks and ride power-ups, cleared on death/restart. Registered in the plist (`scenicBeats`, group `scenic_v2`, optional). Art is a **silhouette placeholder** generated by `tools/make_v2_scenic.py` (pale cool-blue, no collision, no warning colours). It is clipped to the open sky band above the wall and is behind the HUD and platforms. Seen in the simulator crossing the sky band. `scenic_beat` analytics event added.
- **Settings (21.12 / 21.15, partial):** Spectacle Full / Reduced / Off and Reduced Motion rows in Settings (`V2Presentation`). Spectacle Off removes beats; Reduced and Reduced Motion use a still fade instead of movement. The OS "Reduce Motion" flag is not read yet.

**Not built (needs art, design approval or large work)**

- Production replacement of `v2_events.png`, the obstacle/hurdle refresh and its collision migration (21.9, 21.11, phases 3–4), dialogue portraits and the pre-run dialogue screens (21.6, phase 5), the other three scenic beats, the parallax pass, camera shot library and the single cinematic-ownership authority (21.7, 21.13 Step 3), named layer hosts, actors / portraitSets / uiSkins in the schema, challenge links and ghosts, the 22.3 scorecard analytics (launch-to-control, first-run, retry, D1/D7), fresh-player and returning-player sessions.
- The scenic placement is tuned to the current sewer background (sky band above 75% of screen height). Other levels' backgrounds were not checked.
- Everything from 20.14 still stands: only the replay, the daily-rule tests and the registry/validator are machine-verified; the event play-through matrix and all player evidence are missing.

### 23.12 Follow-up (2026-11-21)

- **Scenic placement corrected:** the dinosaurs are now *behind* the wall, not standing on it. Beat bases sit well below the wall top (`yFrac` 0.655) and the clip line stays at the skyline (`clipYFrac` 0.755), so only the neck, back and head show above the wall. Seen in the simulator (neck rising behind the wall). Tuned to the current sewer background only.
- **Scorecard events (22.3, partial):** `V2Metrics` writes `run_start` (mode: normal / daily_ranked / daily_practice / replay, retry gap), `run_control` (intro seconds, launch-to-control once per process) and `run_end` (cause, distance, events, coins, duration, frame-time buckets <=17 / 25 / 34 / 50 / >50 ms and worst frame). Seen in the simulator for a normal run. Not yet logged: first-event-reached percentage as its own event, abandoned runs (scene exit), app version and acquisition source, share / challenge events, D1/D7 (needs a backend).

### 23.13 Pre-run dialogue vertical slice (2026-11-21, simulator-verified once)

- `V2::DialogueDirector` (`V2Dialogue.{h,cpp}`) shows a start-only exchange before the countdown: dim overlay, two inward-facing portraits (speaker bright, listener dimmed, 2.5% push-in unless Reduced Motion), name plate, one line at a time, tap to advance, auto-advance after 3.2 s, **SKIP always visible**, one cleanup path, world frozen behind it (no ticks), never during replays.
- All content is data in `CompleteGameInfo.plist`: `portraitSets`, `dialogueSequences` (speakers, lines, expressions, trigger, priority, `once` / `session` / `daily` / `always`, version) and `dialogueSettings`. `tools/v2_registry.py validate-content` checks speakers, expressions, portraits and the 40-character limit.
- Sequences from 21.6: `cool_first` (first run ever), `cool_rematch_loss` / `cool_rematch_win` (once per session, from the stored last race result), `trex_first` (after the first T-Rex chase), `daily_first` (first daily run of the day). Analytics: `dialogue_shown`, `dialogue_done`.
- **Placeholder art:** `tools/make_v2_portraits.py` draws flat helmet busts with four expressions. They do not meet the painted-style requirement and must be replaced by approved portraits (same frame ids, so no code change).
- Not done: viewing a skip tap and the abbreviated repeat card on a device, localization review, audio ducking, interruption (app background) test, full-scene "rival two-shot" camera.

### 23.14 Small follow-ups (compiled; not exercised on device)

- OS **Reduce Motion** on iOS now counts as Reduced Motion (`V2PresentationApple.mm`, `UIAccessibilityIsReduceMotionEnabled`); Android still reads only the in-game setting.
- Metrics: `first_event` (seconds after control) and `run_end` with cause `abandoned` when a run never reaches the results screen. Still missing: app version, acquisition source, share/challenge events.

### 23.15 Cinematic authority, ghost, friend challenge, session kit (2026-11-21)

**Cinematic ownership (21.7 / 21.13 Step 3, core done)** - `V2::Cinematic` (`V2Cinematic.{h,cpp}`) is the single owner of dialogue / intro / climax / blast / death / revive with the plan's transition table; death and `forceReset()` (new run, scene exit) always win; a denied request means "skip the cosmetic". Wired into `PZRunDirector` (intro, GO, death show, blast, revive, camera punches), the dialogue director and the T-Rex climax (blast zooms and landing punches are suppressed while the climax owns the screen). 14/14 logic checks pass in-app; a normal run still reaches control. Not done: moving the low-level `cameraTo/cameraPunch` code behind the authority (they are gated, not relocated), the scenic reveal / rival two-shot / climax reveal shots, named layer hosts, app-interruption forced reset tests.

**Personal-best ghost (20.10 #4, local only)** - `V2::GhostRider` draws a translucent copy of your best run on the same daily road (same seed + rules + modifier) from the recorded checkpoints; saved when a daily result beats your best on that date (`ghost_<date>.v2r`). Cosmetic only. Seen saving and loading in the simulator; the on-screen ghost was only faintly visible because the bot's two runs were nearly identical. **Friend ghosts are not possible without a server** (a link cannot carry a replay); a deterministic cross-platform replay test on Android is still missing.

**Friend challenge (20.10 #2-3)** - `V2::Challenge`: versioned link `https://nextfictiongames.com/challenge?v=1&d=&r=&c=&m=&s=&q=&k=` / `dinobike://challenge?...` carrying date, rules, content, modifier, score, medal and an FNV check. The share text now contains the link. iOS: custom URL scheme `dinobike` registered (Info.plist + `application:openURL:` hands the link to the menu, which shows the invite). Everywhere: **FRIEND CODE** button in the Daily screen reads the clipboard once on tap. The invite refuses links with a bad check, a modifier that does not match the date, or another rules version; accepted links play the sender's road as **practice** (daily attempts untouched) and the result screen compares both scores. 7/7 token checks pass; the invite screen was seen in the simulator. **Not done:** `https` universal links / Android App Links (need a hosted association file and an Android intent filter + JNI), Game Center / Play Games leaderboards, any verification of the sender's score (the link is trust-based), and a play-through of a challenge run on a device.

**Player sessions** - none run. `versions/v2/PLAYER_SESSIONS.md` (protocol, observation sheet, questions, gates) and `tools/analyze_sessions.py` (per-player and summary report from `v2_analytics.jsonl`: launch-to-control, first-run outcome, voluntary second run, first-event reach, event outcomes by variant/tier, dialogue / scenic / micro / challenge counts, frame-time) are ready. The decisions that need players (balance, retention gates) stay open.

### 23.16 Links, leaderboards and score signing - code done, setup left to the owner (2026-11-21)

**Code written** (iOS compiles and the signing tests pass in the simulator; the Android and Game Center / Play Games parts are **not compiled or run**):
- **Signed links:** the challenge check is now HMAC-SHA-256 (`V2Crypto`, 16 hex chars, key inside the app). SHA-256 and HMAC test vectors pass. Honest limit: this stops edits, typos and casual forgery; anyone who extracts the key from the binary can mint links. Real verification needs a server that signs or re-simulates results.
- **iOS universal links:** `applinks:nextfictiongames.com` in the entitlements and `application:continueUserActivity:` in `GameAppController.mm` (plus the existing `dinobike://` scheme).
- **Android deep links:** intent filters for `dinobike://challenge` and `https://nextfictiongames.com/challenge` (`autoVerify`) in `AndroidManifest.xml`; `AppActivity` keeps the VIEW intent (`onCreate` / `onNewIntent`) and the game polls it through JNI (`takePendingLink`).
- **Leaderboards:** `V2Leaderboard` facade (ids in `CompleteGameInfo.plist` `v2.leaderboards.iosDaily / androidDaily`, empty = off). iOS: `GameCenter::submitScoreTo / showLeaderboardFor`. Android: `PlayGamesBridge.java` (Play Games v2) + gradle dependency. A ranked daily result is submitted automatically; a LEADERBOARD button appears in the Daily screen once an id is set.
- **Hosted file templates:** `docs/web/.well-known/apple-app-site-association`, `docs/web/.well-known/assetlinks.json`, `docs/web/challenge/index.html` (fallback page when the app is not installed).

**Left for you (cannot be done from the repo):**
1. Serve the three files above from `https://nextfictiongames.com` (AASA with content type `application/json`, no redirect, at `/.well-known/apple-app-site-association`). Fill `REPLACE_WITH_PLAY_APP_SIGNING_SHA256_FINGERPRINT` (Play Console > App integrity, the *app signing* key) and `REPLACE_APP_STORE_ID`.
2. Enable the **Associated Domains** capability for the App ID in the Apple developer portal and regenerate the provisioning profile (the entitlement alone will not sign otherwise).
3. Create the daily leaderboards: a *recurring daily, best score* leaderboard in App Store Connect and one in Play Console; paste the ids into the plist.
4. Android: link a Play Games Services project, add `<meta-data android:name="com.google.android.gms.games.APP_ID" .../>` to the manifest, and confirm `play-services-games-v2` version `20.1.2` exists (change it if Gradle cannot resolve it).
5. Test on devices: tap a link with the app installed and not installed, on both platforms.
6. Score verification beyond the HMAC needs a backend (sign results server-side, or accept a replay and re-simulate headlessly); neither exists. Until then leaderboards and links are trust-based.

### 23.17 Production graphics pass — implemented, release validation still open (2026-11-21)

**Implemented**

- Replaced the flat placeholder `v2_events.png`, `v2_portraits.png` and `v2_scenic.png` art with painted production candidates that follow the existing bike, platform and parallax-background language: cinematic arcade realism, cool sewer light, warm safety accents, readable silhouettes and transparent gutters. The existing atlas dimensions, frame rectangles, frame ids and plist contracts are unchanged.
- Added painted construction-worker STOP/GO poses and dialogue close-ups for both riders. Dialogue code and authored sequence data did not change.
- Replaced the faded barrel (`Obstacles_4_0`) as the first obstacle-quality vertical slice. No other legacy frame in `animation_s0.png` changed.
- Rebuilt the barrel boundary from its alpha with the repository-local OpenCV pipeline, reviewed its overlay, and wrote the five-point inset hull to `CompleteGameInfo.plist`. `rulesVersion` is now 3; the new art/content package is `contentVersion` 2.
- Added `tools/build_v2_production_art.py` and source/provenance under `reskin-automation/art/v2-production/`. The builder repacks approved generated art into the existing atlas contracts and preserves the legacy shadow without cumulative rescaling.
- All runtime lookup still flows through `CompleteGameInfo.plist`; no new sprite sheet, frame name or texture path was hard-coded in gameplay code.

**Verified**

- Registry cross-references, plist syntax, atlas dimensions and transparent frame gutters.
- Barrel logical canvas remained 200×200; old/new alpha bounds are within one pixel and alpha IoU is 0.948. Only its registered atlas rectangle changed.
- The art-pipeline automated tests pass. Same-size atlases mean decoded GPU texture memory does not increase, though compressed install size increases and must be checked in the final package.

**Not release-verified — required next, in order**

1. Re-record the golden replay for `rulesVersion` 3. The bundled rules-2 replay is intentionally invalid after collision geometry changes.
2. Run near-miss, direct-hit, landing and high-speed barrel tests on a device; confirm the collision feels fair rather than merely matching alpha.
3. Review every event, portrait and dinosaur in real gameplay at phone scale, including Reduced Motion / Spectacle Reduced / Off. Correct visual noise, unsafe warning-colour use or weak silhouettes before adding more art.
4. Profile representative low-end devices. Atlas dimensions are stable, but the larger compressed files and more detailed alpha edges may affect load/decode time and overdraw.
5. Replace the remaining faded hurdles one family at a time through the same generate → repack → OpenCV overlay → collision review → version/replay pipeline. Do not mass-replace obstacles before the barrel slice passes gameplay testing.
6. Run the player-session protocol. Art polish is not proof of entertainment or retention; measure first-event reach, voluntary second run, dialogue skips and remembered spectacle moments.

The graphics are credible production candidates, not a finished release. The largest current risk is no longer placeholder style; it is shipping untested collision feel and spectacle density.
