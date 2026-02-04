// TODO: BN Port Incomplete - Requires imgui or curses rewrite
// This is a minimal stub to allow compilation
// The full DDA implementation uses imgui which BN doesn't have yet

#include "surroundings_menu.h"
#include "avatar.h"
#include "map.h"
#include "messages.h"
#include "translations.h"

surroundings_menu::surroundings_menu( avatar &, map &, std::optional<tripoint_abs_ms> &, int )
{
    // Stub constructor
}

void surroundings_menu::execute()
{
    // TODO: Implement surroundings menu for BN
    // The DDA version uses imgui which BN doesn't have
    // This needs to be rewritten using curses or wait for BN imgui port
    add_msg( m_info, _( "Surroundings menu not yet implemented in BN (requires imgui port or curses rewrite)" ) );
}
