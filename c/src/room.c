#include "room.h"
#include <stdlib.h>
#include <string.h>

//HELPER: CHECK IN BOUNDS
static bool room_in_bounds(const Room *r, int x, int y){
    return r && x >= 0 && y >= 0 && x < r->width && y < r->height;
}

//RETURN TILECHAR
static char room_base_tile_char(const Room *r, const Charset *charset, int x, int y){
    if (r->floor_grid){
        return (char)(r->floor_grid[y * r->width + x] ? charset->floor : charset->wall);
    }

    if (x == 0 || y == 0 || x == r->width - 1 || y == r->height - 1){
        return charset->wall;
    }

    return charset->floor;
}

//RENDER
static void room_render_base_layer(const Room *r, const Charset *charset, char *buffer){
    for (int row = 0; row < r->height; row++){
        for (int col = 0; col < r->width; col++){
            buffer[row * r->width + col] = room_base_tile_char(r, charset, col, row);
        }
    }
}

//Render treasures
static void room_render_treasures_layer(const Room *r, const Charset *charset, char *buffer){
    for (int i = 0; i < r->treasure_count; i++){
        const Treasure *t = &r->treasures[i];
        if (!t->collected && room_in_bounds(r, t->x, t->y)){
            buffer[t->y * r->width + t->x] = charset->treasure;
        }
    }
}

//Render portals
static void room_render_portals_layer(const Room *r, const Charset *charset, char *buffer){
    for (int i = 0; i < r->portal_count; i++){
        const Portal *p = &r->portals[i];
        if (room_in_bounds(r, p->x, p->y)){
            buffer[p->y * r->width + p->x] = charset->portal;
        }
    }
}

//render pushables
static void room_render_pushables_layer(const Room *r, const Charset *charset, char *buffer){
    for (int i = 0; i < r->pushable_count; i++){
        const Pushable *push = &r->pushables[i];
        if (room_in_bounds(r, push->x, push->y)){
            buffer[push->y * r->width + push->x] = charset->pushable;
        }
    }
}


Room *room_create(int id, const char *name, int width, int height){
    //Ensure positive dimensions and clamp to 1
    if (width < 1){
        width = 1;
    }
    if (height < 1){
        height = 1;
    }

    //Dynamically allocate memory for a room
    Room *r = malloc(sizeof(Room));
    if (!r){
        return NULL;
    }

    //Set to defined vals
    r->width = width;
    r->height = height;
    r->id = id;

    //Set name to null if null
    if (!name){
        r->name = NULL;
    }
    else{
        //Allocate memory for the name
        r->name = malloc(strlen(name) + 1);

        //Ensure memory was allocated
        if (!r->name){
            free(r);
            return NULL;
        }

        //Copy the name into the rooms name
        strcpy(r->name, name);
    }

    //Set vals to initial defaults
    r->floor_grid = NULL;
    r->portals = NULL;
    r->portal_count = 0;
    r->treasures = NULL;
    r->treasure_count = 0;
    r->neighbors = NULL;
    r->neighbor_count = 0;
    r->pushables = NULL;
    r->pushable_count = 0;
    r->switches = NULL;
    r->switch_count = 0;

    //Return
    return r;
}

int room_get_width(const Room *r){
    //Return 0 if room is null
    if (!r){
        return 0;
    }

    //Return width
    return r->width;
}

int room_get_height(const Room *r){
    //Return 0 if room is null
    if (!r){
        return 0;
    }

    //Return width
    return r->height;
}

Status room_set_floor_grid(Room *r, bool *floor_grid){
    //Return invalid if room is null
    if (!r){
        return INVALID_ARGUMENT;
    }

    //Free previous grid and transfer ownership to r
    free(r->floor_grid);
    r->floor_grid = floor_grid;

    return OK;
}

