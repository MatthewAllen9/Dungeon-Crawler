# #import unittest
# # fake test for example

# class TestTimeTravelingToaster(unittest.TestCase):

#     def setUp(self):
#         self.toaster = TimeTravelingToaster()

#     def test_default_darkness(self):
#         result = self.toaster.toast("sourdough")
#         self.assertEqual(result, "sourdough toasted to level 3")

#     def test_set_darkness_valid(self):
#         self.toaster.set_darkness(7)
#         result = self.toaster.toast("rye")
#         self.assertEqual(result, "rye toasted to level 7")

#     def test_set_darkness_invalid(self):
#         with self.assertRaises(ValueError):
#             self.toaster.set_darkness(42)

#     def test_rewind_removes_last_toast(self):
#         self.toaster.toast("white")
#         self.assertEqual(self.toaster.history_count(), 1)
#         last = self.toaster.rewind_last_toast()
#         self.assertEqual(last, "white toasted to level 3")
#         self.assertEqual(self.toaster.history_count(), 0)

#     def test_rewind_without_history_raises(self):
#         with self.assertRaises(RuntimeError):
#             self.toaster.rewind_last_toast()


# if __name__ == "__main__":
#     unittest.main()

import os
import unittest

from treasure_runner.bindings import Direction
from treasure_runner.models.exceptions import GameError
from treasure_runner.models.game_engine import GameEngine

TEST_PATH = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "assets", "treasure_runner.ini")
)

class TestGameEngine(unittest.TestCase):
    def setUp(self):
        self.engine = GameEngine(TEST_PATH)

    def tearDown(self):
        self.engine.destroy()

    def test_room_count_positive(self):
        self.assertGreater(self.engine.get_room_count(), 0)

    def test_room_dimensions_positive(self):
        width, height = self.engine.get_room_dimensions()
        self.assertGreater(width, 0)
        self.assertGreater(height, 0)

    def test_render_returns_string(self):
        rendered = self.engine.render_current_room()
        self.assertIsInstance(rendered, str)
        self.assertGreater(len(rendered), 0)
        self.assertIn("\n", rendered)

    def test_player_position_in_bounds(self):
        x, y = self.engine.player.get_position()
        width, height = self.engine.get_room_dimensions()
        self.assertGreaterEqual(x, 0)
        self.assertGreaterEqual(y, 0)
        self.assertLess(x, width)
        self.assertLess(y, height)

    def test_player_starts_with_zero_treasures(self):
        self.assertEqual(self.engine.player.get_collected_count(), 0)
        self.assertEqual(self.engine.player.get_collected_treasures(), [])

    def test_has_collected_treasure_returns_bool(self):
        self.assertIsInstance(
            self.engine.player.has_collected_treasure(999999),
            bool,
        )

    def test_get_room_ids_returns_list(self):
        room_ids = self.engine.get_room_ids()
        self.assertIsInstance(room_ids, list)
        self.assertGreater(len(room_ids), 0)
        self.assertTrue(all(isinstance(room_id, int) for room_id in room_ids))

    def test_reset_returns_to_start(self):
        start_room = self.engine.player.get_room()
        start_pos = self.engine.player.get_position()

        for direction in (
            Direction.NORTH,
            Direction.SOUTH,
            Direction.EAST,
            Direction.WEST,
        ):
            try:
                self.engine.move_player(direction)
            except GameError:
                pass

        self.engine.reset()
        self.assertEqual(self.engine.player.get_room(), start_room)
        self.assertEqual(self.engine.player.get_position(), start_pos)

    def test_move_attempts_do_not_crash(self):
        for direction in (
            Direction.NORTH,
            Direction.SOUTH,
            Direction.EAST,
            Direction.WEST,
        ):
            try:
                self.engine.move_player(direction)
            except GameError:
                pass


class TestPlayer(unittest.TestCase):
    def setUp(self):
        self.engine = GameEngine(TEST_PATH)
        self.player = self.engine.player

    def tearDown(self):
        self.engine.destroy()

    def test_get_room_returns_int(self):
        self.assertIsInstance(self.player.get_room(), int)

    def test_get_position_returns_tuple(self):
        position = self.player.get_position()
        self.assertIsInstance(position, tuple)
        self.assertEqual(len(position), 2)
        self.assertIsInstance(position[0], int)
        self.assertIsInstance(position[1], int)

    def test_get_collected_treasures_returns_list(self):
        treasures = self.player.get_collected_treasures()
        self.assertIsInstance(treasures, list)

    def test_collected_count_matches_list_length(self):
        count = self.player.get_collected_count()
        treasures = self.player.get_collected_treasures()
        self.assertEqual(count, len(treasures))


if __name__ == "__main__":
    unittest.main()
