#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "room.h"

//Test 1: Checks the basic values are set when create
START_TEST(test_room_create_basic)
{
    Room *r = room_create(1, "Name", 5, 10);
    ck_assert_ptr_nonnull(r);

    ck_assert_int_eq(r->id, 1);
    ck_assert_int_eq(r->width, 5);
    ck_assert_int_eq(r->height, 10);

    ck_assert_ptr_nonnull(r->name);
    ck_assert_str_eq(r->name, "Name");

    ck_assert_ptr_ne(r->name, (void *)"Name");

    ck_assert_ptr_eq(r->floor_grid, NULL);
    ck_assert_ptr_eq(r->portals, NULL);
    ck_assert_ptr_eq(r->treasures, NULL);
        
    ck_assert_int_eq(r->portal_count, 0);
    ck_assert_int_eq(r->treasure_count, 0);

    room_destroy(r);
}
END_TEST

//Test 2: Checks that create works with null name
START_TEST(test_room_create_null_name)
{
    Room *r = room_create(1, NULL, 5, 10);
    ck_assert_ptr_nonnull(r);

    ck_assert_ptr_eq(r->name, NULL);
    ck_assert_int_eq(r->width, 5);
    ck_assert_int_eq(r->height, 10);

    room_destroy(r);
}
END_TEST

//Test 3: Checks that multiple rooms can be created
START_TEST(test_room_create_multi)
{
    Room *r1 = room_create(1, "First", 5, 10);
    ck_assert_ptr_nonnull(r1);

    Room *r2 = room_create(2, "Second", 6, 11);
    ck_assert_ptr_nonnull(r2);

    room_destroy(r1);
    room_destroy(r2);
}
END_TEST

//Test 4: Checks that clamps on negative dimensions
START_TEST(test_room_create_clamps_dimensions)
{
    Room *r1 = room_create(1, "First", 0, 10);
    ck_assert_ptr_nonnull(r1);
    ck_assert_int_eq(r1->width, 1);
    ck_assert_int_eq(r1->height, 10);
    room_destroy(r1);

    Room *r2 = room_create(2, "Second", -10, -20);
    ck_assert_ptr_nonnull(r2);
    ck_assert_int_eq(r2->width, 1);
    ck_assert_int_eq(r2->height, 1);
    room_destroy(r2);
}
END_TEST

//Test 5: Checks null returns
START_TEST(test_room_dim_getters_null)
{
    ck_assert_int_eq(room_get_width(NULL), 0);
    ck_assert_int_eq(room_get_height(NULL), 0);
}
END_TEST

//Test 6: Checks valid returns
START_TEST(test_room_dim_getters_valid)
{
    Room *r = room_create(1, "Room", 5, 10);
    ck_assert_ptr_nonnull(r);

    ck_assert_int_eq(room_get_width(r), 5);
    ck_assert_int_eq(room_get_height(r), 10);

    room_destroy(r);
}
END_TEST

//Test 7: Checks overwrite is successful for floor grid setter
START_TEST(test_room_set_floor_grid_overwrite)
{
    Room *r = room_create(1, "Room", 2, 3);
    ck_assert_ptr_nonnull(r);

    bool *g1 = malloc(sizeof(bool) * 6);
    ck_assert_ptr_nonnull(g1);
    for (int i = 0; i < 6; i++){
        g1[i] = true;
    } 

    ck_assert_int_eq(room_set_floor_grid(r, g1), OK);
    ck_assert_ptr_eq(r->floor_grid, g1);

    bool *g2 = malloc(sizeof(bool) * 6);
    ck_assert_ptr_nonnull(g2);
    for (int i = 0; i < 6; i++){
        g2[i] = false;
    } 

    ck_assert_int_eq(room_set_floor_grid(r, g2), OK);
    ck_assert_ptr_eq(r->floor_grid, g2);

    room_destroy(r);
}
END_TEST

//Test 8: Checks the portal setter with invalid perameters
START_TEST(test_room_set_portals_invalid)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    ck_assert_int_eq(room_set_portals(NULL, NULL, 0), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_portals(r, NULL, 1), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_portals(r, NULL, -2), INVALID_ARGUMENT);

    room_destroy(r);
}
END_TEST

