# Thermite Expansion Mod

This mod adds thermite-based items and expands their uses in Cataclysm: Bright Nights.

## What This Mod Adds

### New Items:
- **Thermite Chemical** (`chem_thermite`): A silvery-greenish powder made from aluminum and chromium oxide that burns at extremely high temperatures
- **Thermite Grenade** (`grenade_thermite`): A hand grenade filled with thermite for intense thermal blasts
- **Thermic Lance** (`thermic_lance`): A compact thermal cutting tool that burns thermite to cut through metal

### New Recipes:
- **Thermite Chemical**: Craft from aluminum powder and chromium oxide
- **Thermite Grenade**: Craft from thermite, delay fuse, and a canister
- **Thermic Lance**: Craft from oxygen tank, hose, pipe, and pilot light
- **Incendiary Rocket (Thermite)**: Alternative recipe for incendiary rockets using thermite

### Expanded Tool Uses:
Thermic lances can now be used as alternatives for various crafting operations that require high heat:

- **Forging** (`forging_standard`): Use thermic lances for heating/melting metal (10 charges vs 20 for forge)
- **Blacksmithing** (`blacksmithing_standard`): Basic blacksmithing with thermic lances (10 charges)
- **Metal Removal** (`metal_removal_standard`): Cut through metal and rebar (10 charges)
- **Glassblowing** (`glassblowing_easy` and `glassblowing_standard`): Melt glass for glassblowing (5 charges)

## Balance

Thermic lances require fewer charges than forges (10 vs 20) or match oxy torches, making them an efficient but consumable option for these operations. For glassblowing, they require 5 charges, the same as forges and oxy torches.

## Compatibility

This mod overrides the following base game requirement definitions:
- `forging_standard`
- `blacksmithing_standard`
- `metal_removal_standard`
- `glassblowing_easy`
- `glassblowing_standard`

It should be compatible with most other mods unless they also modify these specific requirements.

## Installation

This mod is included in the game's mod directory. Simply enable it in the mod selection menu when starting a new world.

