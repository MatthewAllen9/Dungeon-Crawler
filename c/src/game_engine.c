#include "game_engine.h"
#include "graph.h"
#include "player.h"
#include "room.h"
#include "world_loader.h"

#include <stdlib.h>
#include <string.h>

//Match room with id
static Room *game_engine_find_room_by_id(const GameEngine *eng, int room_id){
    const void *const *rooms = NULL;
    int room_count = 0;

    if (!eng){
        return NULL;
    }

    if (graph_get_all_payloads(eng->graph, &rooms, &room_count) != GRAPH_STATUS_OK){
        return NULL;
    }

    for (int i = 0; i < room_count; i++){
        Room *r = (Room *)rooms[i];
        if (r && r->id == room_id){
            return r;
        }
    }

    return NULL;
}

//Change in direction
static Status game_engine_direction_delta(Direction dir, int *dx_out, int *dy_out){
    if (!dx_out || !dy_out){
        return INVALID_ARGUMENT;
    }

    *dx_out = 0;
    *dy_out = 0;

    switch (dir){
        case DIR_NORTH:
            *dy_out = -1;
            return OK;
        case DIR_SOUTH:
            *dy_out = 1;
            return OK;
        case DIR_EAST:
            *dx_out = 1;
            return OK;
        case DIR_WEST:
            *dx_out = -1;
            return OK;
        default:
            return INVALID_ARGUMENT;
    }
}

//Match treasure to id
static Treasure *game_engine_find_treasure_by_id(Room *room, int treasure_id){
    if (!room){
        return NULL;
    }

    for (int i = 0; i < room->treasure_count; i++){
        if (room->treasures[i].id == treasure_id){
            return &room->treasures[i];
        }
    }

    return NULL;
}

//Set player to move
static Status game_engine_handle_floor_move(Player *player, int new_x, int new_y){
    return player_set_position(player, new_x, new_y);
}

//Collect treasure
static Status game_engine_handle_treasure_move(GameEngine *eng, Room *room, int treasure_id){
    Treasure *treasure = game_engine_find_treasure_by_id(room, treasure_id);
    if (!treasure){
        return INTERNAL_ERROR;
    }

    return player_try_collect(eng->player, treasure);
}

//Push and move player
static Status game_engine_handle_pushable_move(GameEngine *eng, Room *room, int pushable_idx, Direction dir, int new_x, int new_y){
    Status status = room_try_push(room, pushable_idx, dir);
    if (status != OK){
        return ROOM_IMPASSABLE;
    }

    return player_set_position(eng->player, new_x, new_y);
}

//Move through portals
static Status game_engine_handle_portal_move(GameEngine *eng, int dest_room_id){
    Room *dest_room = game_engine_find_room_by_id(eng, dest_room_id);
    if (!dest_room){
        return GE_NO_SUCH_ROOM;
    }

    int start_x = 0;
    int start_y = 0;
    Status status = room_get_start_position(dest_room, &start_x, &start_y);
    if (status != OK){
        return status;
    }

    status = player_move_to_room(eng->player, dest_room_id);
    if (status != OK){
        return status;
    }

    return player_set_position(eng->player, start_x, start_y);
}


Status game_engine_create(const char *config_file_path, GameEngine **engine_out){
    //Return invalid if null ouput or path
    if (!config_file_path || !engine_out){
        return INVALID_ARGUMENT;
    }

    //Set output to default
    *engine_out = NULL;

    //Create engine variable and allocate space
    GameEngine *eng = (GameEngine *)malloc(sizeof(GameEngine));
    //Retrun memory error if fail
    if (!eng){
        return NO_MEMORY;
    }

    //Set engine vals to default
    eng->graph = NULL;
    eng->player = NULL;
    eng->room_count = 0;
    eng->initial_room_id = -1;
    eng->initial_player_x = 0;
    eng->initial_player_y = 0;
    Room *first_room = NULL;
    int num_rooms = 0;

    //Create variable to store chars abd fill with 
    Charset chars;
    memset(&chars, 0, sizeof(chars));

    //Create status variable and call loader
    Status status = loader_load_world(config_file_path, &eng->graph, &first_room, &num_rooms, &chars);
    //Free and  return fail if loader fails
    if (status != OK){
        free(eng);
        return status;
    }

    //Set engines charset and room nums to loaded vals
    eng->charset = chars;
    eng->room_count = num_rooms;

    //Check for the case with no rooms and return error
    if (!first_room){
        graph_destroy(eng->graph);
        free(eng);
        return INTERNAL_ERROR;
    }

    //Create variables for player starting location
    int start_x = 0;
    int start_y = 0;

    //Get starting position for initial room
    status = room_get_start_position(first_room, &start_x, &start_y);

    //Return and free if fail
    if (status != OK){
        graph_destroy(eng->graph);
        free(eng);
        return status;
    }

    //Create player using starting vals
    status = player_create(first_room->id, start_x, start_y, &eng->player);

    //Return and free if fail
    if (status != OK){
        graph_destroy(eng->graph);
        free(eng);
        return status;
    }

    //Store initial room and starting values into the engine
    eng->initial_room_id = first_room->id;
    eng->initial_player_x = start_x;
    eng->initial_player_y = start_y;

    //Output the engine to start and return
    *engine_out = eng;
    return OK;
}