//Test 9: Checks the portal setter overwrites
START_TEST(test_room_set_portals_overwrite)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Portal *p1 = malloc(sizeof(Portal) * 1);
    ck_assert_ptr_nonnull(p1);
    p1[0].id = 0;

    p1[0].name = malloc(strlen("First") + 1);
    ck_assert_ptr_nonnull(p1[0].name);
    strcpy(p1[0].name, "First");

    p1[0].x = 1; 
    p1[0].y = 1;
    p1[0].target_room_id = 2;

    ck_assert_int_eq(room_set_portals(r, p1, 1), OK);
    ck_assert_int_eq(r->portal_count, 1);
    ck_assert_ptr_eq(r->portals, p1);

    Portal *p2 = malloc(sizeof(Portal) * 2);
    ck_assert_ptr_nonnull(p2);

    p2[0].id = 0;
    p2[0].name = malloc(strlen("Second") + 1);
    ck_assert_ptr_nonnull(p2[0].name);
    strcpy(p2[0].name, "Second");
    p2[0].x = 0; 
    p2[0].y = 1;
    p2[0].target_room_id = 3;

    p2[1].id = 1;
    p2[1].name = malloc(strlen("Third") + 1);
    ck_assert_ptr_nonnull(p2[1].name);
    strcpy(p2[1].name, "Third");
    p2[1].x = 2; 
    p2[1].y = 1;
    p2[1].target_room_id = 4;

    ck_assert_int_eq(room_set_portals(r, p2, 2), OK);
    ck_assert_int_eq(r->portal_count, 2);
    ck_assert_ptr_eq(r->portals, p2);

    room_destroy(r);
}

//Test 10: Checks treasures setter with invalid perameters
START_TEST(test_room_set_treasures_invalid)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    ck_assert_int_eq(room_set_treasures(NULL, NULL, 0), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_treasures(r, NULL, 1), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_treasures(r, NULL, -1), INVALID_ARGUMENT);

    room_destroy(r);
}
END_TEST

//Test 11: Checks the portal setter overwrites
START_TEST(test_room_set_treasures_overwrite)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Treasure *t1 = malloc(sizeof(Treasure) * 1);
    ck_assert_ptr_nonnull(t1);

    t1[0].id = 0;
    t1[0].name = malloc(strlen("First") + 1);
    ck_assert_ptr_nonnull(t1[0].name);

    strcpy(t1[0].name, "First");

    t1[0].starting_room_id = 1;
    t1[0].initial_x = 1; 
    t1[0].initial_y = 1;
    t1[0].x = 1;
    t1[0].y = 1;
    t1[0].collected = false;

    ck_assert_int_eq(room_set_treasures(r, t1, 1), OK);
    ck_assert_ptr_eq(r->treasures, t1);
    ck_assert_int_eq(r->treasure_count, 1);

    Treasure *t2 = malloc(sizeof(Treasure) * 1);
    ck_assert_ptr_nonnull(t2);

    t2[0].id = 1;
    t2[0].name = malloc(strlen("Second") + 1);
    ck_assert_ptr_nonnull(t2[0].name);

    strcpy(t2[0].name, "Second");

    t2[0].starting_room_id = 1;
    t2[0].initial_x = 2; 
    t2[0].initial_y = 2;
    t2[0].x = 2; 
    t2[0].y = 2;
    t2[0].collected = false;

    ck_assert_int_eq(room_set_treasures(r, t2, 1), OK);
    ck_assert_ptr_eq(r->treasures, t2);
    ck_assert_int_eq(r->treasure_count, 1);

    room_destroy(r);
}

//Test 12: CHecks placing treasure copies name
START_TEST(test_room_place_treasure_copies_name)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Treasure t;
    t.id = 1;
    t.name = "First";
    t.starting_room_id = 1;
    t.initial_x = 1; 
    t.initial_y = 2;
    t.x = 1; 
    t.y = 2;
    t.collected = false;

    ck_assert_int_eq(room_place_treasure(r, &t), OK);
    ck_assert_int_eq(r->treasure_count, 1);

    ck_assert_int_eq(r->treasures[0].id, 1);
    ck_assert_ptr_nonnull(r->treasures[0].name);
    ck_assert_str_eq(r->treasures[0].name, "First");
    ck_assert_ptr_ne(r->treasures[0].name, (void *)"First");

    room_destroy(r);
}
END_TEST

//Test 13: Checks get treasure at works
START_TEST(test_room_get_treasure_at_basic)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Treasure t;
    t.id = 1;
    t.name = "First";
    t.starting_room_id = 1;
    t.initial_x = 0; 
    t.initial_y = 0;
    t.x = 0;
    t.y = 0;
    t.collected = false;

    Treasure t2;
    t2.id = 2;
    t2.name = "Second";
    t2.starting_room_id = 1;
    t2.initial_x = 1; 
    t2.initial_y = 1;
    t2.x = 1; 
    t2.y = 1;
    t2.collected = true;

    ck_assert_int_eq(room_place_treasure(r, &t), OK);
    ck_assert_int_eq(room_place_treasure(r, &t2), OK);
    ck_assert_int_eq(r->treasure_count, 2);

    ck_assert_int_eq(room_get_treasure_at(r, 0, 0), 1);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 1), -1);
    ck_assert_int_eq(room_get_treasure_at(r, 2, 2), -1);
    ck_assert_int_eq(room_get_treasure_at(NULL, 0, 0), -1);

    room_destroy(r);
}
END_TEST

