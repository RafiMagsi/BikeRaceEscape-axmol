# Dino Bike Escape — Store Events for Version 2 ("What's Coming Next")

Prepared: 2026-11-20
Source: `versions/v2/PRODUCT_PLAN.md` and what is already built in `Source/V2/`.
Purpose: ready-to-paste copy for a **Google Play LiveOps event** and an **Apple App Store In-App Event**, both teasing v2.

> Field limits below are from memory of the Play Console and App Store Connect forms. Re-check the character counters when pasting, because both consoles change their limits and asset sizes from time to time.

---

## 1. The event idea: "The Great Dino Chase Week"

A one-week launch event for v2. Its hook is the daily seeded run: **everybody gets the same road each day, with a different twist.** Fits the plan's pitch: *"Escape a ridiculous dinosaur traffic chase, survive changing road events, and beat your friends' runs."*

Why this event and not a generic "new update" banner:

- It shows what is new, not just that something is new. The week previews real v2 features, and every one of them already exists in the repo.
- It needs no backend or live server. Daily seed, modifier and medals are computed locally from the UTC date (`V2Daily`), so the event works offline and is the same on iOS and Android.
- It gives lapsed players a reason to return on a specific date, and gives new players a reason to install.
- No reward is tied to sharing, per the plan (section 8.1).

### Day-by-day schedule (what the event "shows")

Real dates for the recommended window (Mon 30 Nov – Sun 6 Dec 2026, see section 6). The **modifier** column is computed from `modifierFor()` in `V2Daily.cpp` for those dates, so it is what the game will actually show. The **spotlight** column is marketing copy only:

| Day | Date (UTC) | Daily modifier (from code) | Spotlight for the store copy |
|---|---|---|---|
| 1 | Mon 30 Nov | COIN DROUGHT (x1.3) | Dino Crossing |
| 2 | Tue 1 Dec | ONE LIFE (x1.5) | Pizza Panic |
| 3 | Wed 2 Dec | EVENT RUSH (x1.25) | Cool Kid race |
| 4 | Thu 3 Dec | COIN DROUGHT (x1.3) | Construction Chaos |
| 5 | Fri 4 Dec | STANDARD RUN | Meteor Shower |
| 6 | Sat 5 Dec | ONE LIFE (x1.5) | Grandma Turbo |
| 7 | Sun 6 Dec | EVENT RUSH (x1.25) | Finale: all six events |

**Important:** the game does not currently schedule a specific road event per day. The director picks events from a seeded shuffle bag, so on any day a player may meet any of the six. Either word the store copy as "meet all six road events this week" (safe, already true), or make a small code change so the day's seed opens with its spotlight event (about a day of work, not needed for the store listing). The Play/App Store text in this doc uses the safe wording.

If the release date moves, recompute the modifier column: the rotation is `((year*372 + month*31 + day) % 4)` into `standard, one_life, event_rush, coin_drought`. Never promise a modifier on a date without checking it.

Medals (current thresholds in `V2Daily.cpp`): Bronze 500, Silver 1200, Gold 2500. Participation and medal coins are granted once per day (ranked attempt only).

---

## 2. Google Play — LiveOps event

Play Console path: **Grow users → Promote → Events** (create a "Event / Offer" in the Events section, then pick an event type).

### 2.1 Event name and type

| Field | Value |
|---|---|
| Event name (internal, not shown to users, cannot be changed, max 80) | `v2_great_dino_chase_week_2026_12` (33 chars) |
| Event type | **Special event** (alternative: **Major update** if the v2 build ships on day 1) |

Which type: pick **Major update** if v2 is live on the start date. Pick **Special event** if v2 is live and the week of daily challenges is the point. Do not use **Competition**: there are no real-time rankings or cross-platform leaderboard yet (plan section 8.4), so it would over-promise.

### 2.2 Event details (user-facing)

| Field | Limit | Copy |
|---|---|---|
| Title | 80 | `The Great Dino Chase Week` |
| Short description | 80 | `7 days, 6 wild road events. Same road for everyone. Can you earn Gold?` (70) |
| Long description | 500 | see below |
| Language | | English default; add translations for the languages the store listing already has |
| Start / end | | Start Mon 30 Nov 2026, 00:00 UTC (see section 6). End after 7 days (23:59 UTC on day 7). Play events must start in the future: schedule at least a few days ahead, and submit for review early |
| Event link | | Deep link into the daily challenge screen if available, otherwise the app launcher |
| Country / region | | All countries where the app is published |

**Long description (about 430 characters):**

> The dinosaurs are loose and the road is full of surprises. All week, every rider gets the same daily road with a new twist: one life only, road events twice as often, or a coin drought. Dodge dino crossings, chase the pizza delivery rider, race the Cool Kid, dodge meteors and out-run Grandma Turbo. Beat the road, earn Bronze, Silver or Gold, and come back tomorrow for a new challenge. Free to play. Update to the latest version to join.

