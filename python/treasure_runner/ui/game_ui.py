import curses
from treasure_runner.bindings import Direction

#View
class GameUI:
    #Constructor
    def __init__(self, engine, profile_path: str):
        #Create attributes
        self.engine = engine
        self.profile_path = profile_path
        self.message = "-=+ Treasure Runner +=-"
        self.running = True

    #Start the UI
    def run(self) -> None:
        curses.wrapper(self.main)

    def main(self, stdscr) -> None:
        #Remove cursor
        curses.curs_set(0)

        #Game loop
        while self.running:
            #Draw then get input and then handle input then repeat
            self.draw(stdscr)
            key = stdscr.getch()
            self.update(key)

    #Draw the current render
    def draw(self, stdscr) -> None:
        #Clear the screan
        stdscr.clear()

        #Get the terminal size
        max_y, max_x = stdscr.getmaxyx()

        #Get the text for the room and seperate it by each line
        room_text = self.engine.render_current_room()
        room_lines = room_text.splitlines()

        #Print the message on line 1
        if max_y > 0:
            stdscr.addstr(0, 0, self.message[: max_x - 1])

        #Print room num on line 2
        room_id = self.engine.player.get_room()
        if max_y > 1:
            stdscr.addstr(1, 0, f"Room: {room_id}"[: max_x - 1])

        #Draw room render
        game_row = 3

        #Draw each row
        for line in room_lines:
            #Exit if not on terminal
            if game_row >= max_y:
                break

            #Draw row
            stdscr.addstr(game_row, 0, line[: max_x - 1])
            game_row += 1

        #Store values to display and locations
        collected = self.engine.player.get_collected_count()
        status_row = game_row + len(room_lines) + 1
        controls_row = game_row + len(room_lines) + 2

        #Display treasures collected
        if status_row < max_y:
            stdscr.addstr(status_row, 0, f"Treasures collected: {collected}"[: max_x - 1])

        #Display controls
        if controls_row < max_y:
            stdscr.addstr(
                controls_row,
                0,
                "Controls: Arrow keys / WASD move, q quit, r reset"[: max_x - 1],
            )

        stdscr.refresh()

    #Update based on input
    def update(self, key) -> None:
        #Exit if q
        if key in (ord("q"), ord("Q")):
            self.message = "Quitting game..."
            self.running = False
        #Try to move up if up key or w same for rest of controls
        elif key in (curses.KEY_UP, ord("w"), ord("W")):
            self._try_move(Direction.NORTH)
        elif key in (curses.KEY_DOWN, ord("s"), ord("S")):
            self._try_move(Direction.SOUTH)
        elif key in (curses.KEY_RIGHT, ord("d"), ord("D")):
            self._try_move(Direction.EAST)
        elif key in (curses.KEY_LEFT, ord("a"), ord("A")):
            self._try_move(Direction.WEST)
        #Reset game on r
        elif key in (ord("r"), ord("R")):
            self.engine.reset()
            self.message = "Game reset."

    #Try move
    def _try_move(self, direction) -> None:
        #Move if valid
        try:
            self.engine.move_player(direction)
            self.message = "Moved."
        #Print fail
        except Exception as exc:
            self.message = str(exc) if str(exc) else "Failed move."