//Test 14: Check if multiple tressures placed in same place
START_TEST(test_room_multiple_treasures_same_pos)
{
    Room *r = room_create(1, "Room", 3, 3);

    Treasure t;
    t.id = 1;
    t.name = "First";
    t.starting_room_id = 1;
    t.initial_x = 0; 
    t.initial_y = 0;
    t.x = 0;
    t.y = 0;
    t.collected = false;

    Treasure t2;
    t2.id = 2;
    t2.name = "Second";
    t2.starting_room_id = 1;
    t2.initial_x = 0; 
    t2.initial_y = 0;
    t2.x = 0;
    t2.y = 0;
    t2.collected = false;

    room_place_treasure(r, &t);
    room_place_treasure(r, &t2);

    ck_assert_int_eq(room_get_treasure_at(r, 0, 0), 1);

    r->treasures[0].collected = true;
    ck_assert_int_eq(room_get_treasure_at(r, 0, 0), 2);

    r->treasures[0].collected = true;
    r->treasures[1].collected = true;
    ck_assert_int_eq(room_get_treasure_at(r, 0, 0), -1);

    room_destroy(r);
}
END_TEST

//Test 15: Test that destroy works on null room
START_TEST(test_room_destroy_null)
{
    room_destroy(NULL);
}
END_TEST

//Test 16: Test basic functionality of get portal dest
START_TEST(test_room_get_portal_destination_basic)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Portal *ps = malloc(sizeof(Portal) * 2);
    ck_assert_ptr_nonnull(ps);

    ps[0].id = 0;
    ps[0].name = malloc(strlen("First") + 1);
    ck_assert_ptr_nonnull(ps[0].name);
    strcpy(ps[0].name, "First");
    ps[0].x = 1;
    ps[0].y = 1;
    ps[0].target_room_id = 2;

    ps[1].id = 1;
    ps[1].name = malloc(strlen("Second") + 1);
    ck_assert_ptr_nonnull(ps[1].name);
    strcpy(ps[1].name, "Second");
    ps[1].x = 2;
    ps[1].y = 2;
    ps[1].target_room_id = 3;

    ck_assert_int_eq(room_set_portals(r, ps, 2), OK);

    ck_assert_int_eq(room_get_portal_destination(r, 1, 1), 2);
    ck_assert_int_eq(room_get_portal_destination(r, 2, 2), 3);
    ck_assert_int_eq(room_get_portal_destination(r, 0, 0), -1);
    ck_assert_int_eq(room_get_portal_destination(NULL, 1, 1), -1);

    room_destroy(r);
}
END_TEST

//Test 17: Test walkability basic features
START_TEST(test_room_is_walkable_basic)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    //Borders are not walkable
    ck_assert_int_eq(room_is_walkable(r, 0, 0), false);
    ck_assert_int_eq(room_is_walkable(r, 1, 1), true);
    ck_assert_int_eq(room_is_walkable(r, 2, 2), false);

    //Out of bounds is not walkable
    ck_assert_int_eq(room_is_walkable(r, -1, 0), false);
    ck_assert_int_eq(room_is_walkable(r, 0, -1), false);
    ck_assert_int_eq(room_is_walkable(r, 3, 0), false);
    ck_assert_int_eq(room_is_walkable(r, 0, 3), false);

    room_destroy(r);
}
END_TEST

//Test 18: Test walkabilty when using a grid
START_TEST(test_room_is_walkable_with_floor_grid)
{
    Room *r = room_create(1, "Room", 5, 5);
    ck_assert_ptr_nonnull(r);

    bool *g = malloc(sizeof(bool) * 25);
    ck_assert_ptr_nonnull(g);

    for (int i = 0; i < 25; i++){
        g[i] = false;
    } 

    //+ shape in middle
    g[1 * 5 + 2] = true;
    g[2 * 5 + 2] = true;
    g[3 * 5 + 2] = true;
    g[2 * 5 + 1] = true;
    g[2 * 5 + 3] = true;

    ck_assert_int_eq(room_set_floor_grid(r, g), OK);

    ck_assert_int_eq(room_is_walkable(r, 1, 2), true);
    ck_assert_int_eq(room_is_walkable(r, 0, 0), false);
    ck_assert_int_eq(room_is_walkable(r, 1, 1), false);
    ck_assert_int_eq(room_is_walkable(r, 2, 2), true);

    room_destroy(r);
}
END_TEST