void game_engine_destroy(GameEngine *eng){
    //Return in engine doesnt exist
    if (!eng){
        return;
    }

    //Destroy all engines data and free
    player_destroy(eng->player);
    graph_destroy(eng->graph);
    free(eng);
}

const Player *game_engine_get_player(const GameEngine *eng){
    //Return null if no engine exists
    if (!eng){
        return NULL;
    }

    //Return player
    return eng->player;
}

Status game_engine_move_player(GameEngine *eng, Direction dir){
    //Return invalid if engine or player is null
    if (!eng || !eng->player){
        return INVALID_ARGUMENT;
    }

    //Set positions to default
    int pos_x = 0;
    int pos_y = 0;
    if (player_get_position(eng->player, &pos_x, &pos_y) != OK){
        return INTERNAL_ERROR;
    }

    //Set new pos to current pos
    int dx = 0;
    int dy = 0;

    Status status = game_engine_direction_delta(dir, &dx, &dy);
    if (status != OK){
        return status;
    }

    //Set new positions
    int new_x = pos_x + dx;
    int new_y = pos_y + dy;

    int current_room_id = player_get_room(eng->player);
    Room *current_room = game_engine_find_room_by_id(eng, current_room_id);
    if (!current_room){
        return GE_NO_SUCH_ROOM;
    }

    //Create variable to store output id
    int out_id = -1;

    //Find tile type
    RoomTileType tile_type = room_classify_tile(current_room, new_x, new_y, &out_id);

    //Check if new position is passable and return if not
    if (tile_type == ROOM_TILE_INVALID || tile_type == ROOM_TILE_WALL){
        return ROOM_IMPASSABLE;
    }

    //Call each function for tile type
    switch (tile_type){
        case ROOM_TILE_FLOOR:
            return game_engine_handle_floor_move(eng->player, new_x, new_y);
        case ROOM_TILE_TREASURE:
            return game_engine_handle_treasure_move(eng, current_room, out_id);
        case ROOM_TILE_PUSHABLE:
            return game_engine_handle_pushable_move(eng, current_room, out_id, dir, new_x, new_y);
        case ROOM_TILE_PORTAL:
            return game_engine_handle_portal_move(eng, out_id);
        default:
            return INTERNAL_ERROR;
    }
}

Status game_engine_get_room_count(const GameEngine *eng, int *count_out){
    //Return if engine doesnt exist or output is null
    if (!eng){
        return INVALID_ARGUMENT;
    }
    if (!count_out){
        return NULL_POINTER;
    }

    //Set room count and return
    *count_out = eng->room_count;
    return OK;
}

