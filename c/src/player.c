#include "player.h"
#include <stdlib.h>

Status player_create(int initial_room_id, int initial_x, int initial_y, Player **player_out){
    //Check that non null pointer is passed
    if (!player_out){
        return INVALID_ARGUMENT;
    }

    //Allocate space for the player
    Player *p = malloc(sizeof(Player));

    //Check for memory allocation issues
    if (!p){
        *player_out = NULL;
        return NO_MEMORY;
    }

    //Set up player initial vals
    p->room_id = initial_room_id;
    p->x = initial_x;
    p->y = initial_y;

    //Set collected treasure count to default
    p->collected_treasures = NULL;
    p->collected_count = 0;

    //Store the player in pointer and return
    *player_out = p;
    return OK;
}

void player_destroy(Player *p){
    //Check null pointer
    if (!p){
        return;
    }

    //Free the players collected treasures
    free(p->collected_treasures);

    //Set treasures to default
    p->collected_treasures = NULL;
    p->collected_count = 0;

    //Free memory for player
    free(p);
}


int player_get_room(const Player *p){
    //Check for null and return -1
    if (p == NULL){ 
        return -1;
    }

    //Return room id
    return p->room_id;
}

Status player_get_position(const Player *p, int *x_out, int *y_out){
    //Check that all pointers are not null to return position
    if (p && x_out && y_out){
        *x_out = p->x;
        *y_out = p->y;
        return OK;

    }
    //Return if null pointer
    return INVALID_ARGUMENT;
}

Status player_set_position(Player *p, int x, int y){
    //Check if pointer is null
    if (!p){
        return INVALID_ARGUMENT;
    }

    //Set position
    p->x = x;
    p->y = y;
    return OK;
}

Status player_move_to_room(Player *p, int new_room_id){
    //Check if null pointer
    if (!p){
        return INVALID_ARGUMENT;
    }

    //Set to new room
    p->room_id = new_room_id;
    return OK;
}

Status player_reset_to_start(Player *p, int starting_room_id, int start_x, int start_y){
    //Check if null pointer
    if (!p){
        return INVALID_ARGUMENT;
    }

    //Free collected array
    free(p->collected_treasures);

    //Reset treasures collected
    p->collected_treasures = NULL;
    p->collected_count = 0;

    //Return to starting position
    p->room_id = starting_room_id;
    p->x = start_x;
    p->y = start_y;
    return OK;
}

Status player_try_collect(Player *p, Treasure *treasure){
    //Check if any null pointers
    if (!p || !treasure){
        return NULL_POINTER;
    }

    //Check if treasure is already in array
    for (int i = 0; i < p->collected_count; i++){
        if (p->collected_treasures[i] && p->collected_treasures[i]->id == treasure->id){
            return INVALID_ARGUMENT;
        }
    }

    //Increment treasure array
    int new_count = p->collected_count + 1;
    Treasure **new_arr = realloc(p->collected_treasures, sizeof(Treasure *) * (size_t)new_count);

    //Check for memory error
    if (!new_arr){
        return NO_MEMORY;
    }

    //Set new array and add new treasure
    p->collected_treasures = new_arr;
    p->collected_treasures[p->collected_count] = treasure;
    p->collected_count = new_count;

    //Set as collected
    treasure->collected = true;

    //Return
    return OK;
}

bool player_has_collected_treasure(const Player *p, int treasure_id){

    //Check if pointers null or negative id
    if (!p || treasure_id < 0){
        return false;
    }

    //Loop through all collected treasures
    for (int i = 0; i < p->collected_count; i++){
        //Check if id is in the collcted array
        if (p->collected_treasures[i] && p->collected_treasures[i]->id == treasure_id){
            return true;
        }
    }

    //Return
    return false;
}

int player_get_collected_count(const Player *p){
    //Check if pointer is null
    if (!p){
        return 0;
    }

    //Return count
    return p->collected_count;
}

//Create constant treasure pointer
const Treasure * const *
player_get_collected_treasures(const Player *p, int *count_out){
    //Cgeck for null pointers
    if (!p || !count_out){
        return NULL;
    }

    //Set count output to the collected count
    *count_out = p->collected_count;

    //Return collected treasures
    return (const Treasure * const *)p->collected_treasures;
}
