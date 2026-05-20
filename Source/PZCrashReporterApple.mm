#include "PZCrashReporter.h"

#include "PZLegacyCompat.h"

#if defined(__APPLE__)
#    import <Foundation/Foundation.h>
#    import <TargetConditionals.h>
#    if TARGET_OS_IPHONE
#        import <UIKit/UIKit.h>
#    endif
#    include <cstdio>
#endif

namespace {
#if defined(__APPLE__)
static NSString* crashFilePath() {
    // Prefer Axmol writable path when available, but work even if called very early.
    std::string base;
    if (auto* fu = ax::FileUtils::getInstance()) {
        base = fu->getWritablePath();
    }
    if (base.empty()) {
        NSString* home = NSHomeDirectory();
        if (home.length > 0) base = std::string(home.UTF8String) + "/Documents/";
    }
    if (base.empty()) base = "/tmp/";
    return [NSString stringWithUTF8String:(base + "pz_crash_report.txt").c_str()];
}

static NSString* runLogPath() {
    std::string base;
    if (auto* fu = ax::FileUtils::getInstance()) {
        base = fu->getWritablePath();
    }
    if (base.empty()) {
        NSString* home = NSHomeDirectory();
        if (home.length > 0) base = std::string(home.UTF8String) + "/Documents/";
    }
    if (base.empty()) base = "/tmp/";
    return [NSString stringWithUTF8String:(base + "pz_run_log.txt").c_str()];
}

static void writeNSStringToCrashFile(NSString* str) {
    if (!str) return;
    NSData* data = [[str stringByAppendingString:@"\n"] dataUsingEncoding:NSUTF8StringEncoding];
    NSString* path = crashFilePath();
    NSFileHandle* handle = [NSFileHandle fileHandleForWritingAtPath:path];
    if (!handle) {
        [[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
        handle = [NSFileHandle fileHandleForWritingAtPath:path];
    }
    if (!handle) return;
    [handle seekToEndOfFile];
    [handle writeData:data];
    [handle closeFile];
}

static void redirectStdIOToRunLog() {
#if defined(NDEBUG)
    // Only redirect output to file in Release builds.
    // In Debug builds, keep output going to Xcode console for live debugging.
    NSString* path = runLogPath();
    const char* cpath = path.UTF8String;
    if (!cpath || !*cpath) return;
    // Truncate per app launch to avoid mixing multiple runs (which causes confusing greps).
    // This still gives you a single file to copy for the latest run.
    FILE* f = std::fopen(cpath, "w");
    if (f) std::fclose(f);
    std::freopen(cpath, "a", stdout);
    std::freopen(cpath, "a", stderr);
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    std::setvbuf(stderr, nullptr, _IOLBF, 0);
#endif
}

static void uncaughtNSExceptionHandler(NSException* exception) {
    NSString* header = @"==== Uncaught NSException ====";
    NSString* name = [NSString stringWithFormat:@"Name: %@", exception.name ?: @"(null)"];
    NSString* reason = [NSString stringWithFormat:@"Reason: %@", exception.reason ?: @"(null)"];
    NSString* stack = [NSString stringWithFormat:@"CallStack:\n%@", [exception.callStackSymbols componentsJoinedByString:@"\n"]];

    writeNSStringToCrashFile(header);
    writeNSStringToCrashFile(name);
    writeNSStringToCrashFile(reason);
    writeNSStringToCrashFile(stack);
}
#endif
} // namespace

namespace PZCrashReporter {
void installAppleHooks() {
#if defined(__APPLE__)
    @autoreleasepool {
        redirectStdIOToRunLog();

        NSDateFormatter* df = [[NSDateFormatter alloc] init];
        df.locale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];
        df.dateFormat = @"yyyy-MM-dd HH:mm:ss.SSS Z";
        NSString* now = [df stringFromDate:[NSDate date]];

        NSString* header = @"==== Session start (Apple) ====";
        NSString* bundleId = [[NSBundle mainBundle] bundleIdentifier] ?: @"(null)";
        NSString* version = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"] ?: @"";
        NSString* build = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"] ?: @"";
        NSString* model = @"";
        NSString* systemName = @"";
        NSString* systemVersion = @"";
#if TARGET_OS_IPHONE
        UIDevice* device = [UIDevice currentDevice];
        model = device.model ?: @"";
        systemName = device.systemName ?: @"";
        systemVersion = device.systemVersion ?: @"";
#else
        NSProcessInfo* pi = [NSProcessInfo processInfo];
        systemName = pi.operatingSystemVersionString ?: @"";
#endif
        NSString* proc = [NSProcessInfo processInfo].processName ?: @"";
        NSString* runlog = runLogPath();

        NSString* info = [NSString stringWithFormat:@"Time: %@", now ?: @""];
        NSString* info2 = [NSString stringWithFormat:@"Process: %@", proc];
        NSString* info3 = [NSString stringWithFormat:@"Device: %@  OS: %@ %@", model, systemName, systemVersion];
        NSString* info4 = [NSString stringWithFormat:@"BundleId: %@  Version: %@ (%@)", bundleId, version, build];
        NSString* info5 = [NSString stringWithFormat:@"RunLog: %@", runlog ?: @""];
        writeNSStringToCrashFile(header);
        writeNSStringToCrashFile(info);
        writeNSStringToCrashFile(info2);
        writeNSStringToCrashFile(info3);
        writeNSStringToCrashFile(info4);
        writeNSStringToCrashFile(info5);
    }
    NSSetUncaughtExceptionHandler(&uncaughtNSExceptionHandler);
#endif
}
} // namespace PZCrashReporter
