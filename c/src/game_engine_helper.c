#include "game_engine_helper.h"
#include "graph.h"
#include "player.h"
#include "room.h"
#include "world_loader.h"

#include <stdlib.h>
#include <string.h>

static Status game_engine_direction_delta(Direction dir, int *dx_out, int *dy_out);
static Room *game_engine_find_room_by_id(const GameEngine *eng, int room_id);
static Status game_engine_handle_floor_move(Player *player, int new_x, int new_y);
static Status game_engine_handle_treasure_move(GameEngine *eng, Room *room, int treasure_id);
static Status game_engine_handle_pushable_move(GameEngine *eng, Room *room, int pushable_idx, Direction dir, int new_x, int new_y);
static Status game_engine_handle_portal_move(GameEngine *eng, int dest_room_id);
static Treasure *game_engine_find_treasure_by_id(Room *room, int treasure_id);

Status game_engine_move_player_2(GameEngine *eng, Direction dir){
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
            return player_set_position(eng->player, new_x, new_y);
        default:
            return INTERNAL_ERROR;
    }
}

Status game_engine_use_portal(GameEngine *eng){
    if (!eng || !eng->player){
        return INTERNAL_ERROR;
    }

    int room_id = player_get_room(eng->player);

    Room *current_room = game_engine_find_room_by_id(eng, room_id);
    if (!current_room){
        return INTERNAL_ERROR;
    }

    int x = 0;
    int y = 0;
    player_get_position(eng->player, &x, &y);

    int out_id = -1;
    RoomTileType tile_type = room_classify_tile(current_room, x, y, &out_id);

    if (tile_type != ROOM_TILE_PORTAL){
        return ROOM_NO_PORTAL;
    }

    return game_engine_handle_portal_move(eng, out_id);
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