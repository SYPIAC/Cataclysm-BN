# Technical Documentation: Vehicle Grab Ramp Fix

## Summary

Fixed issue #2001 where players could not drag or push vehicles (carts, cannons, etc.) up or down ramps due to the grab being automatically released when crossing Z-levels.

## The Problem

### Root Cause

In `src/game.cpp:9731`, the code unconditionally released the player's grab whenever the destination Z-level differed from the current Z-level:

```cpp
if( grabbed && dest_loc.z != u.posz() ) {
    add_msg( m_warning, _( "You let go of the grabbed object." ) );
    grabbed = false;
    u.grab( OBJECT_NONE );
}
```

This check was designed to prevent dragging objects up/down stairs, but it was too broad and also triggered when moving across ramps, which should allow dragging.

### Why It Broke Vehicle Movement on Ramps

1. Player at position (x, y, 0) grabs vehicle at (x+1, y, 0)
2. Player moves onto ramp, destination is (x+1, y, 1) [Z-level change via ramp]
3. Code sees `dest_loc.z (1) != u.posz() (0)` and releases grab
4. `grabbed_veh_move()` is never called because grab was already released
5. Vehicle stays at (x+1, y, 0) while player moves to (x+1, y, 1)
6. Vehicle is effectively stuck, especially for multi-tile vehicles

### Impact

- **Single-tile vehicles** (shopping carts): Could not be dragged across ramps
- **Multi-tile vehicles** (cannons): Got stuck halfway through ramps with parts on different Z-levels
- **Cross-Z-level grabs**: Could not grab vehicles that were on ramps at different Z-levels

## The Solution

### Code Change

Modified the condition to check the `via_ramp` parameter before releasing the grab:

```cpp
if( grabbed && dest_loc.z != u.posz() && !via_ramp ) {
    add_msg( m_warning, _( "You let go of the grabbed object." ) );
    grabbed = false;
    u.grab( OBJECT_NONE );
}
```

**File**: `src/game.cpp` line 9731  
**Change**: Added `&& !via_ramp` condition  
**Impact**: Minimal, surgical fix - only one line changed

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

### Behavior Matrix

| Scenario | via_ramp | Z-change | Grab Released? |
|----------|----------|----------|----------------|
| Flat ground movement | false | No (0→0) | No |
| Ramp up | true | Yes (0→1) | **No** (fixed) |
| Ramp down | true | Yes (1→0) | **No** (fixed) |
| Stairs up | false | Yes (0→1) | Yes (intended) |
| Stairs down | false | Yes (1→0) | Yes (intended) |

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

This function calculates the correct Z-level for the vehicle based on the movement direction. With the fix, this code now gets executed for ramp transitions.

### Grab Position Storage

The grab position is stored as a `tripoint` in `character.h:2619`:

```cpp
tripoint grab_point = tripoint_zero;
```

This always included Z-component, but it was never used because the grab was released before Z-transition occurred. The fix enables this existing functionality.

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

Multi-tile vehicles can have parts on different Z-levels during ramp transition. The fix allows the grab to persist during this transition, enabling the vehicle movement code to properly handle the complex geometry.

### Vehicle Stuck on Ramp

Before fix: Vehicle could get stuck with some parts on z=0 and some on z=1.  
After fix: Vehicle moves smoothly across ramp with all parts transitioning together.

### Grabbing Across Z-Levels

The `grab_point` tripoint correctly includes Z-offset, so grabbing a vehicle on `(x, y, z+1)` from position `(x-1, y, z)` works correctly because:
1. `grab_point = (1, 0, 1)` is stored
2. `m.veh_at( u.pos() + grab_point )` correctly finds vehicle at different Z-level

## Backwards Compatibility

### No Breaking Changes

- Stairs behavior unchanged (still block dragging)
- Flat ground movement unchanged
- Grab mechanics for furniture unchanged
- Vehicle movement on flat ground unchanged

### Performance Impact

Minimal: Only adds one boolean check (`!via_ramp`) to existing conditional.

## Future Considerations

### Potential Improvements

1. **Multiple ramps in sequence**: Test dragging vehicles across multiple consecutive ramps (z=0 → z=1 → z=2)

2. **Steep ramps**: Some terrain might have different ramp steepness - ensure movement cost calculations account for this

3. **Vehicle weight limits on ramps**: Consider if very heavy vehicles should have additional restrictions on ramps

4. **Diagonal ramp movement**: Verify zigzag movement patterns work correctly on ramps

### Related Issues

- Issue #1978: Original vehicle ramp issues (parent issue)
- Issue #2001: This fix (vehicle grab/drag on ramps)

### Known Limitations

1. **Extremely heavy vehicles**: Very heavy vehicles may still be difficult to push up ramps due to strength requirements (intended behavior)

2. **Ramp width**: Multi-tile vehicles wider than ramp will still have issues (geometry constraint, not grab issue)

3. **Damaged ramps**: Partially destroyed ramps may have unexpected behavior (separate issue)

## References

### Key Files

- `src/game.cpp:9731` - The fix location
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
1. Distinguishing between ramps (allow grab) and stairs (block grab)
2. Leveraging existing Z-level handling infrastructure
3. Making minimal, surgical changes to the codebase
4. Maintaining backwards compatibility with non-ramp scenarios

The fix addresses the root cause while preserving intended behavior for stairs and other vertical movement methods.
