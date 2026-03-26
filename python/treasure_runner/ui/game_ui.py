import curses
from treasure_runner.bindings import Direction

#View
class GameUI:
    #Constructor
    def __init__(self, engine, profile_path: str):
        #Create attributes
        self.engine = engine
        self.profile_path = profile_path
        self.message = "Welcome"
        self.running = True

    #Start the UI
    def run(self) -> None:
        curses.wrapper(self.main)

    def main(self, stdscr) -> None:
        #Remove cursor
        curses.curs_set(0)

        #Enable arrow keys
        stdscr.keypad(True)

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

        #Get room id
        room_id = self.engine.player.get_room()

        #Store room name
        room_name = f"Room {room_id}"

        #Get treasures collected
        collected = self.engine.player.get_collected_count()

        #Get player position
        player_x, player_y = self.engine.player.get_position()

        #Get total room count
        room_count = self.engine.get_room_count()

        #Store screen positions
        message_row = 0
        room_label_row = 1
        game_row = 3
       
        #Find the widest row in the room
        room_width = 0
        for line in room_lines:
            if len(line) > room_width:
                room_width = len(line)

        #Store legend column based on room width
        legend_col = room_width + 2

        #Store room height
        room_height = len(room_lines)

        #Check if terminal too small
        if max_y < 24 or max_x < 76:
            stdscr.addstr(0, 0, "Terminal too small."[: max_x - 1])
            stdscr.refresh()
            return

        #Print the message bar
        if message_row < max_y:
            stdscr.addstr(message_row, 0, self.message[: max_x - 1])

        #Print room number and name
        if room_label_row < max_y:
            stdscr.addstr(room_label_row, 0, f"Room {room_id}: {room_name}"[: max_x - 1])

        #Draw room render
        current_row = game_row

        #Draw each row
        for line in room_lines:
            #Exit if not on terminal
            if current_row >= max_y:
                break

            #Draw cur row
            stdscr.addstr(current_row, 0, line[: max_x - 1])
            current_row += 1

        #Print legend title
        if game_row < max_y and legend_col < max_x:
            stdscr.addstr(game_row, legend_col, "Game Elements:"[: max_x - legend_col - 1])

        #Print legend items
        if game_row + 2 < max_y and legend_col < max_x:
            stdscr.addstr(game_row + 2, legend_col, "@ - Player"[: max_x - legend_col - 1])

        if game_row + 3 < max_y and legend_col < max_x:
            stdscr.addstr(game_row + 3, legend_col, "# - Wall"[: max_x - legend_col - 1])

        if game_row + 4 < max_y and legend_col < max_x:
            stdscr.addstr(game_row + 4, legend_col, ". - Floor"[: max_x - legend_col - 1])

        if game_row + 5 < max_y and legend_col < max_x:
            stdscr.addstr(game_row + 5, legend_col, "$ - Treasure"[: max_x - legend_col - 1])

        if game_row + 6 < max_y and legend_col < max_x:
            stdscr.addstr(game_row + 6, legend_col, "X - Portal"[: max_x - legend_col - 1])

        if game_row + 7 < max_y and legend_col < max_x:
            stdscr.addstr(game_row + 7, legend_col, "O - Pushable"[: max_x - legend_col - 1])

        #Store values to display and locations
        controls_row = game_row + room_height + 1
        status_row = controls_row + 2
        footer_row = status_row + 1

        #Display controls
        if controls_row < max_y:
            stdscr.addstr(controls_row, 0, "Game Controls: Arrows / WASD Move, > Portal, r Reset, q Quit"[: max_x - 1])

        #Display player status
        if status_row < max_y:
            stdscr.addstr(status_row, 0, f"Player Status: Treasures Collected: {collected} | Co-ords: ({player_x},{player_y}) | Total Rooms: {room_count}"[: max_x - 1])

        #Display footer
        if footer_row < max_y:
            stdscr.addstr(footer_row, 0, f"-=+ Treasure Runner +=-    mallen31@uoguelph.ca"[: max_x - 1],)

        #Refresh display
        stdscr.refresh()

    #Update based on input
    def update(self, key) -> None:
        #Exit if q
        if key in (ord("q"), ord("Q")):
            self.message = "Bye bye!"
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
        #Portal key
        elif key == ord(">"):
            self.message = "Portal key pressed."

    #Try move
    def _try_move(self, direction) -> None:
        #Move if valid
        try:
            self.engine.move_player(direction)
            self.message = "Moved."
        #Print fail
        except Exception as exc:
            self.message = str(exc) if str(exc) else "Failed move."