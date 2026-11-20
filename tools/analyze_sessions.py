#!/usr/bin/env python3
"""Summarises v2_analytics.jsonl files from playtest sessions (PRODUCT_PLAN 22.3 scorecard).

  python3 tools/analyze_sessions.py player01.jsonl player02.jsonl ...   one file per tester (a file may hold several launches)
  python3 tools/analyze_sessions.py --json out.json *.jsonl

Pull the file from the device:
  iOS simulator / device:  Documents/v2_analytics.jsonl (Xcode > Devices > app container, or `xcrun simctl get_app_container <id> <bundle> data`)
  Android:                 files dir of the app (adb run-as <package> cat files/v2_analytics.jsonl)

It reports what the log can prove (behaviour). It cannot replace the questions players answer after the session.
"""
import argparse, collections, json, statistics, sys

VOLUNTARY_RETRY_S = 30.0     # a next run started within this many seconds of a run end counts as a voluntary retry


def load(path):
    out = []
    for line in open(path, encoding="utf-8", errors="replace"):
        line = line.strip()
        if line.startswith("{"):
            try:
                out.append(json.loads(line))
            except ValueError:
                pass
    return out


def num(x, d=0.0):
    try:
        return float(x)
    except (TypeError, ValueError):
        return d


def analyse(name, ev):
    runs = [e for e in ev if e["e"] == "run_start"]
    ends = [e for e in ev if e["e"] == "run_end"]
    ctl = [e for e in ev if e["e"] == "run_control"]
    r = {"player": name, "runs": len(runs), "completed_runs": len(ends)}
    lc = [num(e.get("launch_to_control_s"), -1) for e in ctl if e.get("launch_to_control_s") not in (None, "-")]
    r["launch_to_control_s"] = lc[0] if lc else None
    r["first_run_duration_s"] = num(ends[0].get("duration_s")) if ends else None
    r["first_run_cause"] = ends[0].get("cause") if ends else None
    gaps = [num(e.get("retry_gap_s"), -1) for e in runs[1:] if e.get("retry_gap_s") not in (None, "-")]
    r["voluntary_second_run"] = bool(gaps) and gaps[0] <= VOLUNTARY_RETRY_S
    r["median_retry_gap_s"] = statistics.median(gaps) if gaps else None
    first_event_runs = sum(1 for e in ev if e["e"] == "first_event")
    r["runs_reaching_first_event"] = first_event_runs
    r["reached_first_event_in_first_run"] = bool(ev and any(e["e"] == "first_event" and e.get("n") == "1" for e in ev))
    ev_out = collections.defaultdict(collections.Counter)
    for e in ev:
        if e["e"] == "road_event_end":
            ev_out[f"{e.get('event')}/v{e.get('variant')}/{e.get('tier')}"][e.get("outcome")] += 1
    r["events"] = {k: dict(v) for k, v in ev_out.items()}
    r["deaths_during_event"] = sum(c.get("aborted_death", 0) for c in ev_out.values())
    r["dialogue_shown"] = sum(1 for e in ev if e["e"] == "dialogue_shown")
    r["dialogue_done"] = sum(1 for e in ev if e["e"] == "dialogue_done")
    r["scenic_beats"] = sum(1 for e in ev if e["e"] == "scenic_beat")
    r["micro_success"] = sum(1 for e in ev if e["e"] == "micro_challenge_end" and e.get("success") == "1")
    r["micro_total"] = sum(1 for e in ev if e["e"] == "micro_challenge_end")
    r["daily_runs"] = sum(1 for e in runs if e.get("mode", "").startswith("daily"))
    r["challenge_opened"] = sum(1 for e in ev if e["e"] == "challenge_open")
    r["challenge_started"] = sum(1 for e in ev if e["e"] == "challenge_start")
    r["cinematic_denied"] = sum(1 for e in ev if e["e"] == "cinematic_denied")
    worst = [num(e.get("ft_worst_ms")) for e in ends]
    r["worst_frame_ms"] = max(worst) if worst else None
    slow = sum(num(e.get("ft_gt50")) for e in ends)
    frames = sum(num(e.get("frames")) for e in ends)
    r["frames_over_50ms_pct"] = round(100.0 * slow / frames, 2) if frames else None
    return r


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--json")
    a = ap.parse_args()
    rows = [analyse(f.rsplit("/", 1)[-1], load(f)) for f in a.files]
    for r in rows:
        print(f"\n== {r['player']}  runs={r['runs']} completed={r['completed_runs']}  launch->control={r['launch_to_control_s']}s")
        print(f"   first run: {r['first_run_duration_s']}s, ended by {r['first_run_cause']}, reached first event: {r['reached_first_event_in_first_run']}")
        print(f"   voluntary 2nd run: {r['voluntary_second_run']} (median retry gap {r['median_retry_gap_s']}s)")
        print(f"   dialogue shown/done: {r['dialogue_shown']}/{r['dialogue_done']}  scenic beats: {r['scenic_beats']}  micro: {r['micro_success']}/{r['micro_total']}")
        print(f"   daily runs: {r['daily_runs']}  challenge opened/started: {r['challenge_opened']}/{r['challenge_started']}  cinematic denied: {r['cinematic_denied']}")
        print(f"   perf: worst frame {r['worst_frame_ms']} ms, >50ms frames {r['frames_over_50ms_pct']}%")
        for k, v in sorted(r["events"].items()):
            print(f"   event {k}: {v}")
    n = len(rows)
    if n:
        vol = sum(1 for r in rows if r["voluntary_second_run"])
        fe = sum(1 for r in rows if r["reached_first_event_in_first_run"])
        variants = collections.Counter()
        for r in rows:
            variants.update({k.split("/v")[0] + "/v" + k.split("/v")[1].split("/")[0] for k in r["events"]})
        print(f"\n== SUMMARY ({n} players)")
        print(f"   voluntary second run: {vol}/{n}  (gate: >= 6/10 of fresh players)")
        print(f"   reached the first road event in run 1: {fe}/{n}")
        print(f"   distinct event variants seen across players: {len(variants)}")
        print("   NOT measurable from logs (ask the player): can they state each objective, which moment they remember, what killed them,")
        print("   anything unfair or confusing, which rider/event they want again, would they send the challenge to a friend.")
    if a.json:
        json.dump(rows, open(a.json, "w"), indent=2)


if __name__ == "__main__":
    main()
