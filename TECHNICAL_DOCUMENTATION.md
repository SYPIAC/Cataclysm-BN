# Technical Documentation: Vehicle Grab Ramp Fix

## Summary

Fixed issue #2001 where players could not drag or push vehicles (carts, cannons, etc.) up or down ramps due to the grab being automatically released when crossing Z-levels.

## The Problem

### Root Causes (Two Issues)

**Issue 1**: Grab was released on any Z-level change

In `src/game.cpp:9731`, the code unconditionally released the player's grab whenever the destination Z-level differed from the current Z-level:

```cpp
if( grabbed && dest_loc.z != u.posz() ) {
    add_msg( m_warning, _( "You let go of the grabbed object." ) );
    grabbed = false;
    u.grab( OBJECT_NONE );
}
```

**Issue 2**: Vehicle movement was blocked on Z-level changes

In `src/game.cpp:10698-10701`, the `grabbed_move()` function prevented any grabbed object movement when Z-level changed:

```cpp
if( dp.z != 0 ) {
    // No dragging stuff up/down stairs yet!
    return false;
}
```

### Why It Broke Vehicle Movement on Ramps

1. Player at position (x, y, 0) grabs vehicle at (x+1, y, 0)
2. Player moves onto ramp, destination is (x+1, y, 1) [Z-level change via ramp]
3. **Issue 1**: Code saw `dest_loc.z (1) != u.posz() (0)` and released grab
4. **Issue 2**: Even if grab wasn't released, `grabbed_move()` returned false for `dp.z != 0`
5. `grabbed_veh_move()` was never called, so vehicle didn't move
6. Vehicle stayed at (x+1, y, 0) while player moved to (x+1, y, 1)
7. On next move, validation failed: "Can't find grabbed object"

### Impact

- **Single-tile vehicles** (shopping carts): Could not be dragged across ramps
- **Multi-tile vehicles** (cannons): Got stuck halfway through ramps with parts on different Z-levels
- **Cross-Z-level grabs**: Could not grab vehicles that were on ramps at different Z-levels
- **Silent failures**: Vehicle stayed put while player moved, causing confusing "Can't find grabbed object" errors

## The Solution

### Code Changes

**Fix 1**: Modified the grab release check to allow ramps:

```cpp
// Before: releases grab on any Z-change (ramps AND stairs)
if( grabbed && dest_loc.z != u.posz() ) {

// After: releases grab only on stairs, not ramps
if( grabbed && dest_loc.z != u.posz() && !via_ramp ) {
```

**Fix 2**: Modified `grabbed_move()` to allow vehicle movement on ramps:

Function signature change in `game.h:773`:
```cpp
// Before
bool grabbed_move( const tripoint &dp );

// After
bool grabbed_move( const tripoint &dp, bool via_ramp = false );
```

Implementation change in `game.cpp:10698`:
```cpp
// Before: blocks all Z-level movement
if( dp.z != 0 ) {
    return false;
}

// After: blocks only stairs, not ramps
if( dp.z != 0 && !via_ramp ) {
    return false;
}
```

Call site update in `game.cpp:9872`:
```cpp
// Before
if( grabbed_move( dest_loc - u.pos() ) ) {

// After
if( grabbed_move( dest_loc - u.pos(), via_ramp ) ) {
```

**Files Modified**:
- `src/game.h` - Function signature (1 line)
- `src/game.cpp` - Three locations (3 lines)

**Total Changes**: 4 lines modified across 2 files

### Why This Works

1. **`via_ramp` parameter**: The `walk_move` function already receives this boolean parameter that correctly identifies ramp transitions vs. stair transitions

2. **Existing infrastructure**: The tripoint `grab_point` already stores the Z-component correctly:
   ```cpp
   tripoint grab_point; // in character.h:2619
   ```

3. **Vehicle movement code**: The `grabbed_veh_move()` function in `grab.cpp:213` already handles Z-level transitions:
   ```cpp
   grabbed_vehicle->adjust_zlevel( 1, dp );
   ```

4. **Position validation**: The grab validation uses full tripoint with Z:
   ```cpp
   grabbed_vehicle = veh_pointer_or_null( m.veh_at( u.pos() + u.grab_point ) );
   ```

5. **Flow completion**: With both fixes in place:
   - Grab is NOT released when crossing ramps
   - `grabbed_move()` does NOT return false for ramps
   - `grabbed_veh_move()` IS called and moves the vehicle
   - Vehicle follows player correctly across Z-levels

### Behavior Matrix

| Scenario | via_ramp | Z-change | Grab Released? | Move Blocked? | Result |
|----------|----------|----------|----------------|---------------|--------|
| Flat ground movement | false | No (0→0) | No | No | Works |
| Ramp up | true | Yes (0→1) | **No** (fixed) | **No** (fixed) | Works |
| Ramp down | true | Yes (1→0) | **No** (fixed) | **No** (fixed) | Works |
| Stairs up | false | Yes (0→1) | Yes (intended) | Yes (intended) | Blocked |
| Stairs down | false | Yes (1→0) | Yes (intended) | Yes (intended) | Blocked |

## Related Code

### Ramp Detection

Ramps are detected by terrain flags in `src/map.cpp`:
- `TFLAG_RAMP` - Generic ramp flag
- `TFLAG_RAMP_UP` - Ramp going up
- `TFLAG_RAMP_DOWN` - Ramp going down

The `via_ramp` parameter is set based on terrain analysis during movement.

### Vehicle Z-Level Handling

In `src/grab.cpp:198-225`, the `get_move_dir` lambda handles vehicle movement:

```cpp
grabbed_vehicle->adjust_zlevel( 1, dp );
```