Status game_engine_get_room_dimensions(const GameEngine *eng, int *width_out, int *height_out){
    //Return if engine doesnt exist or outputs are null
    if (!eng){
        return INVALID_ARGUMENT;
    }
    if (!width_out || !height_out){
        return NULL_POINTER;
    }
    if (!eng->player){
        return INTERNAL_ERROR;
    }

    //Set room id to current room id
    int room_id = player_get_room(eng->player);

    //Create variables for rooms and room count and set to default
    const void *const *rooms = NULL;
    int room_count = 0;

    //Store rooms and room count and return if fail
    if (graph_get_all_payloads(eng->graph, &rooms, &room_count) != GRAPH_STATUS_OK){
        return INTERNAL_ERROR;
    }

    //Create variable to store room
    Room *room = NULL;

    //Loop through all rooms
    for (int i = 0; i < room_count; i++){
        //Set to current room
        Room *r = (Room *)rooms[i];

        //Check if room id matches
        if (r && r->id == room_id){
            room = r;
            break;
        }
    }

    //Return if no id matches
    if (!room){
        return GE_NO_SUCH_ROOM;
    }

    //Output dimensions and return
    *width_out = room_get_width(room);
    *height_out = room_get_height(room);
    return OK;
}

Status game_engine_reset(GameEngine *eng){
    //Return if engine or player dont exist
    if (!eng || !eng->player){
        return INVALID_ARGUMENT;
    }

    //Reset player first
    Status status = player_reset_to_start(eng->player, eng->initial_room_id, eng->initial_player_x, eng->initial_player_y);

    //Return if fail
    if (status != OK){
        return status;
    }

    //Get all rooms and set counter
    const void *const *rooms = NULL;
    int room_count = 0;

    //Get rooms
    if (graph_get_all_payloads(eng->graph, &rooms, &room_count) != GRAPH_STATUS_OK){
        return INTERNAL_ERROR;
    }

    //Loop through all rooms
    for (int i = 0; i < room_count; i++){
        Room *room = (Room *)rooms[i];

        //Skip if not a room
        if (!room){
            continue;
        }

        //Loop through all rooms treasures
        for (int j = 0; j < room->treasure_count; j++){
            //Reset treasure
            room->treasures[j].collected = false;
            room->treasures[j].x = room->treasures[j].initial_x;
            room->treasures[j].y = room->treasures[j].initial_y;
        }

        //Same for pushables
        for (int j = 0; j < room->pushable_count; j++){
            room->pushables[j].x = room->pushables[j].initial_x;
            room->pushables[j].y = room->pushables[j].initial_y;
        }
    }

    return OK;
}

Status game_engine_render_current_room(const GameEngine *eng, char **str_out){
    //Return if engine doesnt exist or output is null
    if (!eng){
        return INVALID_ARGUMENT;
    }
    if (!str_out){
        return NULL_POINTER;
    }
    if (!eng->player){
        return INTERNAL_ERROR;
    }

    //Set output to default
    *str_out = NULL;

    //Store room id
    int room_id = player_get_room(eng->player);

    //Same as b4 (find room match by id)
    const void *const *rooms = NULL;
    int room_count = 0;

    if (graph_get_all_payloads(eng->graph, &rooms, &room_count) != GRAPH_STATUS_OK){
        return INTERNAL_ERROR;
    }

    Room *room = NULL;
    for (int i = 0; i < room_count; i++){
        Room *r = (Room *)rooms[i];
        if (r && r->id == room_id){
            room = r;
            break;
        }
    }

    if (!room){
        return GE_NO_SUCH_ROOM;
    }

    //Store room dimensions
    int width = room_get_width(room);
    int height = room_get_height(room);

    //Return fail if dimensions are 0 or negative
    if (width <= 0 || height <= 0){
        return INTERNAL_ERROR;
    }

    //Create variable to store all tiles in grid
    size_t tiles = (size_t)width * (size_t)height;
    char *grid = (char *)malloc(tiles);

    //Return if memory fail
    if (!grid){
        return NO_MEMORY;
    }

    //Render the ground using grid
    Status status = room_render(room, &eng->charset, grid, width, height);
    //Free and return if fail
    if (status != OK){
        free(grid);
        return status;
    }

    //Create variables to store position
    int pos_x = 0;
    int pos_y = 0;

    //Get position and free and return if fail
    if (player_get_position(eng->player, &pos_x, &pos_y) != OK){
        free(grid);
        return INTERNAL_ERROR;
    }

    //Check if position is in bounds
    if (pos_x >= 0 && pos_y >= 0 && pos_x < width && pos_y < height){
        //Set current tile to player character
        grid[(size_t)pos_y * (size_t)width + (size_t)pos_x] = eng->charset.player;
    }

    //Create variable to store size of output array and allocate space
    size_t out_len = (((size_t)width + sizeof(char)) * (size_t)height) + sizeof(char);
    char *out = (char *)malloc(out_len);

    //Return and free if memory issue
    if (!out){
        free(grid);
        return NO_MEMORY;
    }

    //Create variable tp store index
    size_t idx = 0;

    //Loop through each row
    for (int row = 0; row < height; row++){
        //Store each row in output
        memcpy(&out[idx], &grid[(size_t)row * (size_t)width], (size_t)width);

        //Incremend to next row by skipping the width
        idx += (size_t)width;
        //Store new line after row and increment index
        out[idx] = '\n';
        idx += sizeof(char);
    }
    //Finish with null character
    out[idx] = '\0';

    //Free and return
    free(grid);
    *str_out = out;
    return OK;
}