### 2.3 Graphics

| Asset | Size / format | Source in repo | Content |
|---|---|---|---|
| Event image (promotional) | 1920 x 1080 (16:9), PNG/JPEG, max 15 MB | start from `store/android/play_games_pc_feature_graphic_1920x1080.png` or `store/rebrand-v2/header-1920x1080.png` | Rider escaping a T-Rex, with a small event banner "7 DAYS · 6 ROAD EVENTS" and the daily modifier icon strip. Keep text inside the centre safe area; Play crops for some placements |
| Promo video (optional) | YouTube URL | not produced yet | See section 4 |

Do not put prices, "FREE", or store ratings in the image. Play rejects those.

### 2.4 Optional: link an offer
Leave out. The plan says no new currencies or reward pressure. The participation coins in the game are enough. If you add an offer later, use the existing coin pack, not a new item.

### 2.5 Submission checklist
- [ ] v2 build is in production (or in a rollout that reaches the start date) before the event goes live.
- [ ] Daily challenge screen and the modifier for each event day verified on a real Android device.
- [ ] Event name, title and description have no unverified claims (no "leaderboard", "multiplayer", "ghost", "friends"; none ship in v2.0).
- [ ] Image meets the size limit and is readable at thumbnail size.
- [ ] Event start is far enough ahead for review.

---

## 3. Apple App Store — In-App Event

App Store Connect path: **Your app → Features (or Distribution) → In-App Events → +**.

| Field | Limit | Value |
|---|---|---|
| Reference name (internal) | 64 | `v2 Great Dino Chase Week` |
| Event name | 30 | `The Great Dino Chase Week` (25) |
| Short description | 50 | `7 days. 6 wild road events. Go for Gold.` (40) |
| Long description | 120 | `One road for everyone each day, with a new twist. Dodge dinos, race Grandma Turbo, earn medals.` (about 96) |
| Badge | | **Major Update** if it matches the v2 release; otherwise **Special Event** (avoid **Challenge** or **Competition**: no head-to-head or leaderboard) |
| Priority | | High |
| Purchase / availability | | Free to play |
| Deep link | | Daily challenge, or the app launch URL |
| Publish start | | At least a few days before the event starts, so it shows in search and on the product page before the event goes live |
| Event start / end | | 7 days from release (event duration up to 31 days is allowed) |
| Territories | | All |
| Primary / secondary language | | English (add other store languages you already ship) |

### Event media

| Asset | Size | Notes |
|---|---|---|
| Event card image | 1920 x 1080 (16:9) | Same art as Play, but follow Apple's text-safe margins; keep text minimal because Apple overlays its own name and badge |
| Event detail page image | 1080 x 1920 (9:16) | Portrait key art; reuse `store/rebrand-v2/listing/01_escape.png` as the base |
| Event video (optional) | 1080p, 16:9 or 9:16, up to 30 s | See section 4 |

Apple limits: up to 10 events approved and 5 published at the same time. Events are reviewed like app metadata, so submit them early. Keep the wording free of Android, Google Play or any competitor reference.

---

## 4. Optional 20–30 s teaser video (both stores)

| Time | Shot | Source |
|---|---|---|
| 0–3 s | Rider coughing on the bike; sign says "DINO CROSSING" | Dino Crossing event |
| 3–8 s | Small dinos cross, big straggler appears, rider jumps | same |
| 8–13 s | Cool Kid overtake meter, PLAYER vs RIVAL | Cool Kid race |
| 13–18 s | Meteor shadows, near-miss bonus | Meteor Shower |
| 18–23 s | Grandma Turbo rockets past, coins drop | Grandma Turbo |
| 23–28 s | Daily screen: modifier + Gold medal | Daily challenge |
| 28–30 s | Logo: "The Great Dino Chase Week" | |

Capture on the iOS simulator (see the existing build and screenshot recipe) with the QA bot, then add captions in an editor. No device frames, and no mention of unreleased features.

---

## 5. Follow-up events (once Phase 2 and 3 of the plan are shipped)

| Event | Store type | Needs first |
|---|---|---|
| "Friend Challenge Weekend" | Challenge / Special event | Seeded friend challenge links, share cards (plan 8.1, 8.2) |
| "Ghost Race Night" | Competition | Golden-replay determinism spike, ghosts (plan 8.3) |
| "T-Rex Boss Escape" | New content / Major update | Boss set piece (plan 5.3 event 6) |
| Weekly league season | New season | Protected leaderboard backend (plan 7.2, 8.4) |

Reuse the same event-card art template so the series reads as one brand.

---

## 6. Release date vs event start date

