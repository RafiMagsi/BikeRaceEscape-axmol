# Player sessions kit (PRODUCT_PLAN 20.8 #7, 21.16 Step 9, 22.3)

Nothing here replaces real players. It makes the sessions cheap to run, comparable and honest. **No session has been run yet.**

## Who and how many
- 10 fresh players (never saw the game) and 5 returning players (played a previous build). Mix of ages / casual and bike-game players.
- Same build for everyone: note the build number, `rulesVersion` and `contentVersion` (printed in `v2_analytics.jsonl`: `v2_run_seed.rules/content`).
- 10-15 minutes of free play, then 5 minutes of questions. One tester at a time, on a real phone (include one low-end device). Record the screen if the tester agrees.

## Facilitator rules
1. Say only: "Play the game as you normally would. Tell me what you are thinking out loud. I cannot help you."
2. Do **not** explain events, the daily, the rival, or the controls beyond what the game shows. Do not coach a retry.
3. Note, with a timestamp: first tap on Play, first confusion, every death and what the tester says caused it, every moment of laughter or surprise, every time they skip, every voluntary extra run.
4. Do not prompt a retry. A retry only counts if the tester starts it without being asked (the log shows `retry_gap_s`).
5. After the session copy `v2_analytics.jsonl` (see `tools/analyze_sessions.py` header) and name it `player01.jsonl`...

## Observation sheet (per player)
| Item | Note |
|---|---|
| Understood the core action within the first run? (Y/N, when) | |
| Could state the objective of each event seen (list events) | |
| Noticed and intentionally took a risk route (which) | |
| Any death they called unfair / unexplained (which, why) | |
| Moment they remember (their words) | |
| Noticed a background dinosaur? Mistook it for a hazard? | |
| Dialogue: read, skipped at which line, annoyed on repeat? | |
| Daily challenge: opened? understood three attempts / modifier? | |
| Friend challenge: would send? opened a link? | |
| Voluntary extra runs and why they stopped | |

## Questions after play (do not lead)
1. What was the goal?
2. Which moment do you remember?
3. What caused your last death?
4. Did anything feel unfair or confusing?
5. Which rider or event would you want to see again?
6. Why did you retry or stop?
7. Would you send this challenge to a friend? (and: what would you tell them?)

## Gates (from 22.3 and 20.5; all relative to a baseline build captured first)
- Launch-to-control must not increase versus baseline; unexplained deaths must decrease.
- 8/10 fresh players describe the core action and one memorable event.
- 8/10 can state each event objective after seeing it once; 7/10 notice and intentionally take one risk route.
- 6/10 start another run without being prompted; first-event completion and median runs per first session improve.
- No tester reports an untelegraphed event death; each sees at least four distinct event variants in 10 minutes.
- No scenic beat mistaken for a hazard; frame time budget holds (`ft_gt50` near zero in `run_end`).

## After the sessions
Run `python3 tools/analyze_sessions.py player*.jsonl`, file the observation sheets next to it, and only then decide the next sprint. If attention rises but voluntary retries do not, fix the run loop, not the marketing (22.3).
