# Patch Status

This snapshot includes:
- C++20 CMake config
- Legacy Box2D include layout fix
- Platform macro compatibility injection
- `__SetIterator` compatibility fix for Axmol 2.11.3
- `PZLegacyInputLayer` inheritance correction

Verify locally:
```bash
grep -n "__SetIterator" Source/PZLegacyCompat.h
```

Expected output contains:
```cpp
using __SetIterator = decltype(std::declval<ax::__Set&>().begin());
using CCSetIterator = __SetIterator;
```
