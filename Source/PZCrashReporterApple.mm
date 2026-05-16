#include "PZCrashReporter.h"

#include "PZLegacyCompat.h"

#if defined(__APPLE__)
#    import <Foundation/Foundation.h>
#endif

namespace {
#if defined(__APPLE__)
static void writeNSStringToCrashFile(NSString* str) {
    if (!str) return;
    auto* fu = ax::FileUtils::getInstance();
    std::string base = fu ? fu->getWritablePath() : std::string();
    if (base.empty()) base = "/tmp/";
    std::string path = base + "pz_crash_report.txt";

    NSData* data = [[str stringByAppendingString:@"\n"] dataUsingEncoding:NSUTF8StringEncoding];
    NSFileHandle* handle = [NSFileHandle fileHandleForWritingAtPath:[NSString stringWithUTF8String:path.c_str()]];
    if (!handle) {
        [[NSFileManager defaultManager] createFileAtPath:[NSString stringWithUTF8String:path.c_str()] contents:nil attributes:nil];
        handle = [NSFileHandle fileHandleForWritingAtPath:[NSString stringWithUTF8String:path.c_str()]];
    }
    if (!handle) return;
    [handle seekToEndOfFile];
    [handle writeData:data];
    [handle closeFile];
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
        NSString* header = @"==== Session start (Apple) ====";
        NSString* bundleId = [[NSBundle mainBundle] bundleIdentifier] ?: @"(null)";
        NSString* version = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"] ?: @"";
        NSString* build = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"] ?: @"";
        NSString* info = [NSString stringWithFormat:@"BundleId: %@  Version: %@ (%@)", bundleId, version, build];
        writeNSStringToCrashFile(header);
        writeNSStringToCrashFile(info);
    }
    NSSetUncaughtExceptionHandler(&uncaughtNSExceptionHandler);
#endif
}
} // namespace PZCrashReporter
