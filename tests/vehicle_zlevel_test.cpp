#include "catch/catch.hpp"

#include <memory>
#include <string>

#include "avatar.h"
#include "calendar.h"
#include "game.h"
#include "map.h"
#include "map_helpers.h"
#include "point.h"
#include "player_helpers.h"
#include "state_helpers.h"
#include "type_id.h"
#include "vehicle.h"
#include "vpart_position.h"

// Test that tripoint_to_mount_with_z preserves Z-level information
TEST_CASE( "vehicle_tripoint_to_mount_with_z_preserves_z_level", "[vehicle][zlevel]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    build_test_map( ter_id( "t_pavement" ) );
    
    const tripoint vehicle_pos( 60, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "motorcycle" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Test point at same Z-level as vehicle
    SECTION( "same z level" ) {
        const tripoint same_z( 61, 60, 0 );
        const auto mount_same = veh->tripoint_to_mount_with_z( same_z );
        const tripoint recovered_same = veh->mount_to_tripoint( mount_same.xy() );
        
        // The X,Y should match
        CHECK( recovered_same.x == same_z.x );
        CHECK( recovered_same.y == same_z.y );
        // Z-level should be preserved  
        CHECK( mount_same.z == 0 );
    }
    
    // Test point at different Z-level
    SECTION( "different z level" ) {
        const tripoint diff_z( 61, 60, 1 );
        const auto mount_diff = veh->tripoint_to_mount_with_z( diff_z );
        
        // The mount coordinates should preserve Z-level offset
        CHECK( mount_diff.z == 1 );
        
        // Old function loses Z-level
        const auto mount_old = veh->tripoint_to_mount( diff_z );
        // mount_old has no Z component, it's just a point (2D)
    }
}

// Test that allowed_move_with_z works correctly across Z-levels
TEST_CASE( "vehicle_allowed_move_with_z_handles_ramps", "[vehicle][zlevel]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    build_test_map( ter_id( "t_pavement" ) );
    
    const tripoint vehicle_pos( 60, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "motorcycle" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Test adjacent tiles at same Z-level (should be allowed)
    SECTION( "adjacent same z" ) {
        const tripoint pos1( 60, 60, 0 );
        const tripoint pos2( 61, 60, 0 );
        
        const bool allowed = veh->allowed_move_with_z( pos1, pos2 );
        CHECK( allowed );
    }
    
    // Test adjacent tiles at different Z-levels (should be allowed for ramps)
    SECTION( "adjacent different z" ) {
        const tripoint pos1( 60, 60, 0 );
        const tripoint pos2( 61, 60, 1 );
        
        const bool allowed = veh->allowed_move_with_z( pos1, pos2 );
        // This should be allowed for ramps
        CHECK( allowed );
    }
    
    // Test non-adjacent tiles at different Z-levels (should be blocked)
    SECTION( "non-adjacent different z" ) {
        const tripoint pos1( 60, 60, 0 );
        const tripoint pos2( 63, 60, 1 );
        
        const bool allowed = veh->allowed_move_with_z( pos1, pos2 );
        // This should be blocked as it's not adjacent
        CHECK( !allowed );
    }
}

// Test check_rotated_intervening_with_z with Z-level changes
TEST_CASE( "vehicle_check_rotated_intervening_with_z_no_crash", "[vehicle][zlevel]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    build_test_map( ter_id( "t_pavement" ) );
    
    const tripoint vehicle_pos( 60, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "car" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Test that it doesn't crash with Z-level differences
    SECTION( "different z levels dont crash" ) {
        const tripoint light_source( 59, 60, 1 );
        const tripoint light_dest( 61, 60, 0 );
        
        const auto mount_from = veh->tripoint_to_mount_with_z( light_source );
        const auto mount_to = veh->tripoint_to_mount_with_z( light_dest );
        
        // This should not trigger debugmsg or crash
        const bool light_allowed = veh->allowed_light_with_z( mount_from, mount_to );
        
        // Just checking it doesn't crash is enough
        SUCCEED();
    }
}
