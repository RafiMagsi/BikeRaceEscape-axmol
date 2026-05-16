#include "PZCrashReporter.h"

#include "PZLegacyCompat.h"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>

#if defined(__APPLE__)
#    include <execinfo.h>
#    include <unistd.h>
#endif

using namespace ax;

namespace {
std::atomic<bool> g_installed{false};
std::string g_crashPath;

static void appendLine(FILE* f, const char* line) {
    if (!f || !line) return;
    std::fputs(line, f);
    std::fputc('\n', f);
    std::fflush(f);
}

static void writeBacktrace(FILE* f) {
#if defined(__APPLE__)
    void* addrs[64];
    int n = backtrace(addrs, (int)(sizeof(addrs) / sizeof(addrs[0])));
    char** symbols = backtrace_symbols(addrs, n);
    if (!symbols) return;
    appendLine(f, "Backtrace:");
    for (int i = 0; i < n; ++i) {
        std::fputs("  ", f);
        std::fputs(symbols[i], f);
        std::fputc('\n', f);
    }
    std::fflush(f);
    std::free(symbols);
#else
    (void)f;
#endif
}

static void writeCrashHeader(FILE* f, const char* reason) {
    appendLine(f, "==== BikeRaceEscapeAxmol crash report ====");
    if (reason) {
        appendLine(f, reason);
    }
    appendLine(f, "Tip: attach this file + Xcode crash log.");
    appendLine(f, "");
}

static void writeTerminateReport() {
    FILE* f = std::fopen(g_crashPath.c_str(), "a");
    if (!f) return;
    writeCrashHeader(f, "std::terminate()");

    if (auto eptr = std::current_exception()) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            std::fputs("Uncaught std::exception: ", f);
            std::fputs(e.what(), f);
            std::fputc('\n', f);
        } catch (...) {
            appendLine(f, "Uncaught non-std exception");
        }
    } else {
        appendLine(f, "No active exception");
    }

    writeBacktrace(f);
    std::fclose(f);
}

static void signalHandler(int sig) {
    // Minimal, best-effort logging. Keep it simple to reduce re-entrancy issues.
    FILE* f = std::fopen(g_crashPath.c_str(), "a");
    if (f) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Signal: %d (%s)", sig, strsignal(sig));
        writeCrashHeader(f, buf);
        writeBacktrace(f);
        std::fclose(f);
    }

    // Re-raise with default handler to preserve crash semantics.
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

static void computeCrashPath() {
    if (!g_crashPath.empty()) return;

    std::string base;
    if (auto* fu = FileUtils::getInstance()) {
        base = fu->getWritablePath();
    }
    if (base.empty()) base = "/tmp/";
    g_crashPath = base + "pz_crash_report.txt";
}
} // namespace

namespace PZCrashReporter {
void install() {
    bool expected = false;
    if (!g_installed.compare_exchange_strong(expected, true)) return;

    computeCrashPath();
#if !defined(NDEBUG)
    AXLOGI("Crash reporter log file: {}", g_crashPath);
#endif

#if defined(__APPLE__)
    installAppleHooks();
#endif

    std::set_terminate([]() {
        writeTerminateReport();
        std::abort();
    });

    // Common crash signals.
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGBUS, signalHandler);
    std::signal(SIGILL, signalHandler);
    std::signal(SIGFPE, signalHandler);
}
} // namespace PZCrashReporter
