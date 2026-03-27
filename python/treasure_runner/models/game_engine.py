import ctypes

from treasure_runner.bindings import lib, Status, GameEngine as CGameEngine, Direction
from treasure_runner.models.exceptions import status_to_exception
from treasure_runner.models.player import Player

class GameEngine:
    #Constructor
    def __init__(self, config_path: str):
        #Set eng to void pointer
        self._eng = CGameEngine()

        #Create bool to store if doubled
        self._destroyed = False

        #Create game engine
        status = lib.game_engine_create(config_path.encode("utf-8"), ctypes.byref(self._eng))

        #Check fail and raise exception
        if status != Status.OK:
            raise status_to_exception(status)

        #Call get player
        player_ptr = lib.game_engine_get_player(self._eng)

        #Check fail and raise runtime error
        if not player_ptr:
            raise RuntimeError()

        #Set instance of player
        self._player = Player(player_ptr)

    @property
    def player(self) -> Player:
        #Return player instance
        return self._player

    def destroy(self) -> None:
        #Check engine isnt already destroyed
        if self._destroyed:
            return

        #Call destroy
        lib.game_engine_destroy(self._eng)
        self._destroyed = True

    def move_player(self, direction: Direction) -> None:
        #Call move player (NEW HELPER)
        status = lib.game_engine_move_player_2(self._eng, direction)

        #Raise if fail
        if status != Status.OK:
            raise status_to_exception(status)

    def use_portal(self) -> None:
        #Call use portal with new helper
        status = lib.game_engine_use_portal(self._eng)

        #Rause if fail
        if status != Status.OK:
            raise status_to_exception(status)

    def render_current_room(self) -> str:
        #Create variable for output
        out = ctypes.c_char_p()

        #Call render
        status = lib.game_engine_render_current_room(self._eng, ctypes.byref(out))

        #Check for error and raise
        if status != Status.OK:
            raise status_to_exception(status, "failed to render current room")

        #Decode the rendered string
        raw = out.value
        rendered = raw.decode("utf-8") if raw is not None else ""

        #Free the string
        lib.game_engine_free_string(out)

        return rendered

    def get_room_count(self) -> int:

        #Create count output and set to default
        out_count = ctypes.c_int(0)

        #Call get room count
        status = lib.game_engine_get_room_count(self._eng, ctypes.byref(out_count))

        #Check fail and raise exception
        if status != Status.OK:
            raise status_to_exception(status)

        #Return count
        return int(out_count.value)

    def get_room_dimensions(self) -> tuple[int, int]:
        #Create width and height output ints
        out_w = ctypes.c_int(0)
        out_h = ctypes.c_int(0)

        #Call get dimensions
        status = lib.game_engine_get_room_dimensions(self._eng, ctypes.byref(out_w), ctypes.byref(out_h))

        #Check fail and raise exception
        if status != Status.OK:
            raise status_to_exception(status)

        #Return the dimensions
        return (int(out_w.value), int(out_h.value))

    def get_room_ids(self) -> list[int]:
        #Create pointer for id list
        ids_ptr = ctypes.POINTER(ctypes.c_int)()
        #Create counter
        count = ctypes.c_int()

        #Call get rooms id
        status = lib.game_engine_get_room_ids(self._eng, ctypes.byref(ids_ptr), ctypes.byref(count))

        #Check for fails and raise
        if status != Status.OK:
            raise status_to_exception(status, "failed to get room ids")

        #Create empty array
        room_ids = []

        #Loop through all ids and add to list
        for i in range(count.value):
            room_ids.append(int(ids_ptr[i]))

        #Free the string in C
        lib.game_engine_free_string(ids_ptr)

        #Return list
        return room_ids

    def reset(self) -> None:
        #Call reset
        status = lib.game_engine_reset(self._eng)

        #Rause if fail
        if status != Status.OK:
            raise status_to_exception(status)
            