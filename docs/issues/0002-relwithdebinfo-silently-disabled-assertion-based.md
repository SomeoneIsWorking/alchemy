---
id: 2
title: RelWithDebInfo silently disabled assertion-based C tests
status: resolved
symptom: Clang Werror reported assertion-only variables as unused, proving NDEBUG compiled the enbaya and controller checks out of the normal optimized test build
tags: tests,instrument,cmake,ndebug
created: 2026-08-22
updated: 2026-08-22
---

## Root cause


## What was tried / dead ends


## Resolution

### Resolution (2026-08-22)
Root cause: CMake RelWithDebInfo defines NDEBUG, so tests implemented with assert() became empty programs while CTest reported pass. The test targets now compile with -UNDEBUG, and global Clang -Wall -Wextra -Wpedantic -Werror caught both that failure and a controller assertion precedence bug. Optimized and ASan/UBSan CTest each pass 9/9 with assertions active.
