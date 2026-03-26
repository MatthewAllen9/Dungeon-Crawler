#!/usr/bin/env python3
"""Treasure Runner A3 launcher"""

import argparse
from treasure_runner.models.game_engine import GameEngine #Import controller
from treasure_runner.ui.game_ui import GameUI #Import view


def main():
    #Create parser and add path to ini file and profile
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", required=True)
    parser.add_argument("--profile", required=True)
    args = parser.parse_args()

    #Create game engine that runs the ini and ui that uses the player and engine to display the game
    engine = GameEngine(args.config)
    ui = GameUI(engine, args.profile)

    #Call the ui to run
    ui.run()

    #Close game
    engine.destroy()

#Start program with main
if __name__ == "__main__":
    main()