//Test 19: Test basic functionailty of classify tle
START_TEST(test_room_classify_tile_basic)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    int out = -100;

    ck_assert_int_eq(room_classify_tile(r, 1, 1, &out), ROOM_TILE_FLOOR);
    ck_assert_int_eq(out, -1);

    ck_assert_int_eq(room_classify_tile(r, 0, 0, &out), ROOM_TILE_WALL);
    ck_assert_int_eq(out, -1);

    ck_assert_int_eq(room_classify_tile(r, -1, -1, &out), ROOM_TILE_INVALID);
    ck_assert_int_eq(out, -1);

    Treasure t;
    t.id = 3;
    t.name = "Treasure";
    t.starting_room_id = 1;
    t.initial_x = 1;
    t.initial_y = 0;
    t.x = 1;
    t.y = 0;
    t.collected = false;

    ck_assert_int_eq(room_place_treasure(r, &t), OK);

    out = -100;
    ck_assert_int_eq(room_classify_tile(r, 1, 0, &out), ROOM_TILE_TREASURE);
    ck_assert_int_eq(out, 3);

    Portal *ps = malloc(sizeof(Portal) * 1);
    ck_assert_ptr_nonnull(ps);

    ps[0].id = 0;
    ps[0].name = malloc(strlen("Portal") + 1);
    ck_assert_ptr_nonnull(ps[0].name);
    strcpy(ps[0].name, "Portal");
    ps[0].x = 1;
    ps[0].y = 2;
    ps[0].target_room_id = 4;

    ck_assert_int_eq(room_set_portals(r, ps, 1), OK);

    out = -100;
    ck_assert_int_eq(room_classify_tile(r, 1, 2, &out), ROOM_TILE_PORTAL);
    ck_assert_int_eq(out, 4);

    ck_assert_int_eq(room_classify_tile(r, 1, 0, NULL), ROOM_TILE_TREASURE);
    ck_assert_int_eq(room_classify_tile(r, 1, 1, NULL), ROOM_TILE_FLOOR);
    ck_assert_int_eq(room_classify_tile(r, 2, 1, NULL), ROOM_TILE_WALL);

    room_destroy(r);
}
END_TEST

//Test 20: Test rendering basic functionailty
START_TEST(test_room_render_basic)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Charset cs;
    cs.wall = '#';
    cs.floor = '.';
    cs.player = '@';
    cs.treasure = '$';
    cs.portal = 'O';
    cs.pushable = 'X';

    Treasure t;
    t.id = 1;
    t.name = "Treasure";
    t.starting_room_id = 1;
    t.initial_x = 1;
    t.initial_y = 1;
    t.x = 1;
    t.y = 1;
    t.collected = false;
    ck_assert_int_eq(room_place_treasure(r, &t), OK);

    Portal *ps = malloc(sizeof(Portal) * 1);
    ck_assert_ptr_nonnull(ps);
    ps[0].id = 0;
    ps[0].name = malloc(strlen("Portal") + 1);
    ck_assert_ptr_nonnull(ps[0].name);
    strcpy(ps[0].name, "Portal");
    ps[0].x = 2;
    ps[0].y = 1;
    ps[0].target_room_id = 3;
    ck_assert_int_eq(room_set_portals(r, ps, 1), OK);

    char buf[9];
    ck_assert_int_eq(room_render(r, &cs, buf, 3, 3), OK);

    //Row 1
    ck_assert_int_eq(buf[0], '#');
    ck_assert_int_eq(buf[1], '#');
    ck_assert_int_eq(buf[2], '#');

    //Row 2
    ck_assert_int_eq(buf[3], '#');
    ck_assert_int_eq(buf[4], '$');
    ck_assert_int_eq(buf[5], 'O');

    //Row 3
    ck_assert_int_eq(buf[6], '#');
    ck_assert_int_eq(buf[7], '#');
    ck_assert_int_eq(buf[8], '#');

    room_destroy(r);
}
END_TEST

