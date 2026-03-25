#include <check.h>
#include <stdlib.h>
#include "player.h"
#include "room.h"
#include "world_loader.h"
#include "game_engine.h"
#include "types.h"

#define STARTER_INI "../assets/starter.ini"

//Test 1: Test create and destory for the engine
START_TEST(test_engine_create_destroy_basic)
{
    GameEngine *eng = NULL;

    Status status = game_engine_create(STARTER_INI, &eng);
    ck_assert_int_eq(status, OK);
    ck_assert_ptr_nonnull(eng);

    game_engine_destroy(eng);

    game_engine_destroy(NULL);
}
END_TEST

//Test 2: Test creating with null inputs
START_TEST(test_engine_create_invalid_inputs)
{
    GameEngine *eng = NULL;

    Status status = game_engine_create(NULL, &eng);
    ck_assert_int_eq(status, INVALID_ARGUMENT);

    status = game_engine_create(STARTER_INI, NULL);
    ck_assert_int_eq(status, INVALID_ARGUMENT);
}
END_TEST

//Test 3: Test basic functions of get player
START_TEST(test_engine_get_player_basic)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    const Player *p = game_engine_get_player(eng);
    ck_assert_ptr_nonnull(p);

    game_engine_destroy(eng);
}
END_TEST

//Test 4: Test get player with null engine
START_TEST(test_engine_get_player_null)
{
    ck_assert_ptr_eq(game_engine_get_player(NULL), NULL);
}
END_TEST

//Test 5: Test basic functionality of get room count
START_TEST(test_engine_get_room_count_basic)
{
    int count = 0;

    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    Status status= game_engine_get_room_count(eng, &count);
    ck_assert_int_eq(status, OK);
    ck_assert_int_ne(count, 0);

    game_engine_destroy(eng);
}
END_TEST

//Test 6: Test get room count with bad inputs
START_TEST(test_engine_get_room_count_invalid_inputs)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    ck_assert_int_eq(game_engine_get_room_count(NULL, NULL), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_get_room_count(eng, NULL), NULL_POINTER);

    game_engine_destroy(eng);
}
END_TEST

//Test 7: Test get dimensions with basic functionality
START_TEST(test_engine_get_room_dimensions_basic)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    int w = 0;
    int h = 0;

    Status status= game_engine_get_room_dimensions(eng, &w, &h);
    ck_assert_int_eq(status, OK);
    ck_assert_int_ne(w, 0);
    ck_assert_int_ne(h, 0);

    game_engine_destroy(eng);
}
END_TEST

//Test 8: test get dimensions with bad input
START_TEST(test_engine_get_room_dimensions_invalid_inputs)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    int w = 0;
    int h = 0;

    ck_assert_int_eq(game_engine_get_room_dimensions(NULL, &w, &h), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_get_room_dimensions(eng, NULL, &h), NULL_POINTER);
    ck_assert_int_eq(game_engine_get_room_dimensions(eng, &w, NULL), NULL_POINTER);

    game_engine_destroy(eng);
}
END_TEST

//Test 9: Test render current room basic functions
START_TEST(test_engine_render_current_room_basic)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    char *out = NULL;
    Status status= game_engine_render_current_room(eng, &out);
    ck_assert_int_eq(status, OK);
    ck_assert_ptr_nonnull(out);
    ck_assert_int_ne((int)strlen(out), 0);

    ck_assert_ptr_nonnull(strchr(out, '\n'));

    free(out);
    game_engine_destroy(eng);
}
END_TEST

//Test 10: Test render room with bad inputs
START_TEST(test_engine_render_current_room_invalid_input)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    ck_assert_int_eq(game_engine_render_current_room(NULL, NULL), INVALID_ARGUMENT);

    game_engine_destroy(eng);
}
END_TEST

//Test 11: Test render room basic functions
START_TEST(test_engine_render_room_basic)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    int *ids = NULL;
    int count = 0;
    ck_assert_int_eq(game_engine_get_room_ids(eng, &ids, &count), OK);
    ck_assert_ptr_nonnull(ids);
    ck_assert_int_ne(count, 0);

    char *out = NULL;
    Status status= game_engine_render_room(eng, ids[0], &out);
    ck_assert_int_eq(status, OK);
    ck_assert_ptr_nonnull(out);
    ck_assert_int_ne((int)strlen(out), 0);

    free(out);
    free(ids);
    game_engine_destroy(eng);
}
END_TEST

//Test 12: Test render rooms with bad inputs
START_TEST(test_engine_render_room_invalid_inputs)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    char *out = NULL;

    ck_assert_int_eq(game_engine_render_room(NULL, 0, &out), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_render_room(eng, 0, NULL), NULL_POINTER);

    game_engine_destroy(eng);
}
END_TEST