**They are two different dates. The release must be live before the event starts, never after.** A store event is only a listing: it does not ship the game or turn anything on by itself. The in-game week runs from the UTC date, so a player on an old build sees nothing of it.

| Date | What it is | Rule of thumb |
|---|---|---|
| Release date | v2 build goes live on both stores | Submit first; review time is outside your control |
| Event publish date (Apple) | The event becomes visible on the product page and in search | Up to 14 days before the event start |
| Event start date | The event opens in the store and in-app | Must be after v2 is actually available to most players |
| Event end date | The event closes | Start + 7 days |

### How the stores handle it

- **Google Play.** Events can be tied to a planned app release: you pick an estimated or actual launch date and time zone and can change it later. Google says event approval can take up to 4 days, so submit at least 4 days before the start (14 days if you request featuring, not needed here). App updates themselves can take hours up to about 7 days to review, so Google recommends a buffer of about a week between submitting a build and the date it must be live.
- **Apple.** An event has a separate publish date (up to 14 days before the start). After approval you can still change the start time, end time and publish date while they are in the future; once the event has started only the end date can change. Event metadata is reviewed, so submit it early.
- I could not confirm from public docs whether either store rejects a start time already in the past. Do not rely on it: always schedule in the future.

### Recommended timeline (the game is ready tomorrow, Sat 21 Nov 2026)

| Date | Action |
|---|---|
| Sat 21 Nov | Submit v2 to App Store review and to Play (production, staged rollout 20% then 100%, or managed publishing). Create both store events as drafts. |
| Sun 22 – Wed 25 Nov | Builds approved and live. Check the daily challenge on one real iPhone and one real Android phone. Submit both events for review (Play needs at least 4 days). |
| Thu 26 Nov | Apple event publish date (visible on the product page). Play event approved. |
| **Mon 30 Nov** | **Event starts, 00:00 UTC.** Daily challenge week opens. |
| Sun 6 Dec | Event ends 23:59 UTC. Post results / medals and tease the next event. |

Why not start the event the same day as the release: staged rollout means only a fraction of players have v2 for the first days, and a review delay would leave you with a live event and an old build. A 9-day gap leaves room for one rejected build and one resubmission. If you must start earlier, the earliest safe start is the day **after** both stores show v2 as fully live, and not before Thu 26 Nov.

Do not set the event start to the day of release unless you have already seen the update live in both stores.

---

## 7. Leaderboard for the event

### 7.1 Decision

**Use a separate Daily Challenge leaderboard. Do not reuse the existing all-time distance leaderboard.**

- The existing board stores distance. The daily run scores `(distance + 2*coins + 25*events) x modifier`, a different number. Mixing them breaks both boards.
- The plan (section 8.4) says to keep boards separate per rules version. The code has `kRulesVersion = 1`; if scoring changes, create a new board, do not rewrite the old one.
- Players on the same date play the same road, so a daily board is fair. An all-time distance board is not.

**Do you need it for the event?** No. The event works with medals only. Do not delay v2 for it. But it makes the event much stronger and gives the week a "who won today" story, so add it as soon as it is ready and mention it in store copy only after it is live and tested.

### 7.2 Recommended boards

| Platform | Boards to create | Why |
|---|---|---|
| Android (Play Games Services) | **1** board: "Daily Challenge" | Play Games automatically offers Daily, Weekly and All-time views for every leaderboard, so one board covers the daily and event-week rankings. |
| iOS (Game Center) | **2** recurring boards: "Daily Challenge" (24 h) and "Chase Week" (7 days) | Game Center boards are either classic (permanent) or recurring (reset on a schedule). Two recurring boards give the same daily and weekly views. |

Both stores keep a separate list per platform. The plan already says a cross-platform leaderboard needs a protected backend; that is out of scope here. Say "iOS and Android rankings are separate" in any copy that mentions it.

### 7.3 Suggested IDs

| Board | Game Center ID | Play Games |
|---|---|---|
| Daily Challenge (rules v1) | `dinobike.daily.v1` | Play Console generates the ID (looks like `CgkI...`); copy it into the app |
| Chase Week (iOS only) | `dinobike.chaseweek.2026w49` | not needed |

Use your real bundle prefix if different. IDs cannot be reused, so include a version.

### 7.4 Setup: Google Play Games Services (Android)

Menu names can differ slightly between console versions; the path below is from Google's current docs.

