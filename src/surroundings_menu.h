#pragma once
#ifndef CATA_SRC_SURROUNDINGS_MENU_H
#define CATA_SRC_SURROUNDINGS_MENU_H

// TODO: BN Port Incomplete - Requires imgui or curses rewrite
// This is a minimal stub to allow compilation
// The full DDA implementation uses imgui which BN doesn't have yet

#include <optional>
#include "coordinates.h"

class avatar;
class map;

enum class surroundings_menu_tab_enum : int {
    items = 0,
    monsters,
    terfurn,
    num_tabs
};

// Minimal stub class - execute() currently does nothing
class surroundings_menu
{
    public:
        surroundings_menu( avatar &u, map &m, std::optional<tripoint_abs_ms> &path_end, int radius );
        void execute();
};

#endif // CATA_SRC_SURROUNDINGS_MENU_H
