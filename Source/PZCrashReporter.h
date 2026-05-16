#pragma once

namespace PZCrashReporter {
// Installs signal/terminate handlers that write a minimal crash report to a file in the writable directory.
// Safe to call multiple times.
void install();

#if defined(__APPLE__)
// Adds Apple-specific uncaught NSException hook (implemented in ObjC++).
void installAppleHooks();
#endif
} // namespace PZCrashReporter