1. **Play Console → Grow → Play Games Services → Setup and management → Configuration.** Create the Play Games Services project for the app (or open it if it exists). Fill in the game details (name, description, 512x512 icon).
2. **Link the Android app and add credentials.** Add an Android credential for the app's package name using the SHA-1 of the **Play app signing** key (Play Console → Test and release → App integrity → App signing), not only your upload key. Add the debug key SHA-1 too for local testing.
3. **Leaderboards → Create leaderboard.** Name: `Daily Challenge`. Format: numeric (integer, no decimals). Ordering: larger is better. Optional icon. Leave score limits empty, or set a sane maximum.
4. **Copy the leaderboard ID.** Daily, weekly and all-time views are automatic; you do not create extra boards.
5. **Testers.** Add your Google accounts under Testers, because unpublished boards are only visible to testers.
6. **Publish** the Play Games Services configuration (this publishes the draft boards with it). Do this a few days before the event, not on the start date.
7. **Code:** add the Play Games Services v2 SDK, call sign-in at startup (automatic sign-in), submit with the board ID, and open the leaderboard UI. Details in 7.6.

### 7.5 Setup: Apple Game Center (iOS)

1. **App Store Connect → your app → Services → Game Center** (called "Features → Game Center" in some versions). Enable Game Center if it is not enabled; the app already uses it for the distance board.
2. **Leaderboards → +.** Create:
   - `Daily Challenge`: type **Recurring**, ID `dinobike.daily.v1`, score format Integer, sort High to Low, submission type **Best Score**, start `2026-11-30 00:00 UTC`, duration 1 day, recurrence every 1 day.
   - `Chase Week`: type **Recurring**, ID `dinobike.chaseweek.2026w49`, same format, start `2026-11-30 00:00 UTC`, duration 7 days, no repeat.
3. **Localization.** Add at least English: name and score format label (for example "points").
4. **Attach to a version.** New leaderboards must be included with an app version submission or approved Game Center configuration before they appear to players. Check the Game Center section for the "add to version" or release prompt and include it when submitting v2 or v2.1.
5. **Test in sandbox** on a real device signed into a sandbox Game Center account before submission.

I could not verify the exact fields of recurring leaderboards from public docs in this session (Apple's help text for them was not returned). Confirm the field names and the time zone setting in App Store Connect when you create them. The important requirement is that the daily reset is at **00:00 UTC**, to match the game's `dateKeyNow()` (UTC).

### 7.6 What the code needs (not built yet)

Current state, from the repo:

- iOS: `PZGameCenterApple.mm` authenticates, shows the board UI and submits a score, but only to **one** leaderboard ID (set via `setLeaderboardId` or the Info.plist key `PZGameCenterLeaderboardId`).
- Android: `PZGameCenterStub.cpp` is a no-op for every call. There is no Play Games bridge, no Play Games dependency in `build.gradle`, and no Play Games app ID in the manifest.
- `V2Daily` already stores each finished daily result in a local upload queue (`pendingUploads()` / `markUploaded()`) so an offline result is not lost.

Work list:

| # | Task | Platform | Size |
|---|---|---|---|
| 1 | Add `submitScore(boardId, score)` and `showLeaderboard(boardId)` overloads to `PZ::GameCenter` | shared | small |
| 2 | iOS: call them with the daily and week IDs; drain `V2Daily::pendingUploads()` after sign-in and on result screen; mark uploaded on success | iOS | small |
| 3 | Android: `PlayGamesBridge.java` (like the existing `ShareBridge.java`) + JNI glue: auto sign-in, submit score, show leaderboard | Android | medium (1-2 days) |
| 4 | Android: add the Play Games v2 dependency and the `com.google.android.gms.games.APP_ID` manifest meta-data (project ID from the Play Games configuration) | Android | small |
| 5 | Daily result screen: "Leaderboard" button next to medal; hidden if not signed in or the board is not configured | both | small |
| 6 | Only submit **ranked** results (the first finished run of the day), never practice runs | shared | trivial |

Submit rules: score is an integer, only ranked daily results, one per UTC day, retry from the queue after failures, never block the result screen on the network.

Risks: Play Games Services setup and review is the longest step (credentials, publishing). Allow at least 3 to 4 days before the event start.

### 7.7 Best option, given the tomorrow release

1. **Ship v2 tomorrow without leaderboard changes.** Event copy mentions medals only (this is already how sections 2 and 3 are written).
2. **Build items 1-6 on a v2.1** in the same week and release by Thu 26 Nov at the latest.
3. If v2.1 is live and tested before 30 Nov, add one line to the event text: "Climb the daily leaderboard." If not, the event still runs, and the leaderboard joins the next event.

I can do the code work in 7.6 next if you want it (iOS first, then the Android bridge).

---

## 8. Open questions for you

1. Confirm the release plan: submit tomorrow (Sat 21 Nov), event starts Mon 30 Nov. Or pick another date and I will recompute the modifier table.
2. Do you want the leaderboard in v2 (delays release by about 2 days for Android) or in v2.1 (recommended)?
3. Which Apple bundle prefix and Play Games project should the board IDs use?
4. Which store languages should the event be translated into?
5. Do you want the "spotlight event per day" code change, or keep the "all six events this week" wording?
