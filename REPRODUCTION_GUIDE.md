# Reproduction Guide for Vehicle Grab/Ramp Issue #2001

This document provides step-by-step instructions for manually reproducing and verifying the vehicle grab mechanics fix across ramps.

## Prerequisites

- Cataclysm: Bright Nights built from source (with the fix applied)
- Debug menu enabled (see below)

## Enabling Debug Menu

1. Modify `data/` configuration or use command-line flag
2. Or press `?` in-game and enable debug mode

## Test Case 1: Single-Tile Vehicle (Shopping Cart) Up Ramp

### Setup

1. Start a new game or load a test world
2. Open debug menu (`~` key)
3. Select "Map" → "Change Terrain" 
4. Create a ramp structure:
   ```
   Z=0: [pavement] [ramp_up_low]
   Z=1: [open_air] [ramp_up_high] [pavement]
   ```
5. Use debug menu "Spawn" → "Vehicle" → "shopping_cart"
6. Place cart on z=0 pavement, adjacent to ramp

### Steps to Reproduce Bug (Without Fix)

1. Stand next to the shopping cart on z=0
2. Press `G` (Grab) and select the cart
3. Verify message: "You grab the shopping_cart"
4. Walk toward the ramp (z=0 → z=0, should work)
5. Try to move onto the ramp low tile (stays z=0)
6. Move onto the ramp high tile (transitions to z=1)
7. **BUG**: Message appears "You let go of the grabbed object"
8. Cart remains on z=0, player is on z=1
9. Cart is now unreachable without going back down

### Expected Behavior (With Fix)

1-5. Same as above
6. Move onto the ramp high tile (transitions to z=1)
7. **FIXED**: Grab is maintained, no message about letting go
8. Cart follows player up the ramp to z=1
9. Player and cart are both on z=1, grab still active

### Verification

- Check player position with debug menu "Info" → "Display stats"
- Check cart position with "Look" command (`x` key)
- Both should be at same z-level after crossing ramp
- Grab should still be active (status line shows "Grabbing")

## Test Case 2: Multi-Tile Vehicle (Cannon) Up Ramp

### Setup

1. Same ramp setup as Test Case 1, but make ramp wider (3-4 tiles)
2. Use debug menu "Spawn" → "Vehicle" → Select a cannon or small multi-tile vehicle
3. Position vehicle perpendicular to ramp direction

### Steps to Reproduce Bug (Without Fix)

1. Stand behind the vehicle
2. Press `G` to grab the vehicle
3. Push vehicle toward ramp
4. As vehicle starts crossing ramp (parts on different Z-levels):
   - **BUG**: Grab releases
   - Vehicle gets stuck halfway through ramp
   - Some parts on z=0, some on z=1
   - Vehicle cannot be moved without teleportation

### Expected Behavior (With Fix)

1-3. Same as above  
4. As vehicle crosses ramp:
   - **FIXED**: Grab maintained throughout transition
   - Vehicle moves smoothly across ramp
   - All parts transition to z=1 together
   - Vehicle is fully functional on z=1

### Notes on Multi-Tile Vehicles

- Ensure vehicle has wheels for easier pushing
- Some vehicles may be too heavy - use debug to increase player strength
- Vehicle must fit through ramp (check vehicle width vs ramp width)

## Test Case 3: Grabbing Vehicle Across Z-Level (On Ramp)

### Setup

1. Create ramp as before
2. Place shopping cart on ramp high section (z=1)
3. Position player on ramp low section (z=0)

### Steps

1. Player is on z=0, cart is on z=1 (both on ramp structure)
2. Press `G` to grab
3. **BEFORE FIX**: May have issues grabbing across Z-level
4. **AFTER FIX**: Should be able to grab cart from different Z-level
5. The grab_point tripoint correctly stores Z-component
6. Validate with `m.veh_at( u.pos() + u.grab_point )` finding the vehicle

## Test Case 4: Stairs Should Still Block Grab

### Setup

1. Create stairs (not ramps) connecting z=0 and z=1
2. Place shopping cart on z=0
3. Player stands next to cart

### Steps

1. Grab the cart on z=0
2. Try to use stairs to move to z=1
3. **EXPECTED**: Message "You can't drag things up and down stairs"
4. **EXPECTED**: Grab is released
5. Cart remains on z=0

### Verification

This ensures the fix doesn't break the intentional stairs behavior. The `via_ramp=false` condition should trigger for stairs.

## Debugging Tips

### Check Current Z-Level

- Debug menu → "Info" → "Display stats"
- Look for "pos: (x, y, z)"

### Check Vehicle Position

- Use "Look" command (`x` key)
- Navigate to vehicle
- Position shown in top-right corner includes Z-level

### Check Grab State

- Status line at bottom should show "Grabbing" when grab is active
- Debug menu can show internal grab_point value

### Common Issues

1. **Ramp not properly constructed**
   - Use debug menu to verify terrain types
   - Ensure ramp_up_low on z=0 and ramp_up_high on z=1
   - Adjacent horizontally, not vertically

2. **Vehicle too heavy**
   - Use debug menu to increase player strength
   - Or spawn lighter vehicle (shopping cart is lightest)

3. **Via_ramp not being set correctly**
   - This would indicate the fix needs adjustment
   - Report if grab releases on ramps but works on other Z-transitions

## Success Criteria

After the fix:
- ✅ Can drag shopping cart up/down ramps without losing grab
- ✅ Can push multi-tile vehicles (cannons) up/down ramps
- ✅ Can grab vehicles on ramps from different Z-level
- ✅ Vehicles don't get stuck in ramps
- ✅ Normal vehicle movement on flat ground still works
- ✅ Grab still releases on stairs (intended behavior)

## Additional Test Scenarios

### Ramp Down

Repeat Test Cases 1-2 but going down a ramp (z=1 → z=0)

### Zigzag Movement on Ramp

1. Grab vehicle on z=0
2. Move diagonally across ramp
3. Verify grab maintained during complex movement

### Heavy Vehicles

Test with armored car or similar heavy vehicles to ensure strength checks still work

### Multiple Ramps

Create series of ramps (z=0 → z=1 → z=2) and test continuous climbing
