import ctypes

from treasure_runner.bindings import lib

class Player:
    def __init__(self, ptr):
        #Initialize the player pointer
        self._ptr = ptr

    def get_room(self) -> int:
        #Call  get room
        return lib.player_get_room(self._ptr)

    def get_position(self) -> tuple[int, int]:
        #Create variables to return
        x = ctypes.c_int()
        y = ctypes.c_int()

        #Call get position and return
        lib.player_get_position(self._ptr, ctypes.byref(x), ctypes.byref(y))
        return x.value, y.value

    def get_collected_count(self) -> int:
        #Return get collected count
        return lib.player_get_collected_count(self._ptr)

    def has_collected_treasure(self, treasure_id: int) -> bool:
        #Return if the player has collected the specified treasure
        return lib.player_has_collected_treasure(self._ptr, treasure_id)

    def get_collected_treasures(self) -> list[dict]:
        #Create varuable to store count
        count = ctypes.c_int()

        #Create array to later return
        collected = []

        #Get all collected treasures
        treasures = lib.player_get_collected_treasures(self._ptr, ctypes.byref(count))

        #Loop through all collected treasures
        for i in range(count.value):
            #Set the treasure pointer and store each individuals treasures content
            treasure_ptr = treasures[i]
            treasure = treasure_ptr.contents

            #Add the content to the array with their labels
            collected.append({"id": treasure.id, "name": treasure.name.decode("utf-8") if treasure.name else "", "starting_room_id": treasure.starting_room_id, "initial_x": treasure.initial_x, "initial_y": treasure.initial_y, "x": treasure.x, "y": treasure.y, "collected": treasure.collected})

        #Return the array
        return collected
        