Status room_set_portals(Room *r, Portal *portals, int portal_count){
    //Return invalid if room is null
    if (!r){
        return INVALID_ARGUMENT;
    }

    //Return invalid if there are null portals but the count is greater than 0
    if (portal_count > 0 && portals == NULL){
        return INVALID_ARGUMENT;
    }

    //Returns invalid if negative portals
    if (portal_count < 0){
        return INVALID_ARGUMENT;
    }

    //Free previous portals and names
    if (r->portals){
        for (int i = 0; i < (r->portal_count); i++){
            free(r->portals[i].name);
        }
        free(r->portals);
    }

    //Transfer ownership and set count
    r->portals = portals;
    r->portal_count = portal_count;
    return OK;
}

Status room_set_treasures(Room *r, Treasure *treasures, int treasure_count){
    //Return invalid if room is null
    if (!r){
        return INVALID_ARGUMENT;
    }

    //Return invalid if there are null treasures but the count is greater than 0
    if (treasure_count > 0 && treasures == NULL){
        return INVALID_ARGUMENT;
    }

    //Returns invalid if negative treasures
    if (treasure_count < 0){
        return INVALID_ARGUMENT;
    }

    //Free previous treasures and names
    if (r->treasures){
        for (int i = 0; i < (r->treasure_count); i++){
            free(r->treasures[i].name);
        }
        free(r->treasures);
    }

    //Transfer ownership and set count
    r->treasures = treasures;
    r->treasure_count = treasure_count;
    return OK;
}

Status room_place_treasure(Room *r, const Treasure *treasure){
    //Return invalid if null room or treasure
    if (!r || !treasure){
        return INVALID_ARGUMENT;
    }

    //Increment treasure count
    int temp = r->treasure_count + 1;

    //Reallocate to fit new trasure
    Treasure *new = realloc(r->treasures, sizeof(Treasure) * temp);
    if (!new){
        return NO_MEMORY;
    }

    //Transfer ownership
    r->treasures = new;

    //Define new treasure
    Treasure t = *treasure;

    //Set name to null if null
    if (!treasure->name){
        t.name = NULL;
    }
    else{
        //Allocate memory for the name
        t.name = malloc(strlen(treasure->name) + 1);

        //Ensure memory was allocated
        if (!t.name){
            return NO_MEMORY;
        }

        //Copy the name into the new treasures name
        strcpy(t.name, treasure->name);
    }

    //Add new treasure to treasures and increment count
    r->treasures[r->treasure_count] = t;
    r->treasure_count = temp;
    return OK;
}

int room_get_treasure_at(const Room *r, int x, int y){
    //Return -1 if room is null
    if (!r){
        return -1;
    }

    //Loop through each treasure to see if it matches the position
    for (int i = 0; i < (r->treasure_count); i++){
        //Set pointer to address of treasure at current index
        const Treasure *t = &r->treasures[i];

        //Checks if treasure was already collected and skips
        if (t->collected){
            continue;
        }
        //Checks if position matches
        if (t->x == x && t->y == y){
            return t->id;
        }
    }

    return -1;
}

int room_get_portal_destination(const Room *r, int x, int y){
    //Return -1 if room is null
    if (!r){
        return -1;
    }

    //Loop through all portals
    for (int i = 0; i < r->portal_count; i++){
        //Set pointer to the adress of portal at current index
        const Portal *p = &r->portals[i];

        //Check if current portal matches position
        if (p->x == x && p->y == y){
            //Return the new room id
            return p->target_room_id;
        }
    }

    return -1;
}

bool room_is_walkable(const Room *r, int x, int y){
    //Check if room is null and return false
    if (!r){
        return false;
    }

    //Checks if position out of bounds
    if (x < 0 || y < 0 || x >= r->width || y >= r->height){
        return false;
    }

    //Check if pushable is not walkable
    if (room_has_pushable_at(r, x, y, NULL)){
        return false;
    }

    //Checks if floor grid exists
    if (r->floor_grid){
        //Returns the bool value from the array (row * number of columns + column)
        return r->floor_grid[y * r->width + x];
    }

    //Checks if position is on the borders
    if (x == 0 || y == 0 || x == r->width - 1 || y == r->height - 1){
        return false;
    }

    return true;
}

