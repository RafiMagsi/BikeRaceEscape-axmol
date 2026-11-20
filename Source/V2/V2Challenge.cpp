#include "V2/V2Challenge.h"
#include "V2/V2Daily.h"
#include "V2/V2Config.h"
#include "V2/V2Content.h"
#include "V2/V2Crypto.h"

#include "PZLegacyCompat.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif
#include <cstdio>
#include <cstdlib>
#include <map>

namespace V2 {
namespace Challenge {

namespace {
std::string g_pending;
const char* kBase = "https://nextfictiongames.com/challenge";

std::string check(const Token& t) {
    const std::string s = "dbe|" + std::to_string(t.version) + "|" + std::to_string(t.date) + "|" + std::to_string(t.rules) + "|" +
                          std::to_string(t.content) + "|" + std::to_string(t.modifier) + "|" + std::to_string(t.score) + "|" + std::to_string(t.medal);
    // HMAC-SHA-256 with a key that ships inside the app, first 16 hex characters. This stops edited scores, typos and casual
    // forgery. It is NOT proof against someone who extracts the key from the binary: only a server that re-simulates or signs
    // results can do that (see PRODUCT_PLAN 23.16).
    static const std::string key = [] {
        static const unsigned char k[] = {0x71, 0x2a, 0x5c, 0x19, 0x83, 0xd4, 0x0e, 0x66, 0xb1, 0x37, 0x9a, 0x42, 0xee, 0x15, 0x68, 0xc3};
        std::string s2;
        for (size_t i = 0; i < sizeof(k); ++i) s2.push_back(static_cast<char>(k[i] ^ (0x5a + i * 7)));
        return "dbe-challenge-v1:" + s2;
    }();
    return Crypto::hmacSha256Hex(key, s).substr(0, 16);
}
}

std::string makeUrl(const Token& t) {
    return std::string(kBase) + "?v=" + std::to_string(t.version) + "&d=" + std::to_string(t.date) + "&r=" + std::to_string(t.rules) +
           "&c=" + std::to_string(t.content) + "&m=" + std::to_string(t.modifier) + "&s=" + std::to_string(t.score) +
           "&q=" + std::to_string(t.medal) + "&k=" + check(t);
}

Verdict parse(const std::string& text, Token& out) {
    out = Token();
    size_t q = text.find("challenge?");
    if (q == std::string::npos) return Verdict::Malformed;
    std::string query = text.substr(q + 10);
    const size_t end = query.find_first_of(" \n\r\t\"'<>");
    if (end != std::string::npos) query.resize(end);
    std::map<std::string, std::string> kv;
    size_t i = 0;
    while (i < query.size()) {
        size_t amp = query.find('&', i);
        if (amp == std::string::npos) amp = query.size();
        const std::string part = query.substr(i, amp - i);
        const size_t eq = part.find('=');
        if (eq != std::string::npos) kv[part.substr(0, eq)] = part.substr(eq + 1);
        i = amp + 1;
    }
    for (const char* k : {"v", "d", "r", "c", "m", "s", "k"}) if (!kv.count(k)) return Verdict::Malformed;
    out.version = std::atoi(kv["v"].c_str());
    out.date = std::atoi(kv["d"].c_str());
    out.rules = std::atoi(kv["r"].c_str());
    out.content = std::atoi(kv["c"].c_str());
    out.modifier = std::atoi(kv["m"].c_str());
    out.score = std::atoi(kv["s"].c_str());
    out.medal = kv.count("q") ? std::atoi(kv["q"].c_str()) : 0;
    out.raw = text;
    if (out.version != 1 || out.date < 20200101 || out.date > 29991231 || out.score < 0) return Verdict::Malformed;
    if (check(out) != kv["k"]) return Verdict::BadChecksum;
    if (Daily::modifierFor(out.date).index != out.modifier) return Verdict::WrongModifier;
    if (out.rules != cfg().rulesVersion) return Verdict::IncompatibleRules;     // different scoring / event rules: scores are not comparable
    return Verdict::Ok;
}

const char* verdictText(Verdict v) {
    switch (v) {
    case Verdict::Ok: return "READY";
    case Verdict::Malformed: return "NOT A VALID CHALLENGE LINK";
    case Verdict::BadChecksum: return "THIS LINK WAS CHANGED OR CUT OFF";
    case Verdict::WrongModifier: return "THIS LINK DOES NOT MATCH THE ROAD";
    case Verdict::IncompatibleRules: return "UPDATE THE GAME: DIFFERENT RULES VERSION";
    }
    return "?";
}

void setPending(const std::string& url) { g_pending = url; }

std::string platformPendingLink() {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    // AppActivity stores the VIEW intent (dinobike:// or https://nextfictiongames.com/challenge) until the game thread asks for it
    return ax::JniHelper::callStaticStringMethod("dev/axmol/app/AppActivity", "takePendingLink");
#else
    return "";                                           // iOS pushes links through application:openURL: / continueUserActivity:
#endif
}

bool hasPending() {
    if (g_pending.empty()) g_pending = platformPendingLink();
    return !g_pending.empty();
}
bool takePending(Token& out, Verdict& verdict) {
    if (g_pending.empty()) return false;
    verdict = parse(g_pending, out);
    g_pending.clear();
    return true;
}

#if !defined(__APPLE__)
std::string clipboardText() { return ""; }
#endif

std::string selfTest() {
    std::string r;
    int fails = 0, checks = 0;
    auto ck = [&](bool ok, const char* what) { ++checks; if (!ok) ++fails; r += std::string(ok ? "  ok   " : "  FAIL ") + what + "\n"; };
    Token t;
    t.date = 20991231;
    t.rules = cfg().rulesVersion;
    t.content = Content::shared().contentVersion();
    t.modifier = Daily::modifierFor(t.date).index;
    t.score = 1234;
    t.medal = 2;
    const std::string url = makeUrl(t);
    Token p;
    ck(parse(url, p) == Verdict::Ok && p.score == 1234 && p.date == t.date && p.medal == 2, "round trip");
    ck(parse("Beat me! " + url + " good luck", p) == Verdict::Ok, "link inside a longer message");
    std::string bad = url;
    bad.replace(bad.find("&s=1234"), 7, "&s=9999");
    ck(parse(bad, p) == Verdict::BadChecksum, "edited score is detected");
    ck(parse(url.substr(0, url.size() - 3), p) == Verdict::BadChecksum, "cut-off link is detected");
    ck(Crypto::sha256Hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256 test vector");
    ck(Crypto::hmacSha256Hex("key", "The quick brown fox jumps over the lazy dog") == "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8", "HMAC-SHA-256 test vector");
    ck(parse("hello", p) == Verdict::Malformed, "random text is rejected");
    Token w = t;
    w.rules = cfg().rulesVersion + 1;
    ck(parse(makeUrl(w), p) == Verdict::IncompatibleRules, "other rules version is refused");
    Token m = t;
    m.modifier = (t.modifier + 1) % 6;
    ck(parse(makeUrl(m), p) == Verdict::WrongModifier, "modifier must match the date");
    r += "CHECKS: " + std::to_string(checks) + "  FAILED: " + std::to_string(fails) + "\n";
    r += fails == 0 ? "RESULT: PASS\n" : "RESULT: FAIL\n";
    return r;
}

} // namespace Challenge
} // namespace V2