//Test 21: Test rendering room using null and invalid inputs
START_TEST(test_room_render_invalid_inputs)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    Charset cs;
    cs.wall = '#';
    cs.floor = '.';
    cs.player = '@';
    cs.treasure = '$';
    cs.portal = 'O';
    cs.pushable = 'X';

    char buf[9];

    ck_assert_int_eq(room_render(NULL, &cs, buf, 3, 3), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, NULL, buf, 3, 3), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, &cs, NULL, 3, 3), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, &cs, buf, 4, 3), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, &cs, buf, 3, 4), INVALID_ARGUMENT);

    room_destroy(r);
}
END_TEST

//Test 22: Test get rooms and make sure the start position is the first portal
START_TEST(test_room_get_start_position_with_portals)
{
    Room *r = room_create(1, "Room", 4, 4);
    ck_assert_ptr_nonnull(r);

    Portal *ps = malloc(sizeof(Portal) * 2);
    ck_assert_ptr_nonnull(ps);

    ps[0].id = 0;
    ps[0].name = malloc(strlen("Portal1") + 1);
    ck_assert_ptr_nonnull(ps[0].name);
    strcpy(ps[0].name, "Portal1");
    ps[0].x = 2; 
    ps[0].y = 2;
    ps[0].target_room_id = 2;

    ps[1].id = 1;
    ps[1].name = malloc(strlen("Portal2") + 1);
    ck_assert_ptr_nonnull(ps[1].name);
    strcpy(ps[1].name, "Portal2");
    ps[1].x = 1; 
    ps[1].y = 1;
    ps[1].target_room_id = 3;

    ck_assert_int_eq(room_set_portals(r, ps, 2), OK);

    int x = -1; 
    int y = -1;
    ck_assert_int_eq(room_get_start_position(r, &x, &y), OK);
    ck_assert_int_eq(x, 2);
    ck_assert_int_eq(y, 2);

    room_destroy(r);
}
END_TEST

//Test 23: Test that with no portals the start position is the first walkable tile
START_TEST(test_room_get_start_position_fallback_interior)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    int x = -1, y = -1;
    ck_assert_int_eq(room_get_start_position(r, &x, &y), OK);
    ck_assert_int_eq(x, 1);
    ck_assert_int_eq(y, 1);

    room_destroy(r);
}
END_TEST

//Test 24: Test get start position with null inputs
START_TEST(test_room_get_start_position_invalid_inputs)
{
    Room *r = room_create(1, "Room", 3, 3);
    ck_assert_ptr_nonnull(r);

    int x = -1, y = -1;

    ck_assert_int_eq(room_get_start_position(NULL, &x, &y), INVALID_ARGUMENT);
    ck_assert_int_eq(room_get_start_position(r, NULL, &y), INVALID_ARGUMENT);
    ck_assert_int_eq(room_get_start_position(r, &x, NULL), INVALID_ARGUMENT);

    room_destroy(r);
}
END_TEST



Suite *room_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("TestRoom");

    // Create a test case to group related tests
    TCase *tc = tcase_create("Room");

    // Add individual test functions to the test case
    tcase_add_test(tc, test_room_classify_tile_basic);
    tcase_add_test(tc, test_room_create_basic);
    tcase_add_test(tc, test_room_create_clamps_dimensions);
    tcase_add_test(tc, test_room_create_multi);
    tcase_add_test(tc, test_room_create_null_name);
    tcase_add_test(tc, test_room_destroy_null);
    tcase_add_test(tc, test_room_dim_getters_null);
    tcase_add_test(tc, test_room_dim_getters_valid);
    tcase_add_test(tc, test_room_get_portal_destination_basic);
    tcase_add_test(tc, test_room_get_start_position_fallback_interior);
    tcase_add_test(tc, test_room_get_start_position_invalid_inputs);
    tcase_add_test(tc, test_room_get_start_position_with_portals);
    tcase_add_test(tc, test_room_get_treasure_at_basic);
    tcase_add_test(tc, test_room_is_walkable_basic);
    tcase_add_test(tc, test_room_is_walkable_with_floor_grid);
    tcase_add_test(tc, test_room_multiple_treasures_same_pos);
    tcase_add_test(tc, test_room_place_treasure_copies_name);
    tcase_add_test(tc, test_room_render_basic);
    tcase_add_test(tc, test_room_render_invalid_inputs);
    tcase_add_test(tc, test_room_set_floor_grid_overwrite);
    tcase_add_test(tc, test_room_set_portals_invalid);
    tcase_add_test(tc, test_room_set_portals_overwrite);
    tcase_add_test(tc, test_room_set_treasures_invalid);
    tcase_add_test(tc, test_room_set_treasures_overwrite);

    // Add the test case to the suite
    suite_add_tcase(s, tc);

    // Return the complete suite so main() can run it
    return s;
}