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

// Test that tripoint_to_mount preserves Z-level information
TEST_CASE( "vehicle_tripoint_to_mount_preserves_z_level", "[vehicle][zlevel]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    build_test_map( ter_id( "t_pavement" ) );
    
    const tripoint vehicle_pos( 60, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "motorcycle" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Test point at same Z-level as vehicle
    const tripoint same_z( 61, 60, 0 );
    const auto mount_same = veh->tripoint_to_mount( same_z );
    const tripoint recovered_same = veh->mount_to_tripoint( mount_same );
    CHECK( recovered_same.z == same_z.z );
    
    // Test point at different Z-level (vehicle on ramp scenario)
    const tripoint diff_z( 61, 60, 1 );
    const auto mount_diff = veh->tripoint_to_mount( diff_z );
    const tripoint recovered_diff = veh->mount_to_tripoint( mount_diff );
    
    // BUG: Z-level is lost in tripoint_to_mount, so recovered_diff.z will be wrong
    // This test will FAIL with current buggy code
    CHECK( recovered_diff.z == diff_z.z );
}

// Test that allowed_move works correctly across Z-levels
TEST_CASE( "vehicle_allowed_move_with_z_levels", "[vehicle][zlevel]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    build_test_map( ter_id( "t_pavement" ) );
    
    const tripoint vehicle_pos( 60, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "motorcycle" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Convert tripoints at different Z-levels to mount coordinates
    const tripoint pos1( 60, 60, 0 );
    const tripoint pos2( 61, 60, 1 ); // Adjacent tile, one Z-level up
    
    const auto mount1 = veh->tripoint_to_mount( pos1 );
    const auto mount2 = veh->tripoint_to_mount( pos2 );
    
    // BUG: Because Z-level is lost, these mount points may appear much farther
    // apart than they should be, triggering the "Unexpected movement" error
    const bool allowed = veh->allowed_move( mount1, mount2 );
    
    // This should be true for adjacent tiles even at different Z-levels
    CHECK( allowed );
}

// Test check_rotated_intervening with Z-level changes
TEST_CASE( "vehicle_check_rotated_intervening_with_z_levels", "[vehicle][zlevel]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    build_test_map( ter_id( "t_pavement" ) );
    
    const tripoint vehicle_pos( 60, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "car" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Test light pathfinding through vehicle across Z-levels
    const tripoint light_source( 59, 60, 1 ); // One Z-level above
    const tripoint light_dest( 61, 60, 0 );   // Through vehicle at Z=0
    
    const auto mount_from = veh->tripoint_to_mount( light_source );
    const auto mount_to = veh->tripoint_to_mount( light_dest );
    
    // BUG: Z-level difference causes incorrect delta calculation
    // This may trigger debugmsg "Unexpected movement in rotated vehicle vector"
    const bool light_allowed = veh->allowed_light( mount_from, mount_to );
    
    // Result should be valid (not crash with debugmsg)
    // Just checking it doesn't crash is enough for now
    SUCCEED();
}

// Test vehicle on ramp scenario
TEST_CASE( "vehicle_parts_on_ramp_different_z_levels", "[vehicle][zlevel][ramp]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    
    // Create a ramp setup
    for( int x = 55; x < 65; x++ ) {
        for( int y = 55; y < 65; y++ ) {
            here.ter_set( tripoint( x, y, -1 ), ter_id( "t_rock" ) );
            here.ter_set( tripoint( x, y, 0 ), ter_id( "t_pavement" ) );
            here.ter_set( tripoint( x, y, 1 ), ter_id( "t_open_air" ) );
        }
    }
    
    // Create ramp at x=60
    for( int y = 55; y < 65; y++ ) {
        here.ter_set( tripoint( 60, y, 0 ), ter_id( "t_ramp_up_low" ) );
        here.ter_set( tripoint( 61, y, 0 ), ter_id( "t_ramp_up_high" ) );
    }
    
    here.invalidate_map_cache( 0 );
    here.build_map_cache( 0, true );
    
    // Place vehicle partially on ramp
    const tripoint vehicle_pos( 59, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "motorcycle" ), vehicle_pos, 90_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Vehicle should have parts at different Z-levels due to ramp
    // Test that coordinate conversions work correctly
    for( const auto &pt : veh->get_points() ) {
        const auto mount = veh->tripoint_to_mount( pt );
        const tripoint recovered = veh->mount_to_tripoint( mount );
        
        // BUG: Z-level is lost, recovered.z will be wrong for parts on ramp
        CHECK( recovered.z == pt.z );
    }
}

// Test vehicle grab/drag on ramps
TEST_CASE( "vehicle_grab_position_on_ramp", "[vehicle][zlevel][ramp][grab]" )
{
    clear_all_state();
    calendar::turn = calendar::turn_zero;
    
    auto &here = get_map();
    auto &player = get_avatar();
    
    // Create a ramp
    for( int x = 55; x < 65; x++ ) {
        for( int y = 55; y < 65; y++ ) {
            here.ter_set( tripoint( x, y, -1 ), ter_id( "t_rock" ) );
            here.ter_set( tripoint( x, y, 0 ), ter_id( "t_pavement" ) );
            here.ter_set( tripoint( x, y, 1 ), ter_id( "t_open_air" ) );
        }
    }
    
    for( int y = 55; y < 65; y++ ) {
        here.ter_set( tripoint( 60, y, 0 ), ter_id( "t_ramp_up_low" ) );
        here.ter_set( tripoint( 61, y, 0 ), ter_id( "t_ramp_up_high" ) );
    }
    
    here.invalidate_map_cache( 0 );
    here.build_map_cache( 0, true );
    
    // Place vehicle on flat ground next to ramp
    const tripoint vehicle_pos( 58, 60, 0 );
    vehicle *veh = here.add_vehicle( vproto_id( "shopping_cart" ), vehicle_pos, 0_degrees, 0, 0 );
    REQUIRE( veh != nullptr );
    
    // Player stands on ramp (different Z-level)
    player.setpos( tripoint( 60, 60, 0 ) );
    
    // Test that grab position tracking works when player is on ramp
    // BUG: Z-level mismatch in grab position causes issues
    // This is a placeholder - actual grab mechanics would need more setup
    SUCCEED();
}