Status game_engine_render_room(const GameEngine *eng, int room_id, char **str_out){
    //Return if engine or output is null
    if (!eng){
        return INVALID_ARGUMENT;
    }
    if (!str_out){
        return NULL_POINTER;
    }

    //Set output to default
    *str_out = NULL;

    //Same look for matched id
    const void *const *rooms = NULL;
    int room_count = 0;

    if (graph_get_all_payloads(eng->graph, &rooms, &room_count) != GRAPH_STATUS_OK){
        return INTERNAL_ERROR;
    }

    Room *room = NULL;
    for (int i = 0; i < room_count; i++){
        Room *r = (Room *)rooms[i];
        if (r && r->id == room_id){
            room = r;
            break;
        }
    }

    if (!room){
        return GE_NO_SUCH_ROOM;
    }

    //Create variables to store dimensions
    int width = room_get_width(room);
    int height = room_get_height(room);

    //Return fail if out of bounds directions
    if (width <= 0 || height <= 0){
        return INTERNAL_ERROR;
    }

    //Same as before
    size_t tiles = (size_t)width * (size_t)height;
    char *grid = (char *)malloc(tiles);
    if (!grid){
        return NO_MEMORY;
    }

    //Render room
    Status status = room_render(room, &eng->charset, grid, width, height);
    //Return if fail
    if (status != OK){
        free(grid);
        return status;
    }

    //Same as before
    size_t out_len = ((size_t)width + sizeof(char)) * (size_t)height + sizeof(char);
    char *out = (char *)malloc(out_len);
    if (!out){
        free(grid);
        return NO_MEMORY;
    }

    size_t idx = 0;
    for (int row = 0; row < height; row++){
        memcpy(&out[idx], &grid[(size_t)row * (size_t)width], (size_t)width);
        idx += (size_t)width;
        out[idx] = '\n';
        idx += sizeof(char);
    }
    out[idx] = '\0';

    //Free and output
    free(grid);
    *str_out = out;
    return OK;
}

Status game_engine_get_room_ids(const GameEngine *eng, int **ids_out, int *count_out){
    //Return if engine or outputs are null
    if (!eng){
        return INVALID_ARGUMENT;
    }
    if (!ids_out || !count_out){
        return NULL_POINTER;
    }

    //Set outputs to default
    *ids_out = NULL;
    *count_out = 0;

    //Create variables to store all rooms and count
    const void *const *rooms = NULL;
    int room_count = 0;

    //Call get all rooms
    GraphStatus graph_status = graph_get_all_payloads(eng->graph, &rooms, &room_count);

    //Return if fail
    if (graph_status != GRAPH_STATUS_OK){
        if (graph_status == GRAPH_STATUS_NO_MEMORY){
            return NO_MEMORY;
        }
        return INTERNAL_ERROR;
    }

    //Return if no rooms or negative
    if (room_count <= 0){
        return OK;
    }

    //Allocate space for all room ids
    int *ids = (int *)malloc(sizeof(int) * (size_t)room_count);
    //Return if memory error
    if (!ids){
        return NO_MEMORY;
    }

    //Loop through all rooms
    for (int i = 0; i < room_count; i++){
        //Set current room
        Room *r = (Room *)rooms[i];
        //Add id if room exists
        if (r){
            ids[i] = r->id;
        } 
        else{
            ids[i] = -1;
        }
    }

    //Output all ids and amount
    *ids_out = ids;
    *count_out = room_count;
    return OK;
}

void game_engine_free_string(void *ptr){
    free(ptr);
}