This function calculates the correct Z-level for the vehicle based on the movement direction. With the fixes, this code now gets executed for ramp transitions.

### Grab Position Storage

The grab position is stored as a `tripoint` in `character.h:2619`:

```cpp
tripoint grab_point = tripoint_zero;
```

This always included Z-component, but it was never properly used because:
1. The grab was released before Z-transition occurred (Issue 1)
2. The movement was blocked before vehicle could move (Issue 2)

The fixes enable this existing functionality.

## Testing

### Unit Tests

Created `tests/vehicle_grab_ramp_test.cpp` with 5 test cases:

1. **grab_single_tile_vehicle_up_ramp**: Tests shopping cart dragging up ramp
2. **grab_vehicle_across_zlevel_on_ramp**: Tests grabbing across Z-levels on same ramp
3. **grab_released_on_stairs**: Verifies stairs still block (negative test)
4. **push_multitile_vehicle_up_ramp**: Tests multi-tile vehicles (motorcycles, cannons)
5. **release_and_regrab_vehicle_on_ramp**: Tests grab lifecycle on ramps

### Test Infrastructure

Tests use existing Catch2 framework and follow patterns from:
- `tests/vehicle_ramp_test.cpp` - Ramp terrain setup
- `tests/vehicle_drag_test.cpp` - Vehicle spawning and manipulation

### Manual Testing

See `REPRODUCTION_GUIDE.md` for detailed manual testing instructions.

## Edge Cases Handled

### Multi-Tile Vehicles

Multi-tile vehicles can have parts on different Z-levels during ramp transition. The fixes allow:
- Grab to persist during transition
- Vehicle movement code to properly handle the complex geometry
- All parts to transition together

### Vehicle Stuck on Ramp

Before fixes: Vehicle could get stuck with some parts on z=0 and some on z=1.  
After fixes: Vehicle moves smoothly across ramp with all parts transitioning together.

### Grabbing Across Z-Levels

The `grab_point` tripoint correctly includes Z-offset, so grabbing a vehicle on `(x, y, z+1)` from position `(x-1, y, z)` works correctly because:
1. `grab_point = (1, 0, 1)` is stored
2. Grab is NOT released when moving to same Z-level as vehicle
3. `grabbed_move()` does NOT block the movement
4. `m.veh_at( u.pos() + grab_point )` correctly finds vehicle at different Z-level

### "Can't Find Grabbed Object" Error

This error occurred when:
1. Grab persisted (after Fix 1) but vehicle didn't move (before Fix 2)
2. Player moved to z=1, vehicle stayed at z=0
3. Validation `m.veh_at( u.pos() + grab_point )` looked for vehicle at wrong Z-level
4. Found nothing, triggered "Can't find grabbed object"

With both fixes:
- Vehicle moves with player
- Validation always finds vehicle at correct position
- No error message

## Backwards Compatibility

### No Breaking Changes

- Stairs behavior unchanged (still block dragging)
- Flat ground movement unchanged
- Grab mechanics for furniture unchanged (furniture movement on ramps also works now)
- Vehicle movement on flat ground unchanged

### Performance Impact

Minimal: Only adds two boolean checks (`!via_ramp`) to existing conditionals.

## Future Considerations

### Potential Improvements

1. **Multiple ramps in sequence**: Test dragging vehicles across multiple consecutive ramps (z=0 → z=1 → z=2)

2. **Steep ramps**: Some terrain might have different ramp steepness - ensure movement cost calculations account for this

3. **Vehicle weight limits on ramps**: Consider if very heavy vehicles should have additional restrictions on ramps

4. **Diagonal ramp movement**: Verify zigzag movement patterns work correctly on ramps

5. **Furniture on ramps**: The fix also enables furniture movement on ramps - test this behavior

### Related Issues

- Issue #1978: Original vehicle ramp issues (parent issue)
- Issue #2001: This fix (vehicle grab/drag on ramps)

### Known Limitations

1. **Extremely heavy vehicles**: Very heavy vehicles may still be difficult to push up ramps due to strength requirements (intended behavior)

2. **Ramp width**: Multi-tile vehicles wider than ramp will still have issues (geometry constraint, not grab issue)

3. **Damaged ramps**: Partially destroyed ramps may have unexpected behavior (separate issue)

## References

### Key Files

- `src/game.cpp:9731` - Fix 1 location (grab release check)
- `src/game.cpp:10698` - Fix 2 location (movement block check)
- `src/game.cpp:9872` - Call site update
- `src/game.h:773` - Function signature update
- `src/grab.cpp:101-263` - Vehicle grab movement implementation
- `src/handle_action.cpp:576-624` - Grab action handling
- `src/character.h:2619` - Grab point storage
- `tests/vehicle_grab_ramp_test.cpp` - Test cases

### Related Systems

- Ramp terrain system (`src/map.cpp`, `src/mapdata.h`)
- Vehicle positioning (`src/vehicle.cpp`, `src/vehicle.h`)
- Player movement (`src/game.cpp`)
- Z-level handling (`src/map.cpp`)

## Conclusion

This fix enables proper vehicle grab mechanics across ramps by:
1. Distinguishing between ramps (allow grab) and stairs (block grab) in grab release check
2. Distinguishing between ramps (allow movement) and stairs (block movement) in movement check
3. Leveraging existing Z-level handling infrastructure
4. Making minimal, surgical changes to the codebase (4 lines across 2 files)
5. Maintaining backwards compatibility with non-ramp scenarios

The fix addresses both the root cause of grab release and the movement blocking issue, enabling complete functionality for vehicle movement across ramps.
