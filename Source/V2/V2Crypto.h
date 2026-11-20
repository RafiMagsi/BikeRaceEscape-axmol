//
//  V2Crypto.h  Small SHA-256 / HMAC-SHA-256 (no dependency) used to sign friend-challenge links.
//
#pragma once

#include <string>

namespace V2 {
namespace Crypto {

std::string sha256Hex(const std::string& data);
std::string hmacSha256Hex(const std::string& key, const std::string& message);

} // namespace Crypto
} // namespace V2