RoomTileType room_classify_tile(const Room *r, int x, int y, int *out_id){
    //Set output to default if it exists
    if (out_id){
        *out_id = -1;
    }

    //Return if room is null
    if (!r){
        return ROOM_TILE_INVALID;
    }

    //Return if tile is out of bounds
    if (x < 0 || y < 0 || x >= r->width || y >= r->height){
        return ROOM_TILE_INVALID;
    }

    //Create variable to check if treasure
    int treasure_id = room_get_treasure_at(r, x, y);

    //Return the treasure id if the tile is is a treasure tile
    if (treasure_id != -1){
        if (out_id){
            *out_id = treasure_id;
        }
        return ROOM_TILE_TREASURE;
    }

    //Create variable to check if portal
    int portal_dest = room_get_portal_destination(r, x, y);

    //Return the portals destination if the tile is a portal tile
    if (portal_dest != -1){
        if (out_id){
            *out_id = portal_dest;
        }
        return ROOM_TILE_PORTAL;
    }

    //Set indext to default
    int push_idx = -1;

    //Return the pushable tile if it is pushable
    if (room_has_pushable_at(r, x, y, &push_idx)){
        if (out_id){
            *out_id = push_idx;
        }
        return ROOM_TILE_PUSHABLE;
    }

    //Check if tile is walkable therefore floor
    if (room_is_walkable(r, x, y)){
        return ROOM_TILE_FLOOR;
    }

    //Return last option wall
    return ROOM_TILE_WALL;
}

Status room_render(const Room *r, const Charset *charset, char *buffer, int buffer_width, int buffer_height){
    //Check if pointers are null
    if (!r || !charset || !buffer){
        return INVALID_ARGUMENT;
    }

    //CHeck buffers exist
    if (buffer_width != r->width || buffer_height != r->height){
        return INVALID_ARGUMENT;
    }

    //Render all layers
    room_render_base_layer(r, charset, buffer);
    room_render_treasures_layer(r, charset, buffer);
    room_render_portals_layer(r, charset, buffer);
    room_render_pushables_layer(r, charset, buffer);

    return OK;
}

Status room_get_start_position(const Room *r, int *x_out, int *y_out){
    //Return invalid if room or outputs are null
    if (!r || !x_out || !y_out){
        return INVALID_ARGUMENT;
    }

    //First portal location
    if (r->portal_count > 0){
        *x_out = r->portals[0].x;
        *y_out = r->portals[0].y;
        return OK;
    }

    //Loop through all tiles
    for (int row = 0; row < r->height; row++){
        for (int col = 0; col < r->width; col++){
            //Set and return position of first walkable tile
            if (room_is_walkable(r, col, row)){
                *x_out = col;
                *y_out = row;
                return OK;
            }
        }
    }

    return ROOM_NOT_FOUND;
}

void room_destroy(Room *r){
    //Return if room is already destroyed
    if (!r){
        return;
    }

    //Free the rooms name
    free(r->name);

    //Free the rooms grid
    free(r->floor_grid);

    //Loop through all portals if there are any
    if (r->portals){
        //Loop through all portals
        for (int i = 0; (i < r->portal_count); i++){
            //Free each portals name
            free(r->portals[i].name);
        }
        //Free the array holding all portals
        free(r->portals);
    }
    //Set count to 0
    r->portal_count = 0;

    //Repeat for treasures
    if (r->treasures){
        for (int i = 0; i < (r->treasure_count); i++){
            free(r->treasures[i].name);
        }
        free(r->treasures);
    }
    r->treasure_count = 0;

    //Free neighbors
    free(r->neighbors);

    //Repeat for pushables
    if (r->pushables){
        for (int i = 0; i < r->pushable_count; i++){
            free(r->pushables[i].name);
        }
        free(r->pushables);
    }
    r->pushable_count = 0;

    //Free switches
    free(r->switches);

    //Free the room
    free(r);
}

