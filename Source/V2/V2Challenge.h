//
//  V2Challenge.h
//  Friend challenge token (PRODUCT_PLAN 20.10 #2-3): a versioned link that carries the road (date -> seed), the rules and content
//  versions, the daily modifier and the sender's score. The receiver plays the SAME road (never ranked, never touching their daily
//  attempts) and the result screen compares both scores. There is NO server: the sender's score is whatever the link says, so a
//  challenge is a friendly dare, not a verified leaderboard result, and it carries no full replay (so no friend ghost).
//
//  Link:  https://nextfictiongames.com/challenge?v=1&d=20261121&r=2&c=1&m=1&s=1234&k=ABCD1234
//         dinobike://challenge?...   (custom scheme, registered on iOS)
//  `k` is an FNV-1a check over the fields: it catches typos and casual edits, it is not security.
//
#pragma once

#include <string>

namespace V2 {
namespace Challenge {

struct Token {
    int version = 1, date = 0, rules = 0, content = 0, modifier = 0, score = 0, medal = 0;
    std::string raw;
};

enum class Verdict { Ok, Malformed, BadChecksum, WrongModifier, IncompatibleRules };

std::string makeUrl(const Token& t);                                  // https link (the shareable form)
Verdict parse(const std::string& textOrUrl, Token& out);              // accepts the full text of a message: finds the link inside
const char* verdictText(Verdict v);

// A link opened by the OS (iOS custom scheme) waits here until the menu can show it.
void setPending(const std::string& url);
bool takePending(Token& out, Verdict& verdict);
bool hasPending();                                                    // also polls the platform (Android intent) once
std::string platformPendingLink();

std::string clipboardText();                                          // iOS: UIPasteboard; elsewhere ""
std::string selfTest();                                               // RESULT: PASS / FAIL

} // namespace Challenge
} // namespace V2