//Test 13: Test render room with bad id
START_TEST(test_engine_render_room_invalid_id)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    char *out = NULL;
    Status status= game_engine_render_room(eng, -1, &out);
    ck_assert_int_eq(status, GE_NO_SUCH_ROOM);
    ck_assert_ptr_eq(out, NULL);

    game_engine_destroy(eng);
}
END_TEST

//Test 14: Test get room with id basic fnctn
START_TEST(test_engine_get_room_ids_basic)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    int *ids = NULL;
    int count = 0;

    Status status= game_engine_get_room_ids(eng, &ids, &count);
    ck_assert_int_eq(status, OK);
    ck_assert_ptr_nonnull(ids);
    ck_assert_int_ne(count, 0);

    free(ids);
    game_engine_destroy(eng);
}
END_TEST

//Test 15: Test get room ids with bad inputs
START_TEST(test_engine_get_room_ids_invalid_inputs)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    int *ids = NULL;
    int count = 0;

    ck_assert_int_eq(game_engine_get_room_ids(NULL, &ids, &count), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_get_room_ids(eng, NULL, &count), NULL_POINTER);
    ck_assert_int_eq(game_engine_get_room_ids(eng, &ids, NULL), NULL_POINTER);

    game_engine_destroy(eng);
}
END_TEST

//Test 16: Test move player with bad inputs
START_TEST(test_engine_move_player_invalid_inputs)
{
    ck_assert_int_eq(game_engine_move_player(NULL, DIR_NORTH), INVALID_ARGUMENT);

    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    ck_assert_int_eq(game_engine_move_player(eng, (Direction)100), INVALID_ARGUMENT);

    game_engine_destroy(eng);
}
END_TEST

//Test 17: Test move player all directions
START_TEST(test_engine_move_player_major)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    Status status;

    status = game_engine_move_player(eng, DIR_NORTH);
    ck_assert(status == OK || status== ROOM_IMPASSABLE);

    status= game_engine_move_player(eng, DIR_SOUTH);
    ck_assert(status == OK || status== ROOM_IMPASSABLE);

    status= game_engine_move_player(eng, DIR_EAST);
    ck_assert(status == OK || status== ROOM_IMPASSABLE);

    status= game_engine_move_player(eng, DIR_WEST);
    ck_assert(status == OK || status== ROOM_IMPASSABLE);

    game_engine_destroy(eng);
}
END_TEST

//Test 18: Test reset with bad inputs
START_TEST(test_engine_reset_invalid_inputs)
{
    ck_assert_int_eq(game_engine_reset(NULL), INVALID_ARGUMENT);
}
END_TEST

//Test 19: Test reset basic functionalty
START_TEST(test_engine_reset_basic)
{
    GameEngine *eng = NULL;
    ck_assert_int_eq(game_engine_create(STARTER_INI, &eng), OK);

    (void)game_engine_move_player(eng, DIR_EAST);
    (void)game_engine_move_player(eng, DIR_SOUTH);

    ck_assert_int_eq(game_engine_reset(eng), OK);

    game_engine_destroy(eng);
}
END_TEST













Suite *game_engine_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("GameEngine");

    // Create a test case to group related tests
    TCase *tc = tcase_create("Engine");

    // Add individual test functions to the test case
    tcase_add_test(tc, test_engine_create_destroy_basic);
    tcase_add_test(tc, test_engine_create_invalid_inputs);
    tcase_add_test(tc, test_engine_get_player_basic);
    tcase_add_test(tc, test_engine_get_player_null);
    tcase_add_test(tc, test_engine_get_room_count_basic);
    tcase_add_test(tc, test_engine_get_room_count_invalid_inputs);
    tcase_add_test(tc, test_engine_get_room_dimensions_basic);
    tcase_add_test(tc, test_engine_get_room_dimensions_invalid_inputs);
    tcase_add_test(tc, test_engine_get_room_ids_basic);
    tcase_add_test(tc, test_engine_get_room_ids_invalid_inputs);
    tcase_add_test(tc, test_engine_move_player_invalid_inputs);
    tcase_add_test(tc, test_engine_move_player_major);
    tcase_add_test(tc, test_engine_render_current_room_basic);
    tcase_add_test(tc, test_engine_render_current_room_invalid_input);
    tcase_add_test(tc, test_engine_render_room_basic);
    tcase_add_test(tc, test_engine_render_room_invalid_id);
    tcase_add_test(tc, test_engine_render_room_invalid_inputs);
    tcase_add_test(tc, test_engine_reset_basic);
    tcase_add_test(tc, test_engine_reset_invalid_inputs);


    // Add the test case to the suite
    suite_add_tcase(s, tc);

    // Return the complete suite so main() can run it
    return s;
}