int room_get_id(const Room *r){
    //Check for null pointer
    if (!r){
        return -1;
    }

    //Return room id
    return r->id;
}

Status room_pick_up_treasure(Room *r, int treasure_id, Treasure **treasure_out){
    //Check for null
    if (!r || !treasure_out){
        return INVALID_ARGUMENT;
    }

    //Check if id is negative
    if (treasure_id < 0){
        return INVALID_ARGUMENT;
    }

    //Set output to default
    *treasure_out = NULL;

    //Loop through all treasures
    for (int i = 0; i < r->treasure_count; i++){

        //Set treasure to current
        Treasure *t = &r->treasures[i];

        //SKip non matches
        if (t->id != treasure_id){
            continue;
        }

        //Check if already collected
        if (t->collected){
            return INVALID_ARGUMENT;
        }

        //Set to collected
        t->collected = true;

        //Set output and return
        *treasure_out = t;
        return OK;
    }

    return ROOM_NOT_FOUND;
}

void destroy_treasure(Treasure *t){
    //Check for null pointer
    if (!t){
        return;
    }

    //Free name and treasure
    free(t->name);
    free(t);
}

bool room_has_pushable_at(const Room *r, int x, int y, int *pushable_idx_out){
    //Check if pointer exists and set to default
    if (pushable_idx_out){
        *pushable_idx_out = -1;
    }

    //Check if room is null
    if (!r){
        return false;
    }

    //Loop through all pushables
    for (int i = 0; i < r->pushable_count; i++){

        //Create pushable and set to current index
        const Pushable *p = &r->pushables[i];

        //Check if matches location
        if (p->x == x && p->y == y){
            //Check if output exists
            if (pushable_idx_out){
                //Set index as output
                *pushable_idx_out = i;
            }
            return true;
        }
    }

    return false;
}

Status room_try_push(Room *r, int pushable_idx, Direction dir){
    //Check if pointer is null
    if (!r){
        return INVALID_ARGUMENT;
    }
    //Check if index is positive
    if (pushable_idx < 0 || pushable_idx >= r->pushable_count){
        return INVALID_ARGUMENT;
    }

    //Set direction positions to default
    int dx = 0;
    int dy = 0;

    //Adjust direction position based on dir
    if (dir == DIR_NORTH){
        dy = -1;
    }
    else if (dir == DIR_SOUTH){
        dy = 1;
    }
    else if (dir == DIR_EAST){
        dx = 1;
    }
    else if (dir == DIR_WEST){
        dx = -1;
    }
    else{
        return INVALID_ARGUMENT;
    }

    //Create pushable pointer to store the pushable at the current index
    Pushable *push = &r->pushables[pushable_idx];

    //Store new position
    int dest_x = push->x + dx;
    int dest_y = push->y + dy;

    

    // //Check borders
    // if (dest_x < 0 || dest_y < 0 || dest_x >= r->width || dest_y >= r->height){
    //     return ROOM_IMPASSABLE;
    // }

    // //Loop through all pushables
    // for (int i = 0; i < r->pushable_count; i++){
    //     if (i == pushable_idx){
    //         continue;
    //     }

    //     //Check dor impassable
    //     if (r->pushables[i].x == dest_x && r->pushables[i].y == dest_y){
    //         return ROOM_IMPASSABLE;
    //     }
    // }

    //Check the new tile
    int out_id = -1;
    RoomTileType dest_type = room_classify_tile(r, dest_x, dest_y, &out_id);
    //Push if only floor
    if (dest_type != ROOM_TILE_FLOOR){
        return ROOM_IMPASSABLE;
    }

    //Set new positions and return
    push->x = dest_x;
    push->y = dest_y;
    return OK;
} 
