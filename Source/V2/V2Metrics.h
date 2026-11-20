//
//  V2Metrics.h
//  Session / run scorecard events (PRODUCT_PLAN 22.3): launch-to-control, run start / control / end, retry gap, per-run frame-time
//  buckets. Everything is written through V2::Analytics (JSONL file sink today, a provider adapter later). No gameplay reads
//  these values; they are measurement only.
//
#pragma once

#include <string>

namespace V2 {
namespace Metrics {

void markLaunch();                                   // process start (call once, early)
void runStarted(const std::string& mode);            // normal | daily_ranked | daily_practice | replay
void controlGranted();                               // GO: the rider can be controlled (first one logs launch-to-control)
void firstEvent();                                   // the first road event of the run started (once per run)
void frame(float realDt);                            // every rendered frame while a run is live
void runEnded(int distance, int eventsCleared, int coins, const std::string& cause);

} // namespace Metrics
} // namespace V2
