import curses
import json
import os
from datetime import datetime, timezone
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
        self.profile = None
        self.visited_rooms = set()
        self.won = False

    #Start the UI
    def run(self) -> None:
        curses.wrapper(self.main)

    def main(self, stdscr) -> None:
        #Remove cursor
        curses.curs_set(0)

        #Enable arrow keys
        stdscr.keypad(True)

        #Load or create profile
        self.profile = self._load_or_create_profile(stdscr)

        #Show startup screen
        self._show_startup_screen(stdscr)

        #Make sure terminal is always big enough
        if not self._check_terminal_size(stdscr):
            return

        #Track initial room
        self.visited_rooms.add(self.engine.player.get_room())

        #Game loop
        while self.running:
            #Make sure terminal is always big enough
            if not self._check_terminal_size(stdscr):
                return

            #Draw then get input and then handle input then repeat
            self._draw(stdscr)
            key = stdscr.getch()
            self._update(key)

        #Update and save profile after the game ends
        self._update_profile_stats()
        self._save_profile()

        if self.won:
            self._show_victory_screen(stdscr)
        else:
            self._show_quit_screen(stdscr)

    #Load existing profile or create a new one
    def _load_or_create_profile(self, stdscr) -> dict:
        #Check if the profile already exists
        if os.path.exists(self.profile_path):
            with open(self.profile_path, "r", encoding="utf-8") as file:
                return json.load(file)

        #Prompt for player name if profile does not exist
        player_name = self._prompt_player_name(stdscr)

        #Create default profile
        profile = {
            "player_name": player_name,
            "games_played": 0,
            "max_treasure_collected": 0,
            "most_rooms_world_completed": 0,
            "timestamp_last_played": ""
        }

        #Ensure folder exists
        folder = os.path.dirname(self.profile_path)
        if folder:
            os.makedirs(folder, exist_ok=True)

        #Save the new profile immediately
        with open(self.profile_path, "w", encoding="utf-8") as file:
            json.dump(profile, file, indent=4)

        return profile

    #Prompt for player name
    def _prompt_player_name(self, stdscr) -> str:
        #SHow typed input
        curses.echo()

        #Clear screen and prompt
        stdscr.clear()
        stdscr.addstr(0, 0, "No profile found")
        stdscr.addstr(1, 0, "Enter player name: ")
        stdscr.refresh()

        #Read name
        name = stdscr.getstr(1, 19, 50).decode("utf-8").strip()

        #Turn echo back off
        curses.noecho()

        #Set default
        if not name:
            name = "Player"

        return name

    #Show startup splash screen
    def _show_startup_screen(self, stdscr) -> None:
        #Clear screen
        stdscr.clear()

        #Get last played text or set to never
        last_played = self.profile["timestamp_last_played"]
        if not last_played:
            last_played = "Never"

        #Lines to display
        lines = ["Treasure Runner", "", f"Player: {self.profile['player_name']}", f"Games Played: {self.profile['games_played']}", f"Max Treasure Collected: {self.profile['max_treasure_collected']}", f"Most Rooms World Completed: {self.profile['most_rooms_world_completed']}", f"Last Played: {last_played}", "", "Press any key to continue"]

        max_y, max_x = stdscr.getmaxyx()

        row = 0
        for line in lines:
            if row >= max_y:
                break
            stdscr.addstr(row, 0, line[: max_x - 1])
            row += 1

        #Refresh and wait
        stdscr.refresh()
        stdscr.getch()

    #Show quit splash screen
    def _show_quit_screen(self, stdscr) -> None:
        #Clear screen
        stdscr.clear()

        #Lines to display
        lines = ["Thanks for playing Treasure Runner", "", f"Player: {self.profile['player_name']}", f"Games Played: {self.profile['games_played']}", f"Max Treasure Collected: {self.profile['max_treasure_collected']}", f"Most Rooms World Completed: {self.profile['most_rooms_world_completed']}", f"Last Played: {self.profile['timestamp_last_played']}", "", "Press any key to exit"]

        max_y, max_x = stdscr.getmaxyx()

        row = 0
        for line in lines:
            if row >= max_y:
                break
            stdscr.addstr(row, 0, line[: max_x - 1])
            row += 1

        #Refresh and wait
        stdscr.refresh()
        stdscr.getch()

    #Show victory screen
    def _show_victory_screen(self, stdscr) -> None:
        #Clear screen
        stdscr.clear()

        #Get terminal size
        max_y, max_x = stdscr.getmaxyx()

        #Get final stats
        collected = self.engine.player.get_collected_count()
        total = self.engine.get_total_treasure_count()
        rooms_visited = len(self.visited_rooms)

        #Lines to display
        lines = ["Victory!", "", f"Player: {self.profile['player_name']}", f"Treasures Collected: {collected}/{total}", f"Rooms Visited: {rooms_visited}", f"Games Played: {self.profile['games_played']}", f"Max Treasure Collected: {self.profile['max_treasure_collected']}", f"Most Rooms World Completed: {self.profile['most_rooms_world_completed']}", "", "You collected every treasure in the world!", "", "Press any key to continue"]

        #Print all lines
        row = 0
        for line in lines:
            if row >= max_y:
                break
            stdscr.addstr(row, 0, line[: max_x - 1])
            row += 1

        #Refresh and wait
        stdscr.refresh()
        stdscr.getch()

    def _get_min_terminal_size(self):
        room_width, room_height = self._get_room_render_size()
        legend_width = self._get_legend_width()

        min_width = self._get_min_terminal_width(room_width, legend_width)
        min_height = self._get_min_terminal_height(room_height)

        return min_height, min_width

    def _get_room_render_size(self):
        room_text = self.engine.render_current_room()
        room_lines = room_text.splitlines()

        room_width = 0
        for line in room_lines:
            if len(line) > room_width:
                room_width = len(line)

        room_height = len(room_lines)
        return room_width, room_height

    def _get_legend_width(self):
        legend_items = self._build_legend_items()

        legend_width = 0
        for text in legend_items:
            if len(text) > legend_width:
                legend_width = len(text)

        return legend_width

    def _get_min_terminal_width(self, room_width, legend_width):
        progress_text = "Treasure Progress: " + self._build_progress_bar()
        controls_text = "Game Controls: Arrows / WASD Move, > Portal, r Reset, q Quit"
        status_text = self._build_status_text()
        footer_text = "-=+ Treasure Runner +=-    mallen31@uoguelph.ca"

        side_by_side_width = room_width + 2 + legend_width

        return max(
            side_by_side_width,
            len(progress_text),
            len(controls_text),
            len(status_text),
            len(footer_text)
        )

    def _get_min_terminal_height(self, room_height):
        game_row = 3
        progress_row = game_row + room_height + 1
        controls_row = progress_row + 2
        status_row = controls_row + 2
        footer_row = status_row + 1

        return footer_row + 1

    def _check_terminal_size(self, stdscr):
        max_y, max_x = stdscr.getmaxyx()
        min_height, min_width = self._get_min_terminal_size()

        if max_y < min_height or max_x < min_width:
            stdscr.clear()
            stdscr.addstr(
                0,
                0,
                f"Terminal too small. Need at least {min_width}x{min_height}"[: max_x - 1]
            )
            stdscr.refresh()
            stdscr.getch()
            return False

        return True

    #Draw the current render
    def _draw(self, stdscr) -> None:
        #Clear the screan
        stdscr.clear()

        #Get the terminal size
        max_y, max_x = stdscr.getmaxyx()

        #Get the text for the room and seperate it by each line
        room_text = self.engine.render_current_room()
        room_lines = room_text.splitlines()

        #Get room id
        room_id = self.engine.player.get_room()

        #Add current room to visited rooms
        self.visited_rooms.add(room_id)

        #Display the message and room
        stdscr.addstr(0, 0, self.message[: max_x - 1])
        stdscr.addstr(1, 0, f"Room {room_id}"[: max_x - 1])

        #Call draw room
        room_width, room_height = self._draw_room(stdscr, room_lines, max_y, max_x)

        #Call draw legend and controls and progress
        self._draw_legend(stdscr, room_width, max_y, max_x)
        self._draw_controls_status(stdscr, room_height + 2, max_y, max_x)
        self._draw_progress_bar(stdscr, 3 + room_height + 1, max_y, max_x)

        #Refresh
        stdscr.refresh()

    #Update based on input
    def _update(self, key) -> None:
        #Map movement keys to directions
        move_keys = {
            curses.KEY_UP: Direction.NORTH,
            ord("w"): Direction.NORTH,
            ord("W"): Direction.NORTH,
            curses.KEY_DOWN: Direction.SOUTH,
            ord("s"): Direction.SOUTH,
            ord("S"): Direction.SOUTH,
            curses.KEY_RIGHT: Direction.EAST,
            ord("d"): Direction.EAST,
            ord("D"): Direction.EAST,
            curses.KEY_LEFT: Direction.WEST,
            ord("a"): Direction.WEST,
            ord("A"): Direction.WEST,
        }

        #Exit if q
        if key in (ord("q"), ord("Q")):
            self.message = "Bye bye!"
            self.running = False
            return

        #Reset game on r
        if key in (ord("r"), ord("R")):
            self.engine.reset()
            self.visited_rooms = {self.engine.player.get_room()}
            self.message = "Game reset."
            return

        #Portal key
        if key == ord(">"):
            self._handle_portal()
            return

        #Movement keys
        if key in move_keys:
            self._try_move(move_keys[key])

    #Try move
    def _try_move(self, direction) -> None:
        #Store room and treasure count before moving
        old_room = self.engine.player.get_room()
        old_count = self.engine.player.get_collected_count()

        #Move if valid
        try:
            self.engine.move_player(direction)

            #Store updated room and treasure count
            new_room = self.engine.player.get_room()
            new_count = self.engine.player.get_collected_count()

            #Update message based on result
            if new_room != old_room:
                self.message = f"Entered Room {new_room}"
            elif new_count > old_count:
                total_count = self.engine.get_total_treasure_count()
                self.message = f"Treasure collected! {new_count}/{total_count}"

                #Check for victory
                if new_count == total_count and total_count > 0:
                    self.running = False
                    self.won = True
            else:
                self.message = f"Moved {direction.name}"

        #Print fail
        except Exception as exc:
            self.message = str(exc) if str(exc) else "Can't go there"

    def _draw_room(self, stdscr, room_lines, max_y, max_x):

        #Store first row for map and current row
        game_row = 3
        current_row = game_row

        #Find the widest line
        room_width = 0
        for line in room_lines:
            if len(line) > room_width:
                room_width = len(line)

        #Leave if room is offscreen
        for line in room_lines:
            if current_row >= max_y:
                break

            #Draw the line
            stdscr.addstr(current_row, 0, line[: max_x - 1])
            current_row += 1

        #Return
        return room_width, len(room_lines)

    def _draw_legend(self, stdscr, room_width, max_y, max_x):

        #Store legend column based on room width and set game row
        legend_col = room_width + 2
        game_row = 3

        legend_items = self._build_legend_items()

        row = game_row
        for text in legend_items:
            if row < max_y and legend_col < max_x:
                stdscr.addstr(row, legend_col, text[: max_x - legend_col - 1])
            row += 1

    def _draw_progress_bar(self, stdscr, row, max_y, max_x):
        if row < max_y:
            progress_text = "Treasure Progress: " + self._build_progress_bar()
            stdscr.addstr(row, 0, progress_text[: max_x - 1])

    def _draw_controls(self, stdscr, controls_row, max_y, max_x):
        #Displat controls
        if controls_row < max_y:
            stdscr.addstr(controls_row, 0, "Game Controls: Arrows / WASD Move, > Portal, r Reset, q Quit"[: max_x - 1])

    def _draw_status_footer(self, stdscr, status_row, footer_row, max_y, max_x):
        #Display status
        if status_row < max_y:
            stdscr.addstr(status_row, 0, self._build_status_text()[: max_x - 1])

        #Display footer
        if footer_row < max_y:
            stdscr.addstr(footer_row, 0, "-=+ Treasure Runner +=-    mallen31@uoguelph.ca"[: max_x - 1])

    def _draw_controls_status(self, stdscr, room_height, max_y, max_x):
        game_row = 3
        controls_row = game_row + room_height + 1
        status_row = controls_row + 2
        footer_row = status_row + 1

        self._draw_controls(stdscr, controls_row, max_y, max_x)
        self._draw_status_footer(stdscr, status_row, footer_row, max_y, max_x)

    def _build_legend_items(self):
        charset = self.engine.get_charset()

        return [
            "Game Elements:",
            "",
            f"{charset['player']} - Player",
            f"{charset['wall']} - Wall",
            f"{charset['floor']} - Floor",
            f"{charset['treasure']} - Treasure",
            f"{charset['portal']} - Portal",
            f"{charset['pushable']} - Pushable",
        ]

    def _build_progress_bar(self):
        collected = self.engine.player.get_collected_count()
        total = self.engine.get_total_treasure_count()

        if total <= 0:
            return "[--------------------------------------------------] 0%"

        filled = int((collected / total) * 50)
        empty = 50 - filled
        percent = int((collected / total) * 100)

        return "[" + ("=" * filled) + ("-" * empty) + f"] {percent}%"

    def _build_status_text(self):
        player_x, player_y = self.engine.player.get_position()
        collected = self.engine.player.get_collected_count()

        room_count = self.engine.get_room_count()

        return f"Player Status: {self.profile['player_name']} | " f"Treasures Collected: {collected} | " f"Co-ords: ({player_x},{player_y}) | " f"Rooms Visited: {len(self.visited_rooms)}/{room_count}"

    def _handle_portal(self):
        old_room = self.engine.player.get_room()
        try:
            self.engine.use_portal()
            new_room = self.engine.player.get_room()

            if new_room != old_room:
                self.message = f"Entered Room {new_room}"
            else:
                self.message = "Used portal."

        except Exception as exc:
            self.message = str(exc) if str(exc) else "No portal here."

    #Save profile to file
    def _save_profile(self) -> None:
        #Ensure folder exists
        folder = os.path.dirname(self.profile_path)
        if folder:
            os.makedirs(folder, exist_ok=True)

        #Write profile to file
        with open(self.profile_path, "w", encoding="utf-8") as file:
            json.dump(self.profile, file, indent=4)

    #Update profile stats after game ends
    def _update_profile_stats(self) -> None:
        #Get current collected treasure count
        collected = self.engine.player.get_collected_count()

        #Get number of rooms visited
        rooms_visited = len(self.visited_rooms)

        #Increment games played
        self.profile["games_played"] += 1

        #Update max treasure if needed
        if collected > self.profile["max_treasure_collected"]:
            self.profile["max_treasure_collected"] = collected

        #Update most rooms completed if needed
        if rooms_visited > self.profile["most_rooms_world_completed"]:
            self.profile["most_rooms_world_completed"] = rooms_visited

        #Update timestamp
        self.profile["timestamp_last_played"] = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")
