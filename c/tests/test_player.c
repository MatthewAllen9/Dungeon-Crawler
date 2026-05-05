#include <check.h>
#include <stdlib.h>
#include "player.h"
#include "types.h"

//Test 1: Creating player with null output
START_TEST(test_player_create_null)
{
    Status status = player_create(1, 2, 3, NULL);
    ck_assert_int_eq(status, INVALID_ARGUMENT);
}
END_TEST

//Test 2: Create player then destroy
START_TEST(test_player_create_destroy)
{
    Player *p = NULL;

    Status status = player_create(1, 2, 3, &p);
    ck_assert_int_eq(status, OK);
    ck_assert_ptr_nonnull(p);

    ck_assert_int_eq(player_get_room(p), 1);

    int x = 0;
    int y = 0;
    ck_assert_int_eq(player_get_position(p, &x, &y), OK);
    ck_assert_int_eq(x, 2);
    ck_assert_int_eq(y, 3);

    player_destroy(p);
    player_destroy(NULL);
}
END_TEST

//Test 3: Get room from null player
START_TEST(test_player_get_room_null)
{
    ck_assert_int_eq(player_get_room(NULL), -1);
}
END_TEST

//Test 4: Get position with null arguements
START_TEST(test_player_get_position_nulls)
{
    Player *p = NULL;
    ck_assert_int_eq(player_create(1, 2, 3, &p), OK);

    int x = 0;
    int y = 0;

    ck_assert_int_eq(player_get_position(NULL, &x, &y), INVALID_ARGUMENT);
    ck_assert_int_eq(player_get_position(p, NULL, &y), INVALID_ARGUMENT);
    ck_assert_int_eq(player_get_position(p, &x, NULL), INVALID_ARGUMENT);

    player_destroy(p);
}
END_TEST

//Test 5: Test position getter and setter
START_TEST(test_player_position_getter_setter)
{
    Player *p = NULL;
    ck_assert_int_eq(player_create(1, 2, 3, &p), OK);

    ck_assert_int_eq(player_set_position(NULL, 1, 1), INVALID_ARGUMENT);
    ck_assert_int_eq(player_set_position(p, 4, 5), OK);

    int x = 0;
    int y = 0;
    ck_assert_int_eq(player_get_position(p, &x, &y), OK);
    ck_assert_int_eq(x, 4);
    ck_assert_int_eq(y, 5);

    player_destroy(p);
}
END_TEST

//Test 6: Move players room and check that position remains the same
START_TEST(test_player_move_to_room)
{
    Player *p = NULL;
    ck_assert_int_eq(player_create(1, 2, 3, &p), OK);

    ck_assert_int_eq(player_move_to_room(NULL, 2), INVALID_ARGUMENT);
    ck_assert_int_eq(player_move_to_room(p, 3), OK);
    ck_assert_int_eq(player_get_room(p), 3);

    int x = 0;
    int y = 0;
    ck_assert_int_eq(player_get_position(p, &x, &y), OK);
    ck_assert_int_eq(x, 2);
    ck_assert_int_eq(y, 3);

    player_destroy(p);
}
END_TEST

//Test 7: Reset the player position and room and check that the new position is to what it was set to
START_TEST(test_player_reset_to_start)
{
    Player *p = NULL;
    ck_assert_int_eq(player_create(1, 2, 3, &p), OK);

    ck_assert_int_eq(player_reset_to_start(NULL, 2, 3, 4), INVALID_ARGUMENT);
    ck_assert_int_eq(player_reset_to_start(p, 2, 3, 4), OK);

    ck_assert_int_eq(player_get_room(p), 2);

    int x = 0;
    int y = 0;
    ck_assert_int_eq(player_get_position(p, &x, &y), OK);
    ck_assert_int_eq(x, 3);
    ck_assert_int_eq(y, 4);

    player_destroy(p);
}
END_TEST

Suite *player_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("Player");

    // Create a test case to group related tests
    TCase *tc = tcase_create("PlayerCase");

    // Add individual test functions to the test case
    tcase_add_test(tc, test_player_create_destroy);
    tcase_add_test(tc, test_player_create_null);
    tcase_add_test(tc, test_player_get_position_nulls);
    tcase_add_test(tc, test_player_get_room_null);
    tcase_add_test(tc, test_player_move_to_room);
    tcase_add_test(tc, test_player_position_getter_setter);
    tcase_add_test(tc, test_player_reset_to_start);

    // Add the test case to the suite
    suite_add_tcase(s, tc);

    // Return the complete suite so main() can run it
    return s;
}