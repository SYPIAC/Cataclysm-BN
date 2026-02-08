# Third Fix: Z-Level Movement Detection in Vehicle Grab

## Problem Report (Third Issue)

After implementing the first two fixes, testing revealed a third issue:

> "One-tile sized shopping cart still detaches itself silently when being dragged - it now makes to the top/bottom of the ramp but not any further, detaching itself when player changes Z level walking across the lamp"

## Root Cause Analysis

### Issue 3: Movement Not Recognized on Z-Level Change

**Location**: `src/grab.cpp:139-142`

The `grabbed_veh_move()` function has logic to determine if the player is actually moving the vehicle or just repositioning around it:

```cpp
} else if( std::abs( dp.x + dp_veh.x ) != 2 && std::abs( dp.y + dp_veh.y ) != 2 ) {
    // Not actually moving the vehicle, don't do the checks
    u.grab_point = -( dp + dp_veh );
    return false;  // ← Vehicle doesn't move!
}
```

### The Problem

This condition checks X and Y movement but **ignores Z**. When crossing a ramp:

**Example:**
- Player at (10,5,0), vehicle at (11,5,0), grab_point = (1,0,0)
- Player moves to (11,5,1) via ramp, dp = (1,0,1)
- dp_veh = -grab_point = (-1,0,0)
- Check X: `abs(1 + (-1)) != 2` = `abs(0) != 2` = TRUE
- Check Y: `abs(0 + 0) != 2` = TRUE
- Both conditions TRUE → code thinks player is just repositioning
- Returns false → vehicle doesn't move!
- Next move: grab validation fails because vehicle is at wrong Z-level

### Why This Happens

The logic tries to detect if the player is moving in a way that would actually displace the vehicle:
- `abs(dp.x + dp_veh.x) == 2` means pulling/pushing in X direction
- `abs(dp.y + dp_veh.y) == 2` means pulling/pushing in Y direction
- If neither is true, player is just repositioning

But when `dp.z != 0` (crossing Z-levels), this IS actual movement that should displace the vehicle!

## The Fix

Added Z-check to the condition:

```cpp
} else if( std::abs( dp.x + dp_veh.x ) != 2 && std::abs( dp.y + dp_veh.y ) != 2 && dp.z == 0 ) {
    // Not actually moving the vehicle, don't do the checks
    // Exception: if dp.z != 0, we ARE moving (crossing Z-levels via ramp)
    u.grab_point = -( dp + dp_veh );
    return false;
}
```

Now the condition includes `&& dp.z == 0`:
- When dp.z == 0 (flat ground), original logic applies
- When dp.z != 0 (crossing Z-levels), condition is FALSE
- Code proceeds to actually move the vehicle

## Complete Flow After Fix

1. Player at (10,5,0), vehicle at (11,5,0), grab_point = (1,0,0)
2. Player moves to (11,5,1) via ramp, dp = (1,0,1)
3. walk_move() with via_ramp=true
4. ✅ Check 1 (game.cpp:9731): Grab maintained (via_ramp check)
5. ✅ Check 2 (game.cpp:10698): Movement allowed (via_ramp check)
6. grabbed_veh_move() called with dp = (1,0,1)
7. ✅ Check 3 (grab.cpp:139): Recognized as movement (dp.z != 0 check)
8. Vehicle movement proceeds normally
9. next_grab = -dp = (-1,0,-1)
10. Vehicle displaced, grab_point updated
11. Success!

## Summary of Three Fixes

| Issue | Location | Problem | Fix |
|-------|----------|---------|-----|
| **1** | game.cpp:9731 | Grab released on Z-change | Add `&& !via_ramp` |
| **2** | game.cpp:10698 | Movement blocked on Z-change | Add `&& !via_ramp` |
| **3** | grab.cpp:139 | Z-movement not recognized | Add `&& dp.z == 0` |

All three checks needed to be updated to properly handle Z-level transitions via ramps.

## What This Fixes

- ✅ Shopping carts stay grabbed when crossing ramps
- ✅ Single-tile vehicles move correctly across Z-levels  
- ✅ Player can drag vehicles up/down ramps continuously
- ✅ No "Can't find grabbed object" errors
- ✅ Vehicles follow player through complete ramp transitions

## Remaining Issue

The bicycle collision issue ("dragging bicycle is causing it to collide with the concrete wall below") is a separate timing/collision detection issue that may need additional investigation. The vehicle's Z-level adjustment during transitions might cause premature collision detection with obstacles at the destination Z-level.

## Files Changed

- `src/game.cpp:9731` - Fix 1: Grab release check
- `src/game.cpp:10698` - Fix 2: Movement blocking check
- `src/game.cpp:9872` - Fix 2: Call site update
- `src/game.h:773` - Fix 2: Function signature
- `src/grab.cpp:139` - Fix 3: Movement detection check

**Total**: 5 lines modified across 3 files
