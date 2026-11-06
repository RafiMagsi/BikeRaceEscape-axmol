//
//  PZLinks.h
//  Opens a web page for the player (privacy policy). iOS: an in-app Safari sheet (SFSafariViewController); Android: the
//  system browser (ACTION_VIEW, no extra permission); other platforms: Application::openURL.
//
#pragma once

#include <string>

namespace PZLinks {

// Returns false when `url` is empty / not http(s).
bool openUrl(const std::string& url);

} // namespace PZLinks
