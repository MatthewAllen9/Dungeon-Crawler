#!/usr/bin/env python3
"""Deterministic system integration test runner for Treasure Runner."""

# below are the imports that are in the instructors solution.
# you may need different ones.  Use them if you wish.
import os
import argparse
from treasure_runner.bindings import Direction
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.models.exceptions import GameError, ImpassableError


# put your functions here here

#Log state
def state_string(engine: GameEngine) -> str:
    #Store the room id
    room_id = engine.player.get_room()

    #Store the player pos
    x, y = engine.player.get_position()

    #Store the collected treasure count
    collected = engine.player.get_collected_count()

    #Return the state log string
    return f"room={room_id}|x={x}|y={y}|collected={collected}"


#Generate the first possible direction
def find_entry_direction(engine: GameEngine) -> Direction:
    #Store all directionss
    dirs = [Direction.SOUTH, Direction.WEST, Direction.NORTH, Direction.EAST]

    #Loop through all diretions
    for direction in dirs:
        #Get the room and starting vals
        start_room = engine.player.get_room()
        start_x, start_y = engine.player.get_position()

        #Attempt to move the player in the current dir
        try:
            engine.move_player(direction)

            #CHeck the final room and final position
            end_room = engine.player.get_room()
            end_x, end_y = engine.player.get_position()

            #Check if room has changed or position has changed
            same_room = start_room == end_room
            moved = ((start_x != end_x) or (start_y != end_y))

            #Reset the engine
            engine.reset()

            #Check that move was valid
            if same_room and moved:
                #Successful direction
                return direction

        #Return impassable error
        except ImpassableError:
            pass

        #Reset on error
        except GameError:
            engine.reset()

    #No possible directions
    raise RuntimeError()

#Try move to see if it works 
def try_move(engine: GameEngine, direction: Direction) -> tuple[str, str, str, int]:
    #Store the original string and treasure count
    before = state_string(engine)
    before_count = engine.player.get_collected_count()


    #Try to move the player in the direction
    try:
        engine.move_player(direction)
        #Store new location and count
        after = state_string(engine)
        after_count = engine.player.get_collected_count()

        #Check if no change and return
        if before == after:
            return "NO_PROGRESS", before, after, after_count - before_count

        #Return successful move
        return "OK", before, after, after_count - before_count

    #Except when blocked error and return
    except ImpassableError:
        after = state_string(engine)
        after_count = engine.player.get_collected_count()
        return "BLOCKED", before, after, after_count - before_count

    #Error and return
    except GameError:
        after = state_string(engine)
        after_count = engine.player.get_collected_count()
        return "ERROR", before, after, after_count - before_count

#Run a sweep
def run_sweep(engine: GameEngine, log_file, phase: str, direction: Direction, step: int) -> int:
    #Log the start of sweep
    log_file.write("SWEEP_START|phase=" + phase + "|dir=" + direction.name + "\n")

    #Track states to catch cycle
    completed_states = {state_string(engine)}

    #Track completed moves
    moves = 0

    #Loop until stop condition
    while True:
        #Increment step
        step += 1

        #Try to move in the dir
        result, before, after, changed_collected = try_move(engine, direction)

        #Log the move
        log_file.write("MOVE|step=" + str(step) + "|phase=" + phase + "|dir=" + direction.name + "|result=" + result + "|before=" + before + "|after=" + after + "|delta_collected=" + str(changed_collected) + "\n")

        if result == "OK":
            moves += 1

        #Check if stop condition
        if result == "BLOCKED" or result == "NO_PROGRESS" or result == "ERROR":
            #Log and return the final step
            log_file.write("SWEEP_END|phase=" + phase + "|reason=BLOCKED|moves=" + str(moves) + "\n")
            return step

        #Store state to check if in cycle
        current_state = state_string(engine)

        #Check if in cycle
        if current_state in completed_states:
            #Log cycle and return final step
            log_file.write("SWEEP_END|phase=" + phase + "|reason=CYCLE_DETECTED|moves=" + str(moves) + "\n")
            return step

        #Add to all states for cycle prevention
        completed_states.add(current_state)


def parse_args():
    parser = argparse.ArgumentParser(description="Treasure Runner integration test logger")
    parser.add_argument(
        "--config",
        required=True,
        help="Path to generator config file",
    )
    parser.add_argument(
        "--log",
        required=True,
        help="Output log path",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    config_path = os.path.abspath(args.config)
    log_path = os.path.abspath(args.log)
    #call your main function here

    #Set engine
    engine = GameEngine(config_path)

    #Open the output file
    log_file = open(log_path, "w")

    #Write the start and initial state at step 0 logs
    log_file.write("RUN_START|config=" + config_path + "\n")
    log_file.write("STATE|step=0|phase=SPAWN|state=" + state_string(engine) + "\n")

    #Find the first direction and log it
    entry_dir = find_entry_direction(engine)
    log_file.write("ENTRY|direction=" + entry_dir.name + "\n")

    #Reset the engine
    engine.reset()

    #Try the move and store the 3 strings
    result, before, after, changed_collected = try_move(engine, entry_dir)

    #Write the first move and log it
    log_file.write("MOVE|step=1|phase=ENTRY|dir=" + entry_dir.name + "|result=" + result + "|before=" + before + "|after=" + after + "|delta_collected=" + str(changed_collected) + "\n")

    #Check if fail and terminate
    if result == "ERROR":
        log_file.write("TERMINATED: Initial Move Error\n")
        log_file.write("RUN_END|steps=1|collected_total=" + str(engine.player.get_collected_count()) + "\n")
        log_file.close()
        engine.destroy()
        return 0

    #Set starting step
    step = 1

    #Execute all sweeps in all dirs
    step = run_sweep(engine, log_file, "SWEEP_SOUTH", Direction.SOUTH, step)
    step = run_sweep(engine, log_file, "SWEEP_WEST", Direction.WEST, step)
    step = run_sweep(engine, log_file, "SWEEP_NORTH", Direction.NORTH, step)
    step = run_sweep(engine, log_file, "SWEEP_EAST", Direction.EAST, step)

    #Log final state and end of run
    log_file.write("STATE|step=" + str(step) + "|phase=FINAL|state=" + state_string(engine) + "\n")
    log_file.write("RUN_END|steps=" + str(step) + "|collected_total=" + str(engine.player.get_collected_count()) + "\n")

    #Close the log file
    log_file.close()

    #Free engine
    engine.destroy()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
