# Patch 06 — Legacy touch set adapter

Build log `axmol-build-05.log` showed that Axmol 2.11.3 does not expose `ax::__Set`.
The previous iterator-only compatibility alias therefore still failed in every translation unit
including `PZLegacyCompat.h`.

This patch:
- Adds a transient `ax::__Set` compatibility wrapper backed by `std::vector<ax::Ref*>`
- Preserves old APIs used by the game: `addObject`, `anyObject`, `count`, `begin`, `end`
- Keeps legacy aliases: `__Set`, `CCSet`, `__SetIterator`, `CCSetIterator`
- Allows `PZLegacyInputLayer` to keep forwarding Axmol `EventListenerTouchAllAtOnce`
  events into old Cocos2d-x 2.x-style `ccTouches*` methods.
