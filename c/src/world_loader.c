#include "world_loader.h"
#include "datagen.h"
#include "graph.h"
#include "room.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//Compare and destroy from A0
static int compare_rooms(const void *a, const void *b){
    const Room *ra = (const Room *)a;
    const Room *rb = (const Room *)b;

    if (!ra && !rb) return 0;
    if (!ra) return -1;
    if (!rb) return 1;

    if (ra->id < rb->id) return -1;
    if (ra->id > rb->id) return 1;
    return 0;
}
static void destroy_room(void *payload){
     room_destroy((Room *)payload);
}

//Load charset
static void loader_set_default_charset(Charset *charset_out){
    charset_out->wall = '#';
    charset_out->floor = '.';
    charset_out->player = '@';
    charset_out->treasure = '$';
    charset_out->portal = 'X';
    charset_out->pushable = 'O';
}

//Create copy of charset for ownership
static void loader_copy_charset(Charset *charset_out, const DG_Charset *datagen_chars){
    loader_set_default_charset(charset_out);

    if (!datagen_chars){
        return;
    }

    charset_out->wall = datagen_chars->wall;
    charset_out->floor = datagen_chars->floor;
    charset_out->player = datagen_chars->player;
    charset_out->treasure = datagen_chars->treasure;
    charset_out->portal = datagen_chars->portal;
    charset_out->pushable = datagen_chars->pushable;
}

//Create copy of loaded grid
static Status loader_copy_floor_grid(Room *r, const DG_Room *dg){
    if (!dg->floor_grid){
        return OK;
    }

    size_t tiles = (size_t)dg->width * (size_t)dg->height;
    bool *grid = malloc(sizeof(bool) * tiles);
    if (!grid){
        return NO_MEMORY;
    }

    memcpy(grid, dg->floor_grid, sizeof(bool) * tiles);
    return room_set_floor_grid(r, grid);
}

//Create copy of portals
static Status loader_copy_portals(Room *r, const DG_Room *dg){
    if (dg->portal_count <= 0){
        return OK;
    }

    Portal *portals = malloc(sizeof(Portal) * (size_t)dg->portal_count);
    if (!portals){
        return NO_MEMORY;
    }

    memset(portals, 0, sizeof(Portal) * (size_t)dg->portal_count);

    for (int i = 0; i < dg->portal_count; i++){
        portals[i].id = dg->portals[i].id;
        portals[i].x = dg->portals[i].x;
        portals[i].y = dg->portals[i].y;
        portals[i].target_room_id = dg->portals[i].neighbor_id;
        portals[i].name = NULL;
    }

    return room_set_portals(r, portals, dg->portal_count);
}

//Free all treausure names
static void loader_free_treasure_names(Treasure *treasures, int count){
    for (int i = 0; i < count; i++){
        free(treasures[i].name);
    }
}

//Create copies of all treausures
static Status loader_copy_treasures(Room *r, const DG_Room *dg){
    if (dg->treasure_count <= 0){
        return OK;
    }

    Treasure *treasures = malloc(sizeof(Treasure) * (size_t)dg->treasure_count);
    if (!treasures){
        return NO_MEMORY;
    }

    memset(treasures, 0, sizeof(Treasure) * (size_t)dg->treasure_count);

    for (int i = 0; i < dg->treasure_count; i++){
        treasures[i].id = dg->treasures[i].global_id;
        treasures[i].starting_room_id = dg->id;
        treasures[i].initial_x = dg->treasures[i].x;
        treasures[i].initial_y = dg->treasures[i].y;
        treasures[i].x = dg->treasures[i].x;
        treasures[i].y = dg->treasures[i].y;
        treasures[i].collected = false;

        if (dg->treasures[i].name){
            size_t len = strlen(dg->treasures[i].name) + 1;
            treasures[i].name = malloc(len);
            if (!treasures[i].name){
                loader_free_treasure_names(treasures, i);
                free(treasures);
                return NO_MEMORY;
            }
            memcpy(treasures[i].name, dg->treasures[i].name, len);
        }
    }

    return room_set_treasures(r, treasures, dg->treasure_count);
}

//Free pushables names
static void loader_free_pushable_names(Pushable *pushables, int count){
    for (int i = 0; i < count; i++){
        free(pushables[i].name);
    }
}

//Create copies of pushables
static Status loader_copy_pushables(Room *r, const DG_Room *dg){
    if (dg->pushable_count <= 0){
        return OK;
    }

    Pushable *pushables = malloc(sizeof(Pushable) * (size_t)dg->pushable_count);
    if (!pushables){
        return NO_MEMORY;
    }

    memset(pushables, 0, sizeof(Pushable) * (size_t)dg->pushable_count);

    for (int i = 0; i < dg->pushable_count; i++){
        pushables[i].id = dg->pushables[i].id;
        pushables[i].initial_x = dg->pushables[i].x;
        pushables[i].initial_y = dg->pushables[i].y;
        pushables[i].x = dg->pushables[i].x;
        pushables[i].y = dg->pushables[i].y;

        if (dg->pushables[i].name){
            size_t len = strlen(dg->pushables[i].name) + 1;
            pushables[i].name = malloc(len);
            if (!pushables[i].name){
                loader_free_pushable_names(pushables, i);
                free(pushables);
                return NO_MEMORY;
            }
            memcpy(pushables[i].name, dg->pushables[i].name, len);
        }
    }

    r->pushables = pushables;
    r->pushable_count = dg->pushable_count;
    return OK;
}

//Fill rooms using copies
static Status loader_populate_room(Room *r, const DG_Room *dg){
    Status status = loader_copy_floor_grid(r, dg);
    if (status != OK){
        return status;
    }

    status = loader_copy_portals(r, dg);
    if (status != OK){
        return status;
    }

    status = loader_copy_treasures(r, dg);
    if (status != OK){
        return status;
    }

    status = loader_copy_pushables(r, dg);
    if (status != OK){
        return status;
    }

    return OK;
}

//Fill in graph with rooms and connections
static Status loader_connect_rooms(Graph *g){
    const void *const *rooms = NULL;
    int rooms_count = 0;

    if (graph_get_all_payloads(g, &rooms, &rooms_count) != GRAPH_STATUS_OK){
        return INTERNAL_ERROR;
    }

    for (int i = 0; i < rooms_count; i++){
        Room *src = (Room *)rooms[i];

        for (int j = 0; j < src->portal_count; j++){
            int target_id = src->portals[j].target_room_id;
            if (target_id < 0){
                continue;
            }

            Room *dest = NULL;
            for (int k = 0; k < rooms_count; k++){
                Room *current = (Room *)rooms[k];
                if (current->id == target_id){
                    dest = current;
                    break;
                }
            }

            if (!dest){
                return GE_NO_SUCH_ROOM;
            }

            graph_connect(g, src, dest);
        }
    }

    return OK;
}

Status loader_load_world(const char *config_file, Graph **graph_out, Room **first_room_out, int *num_rooms_out, Charset *charset_out){
    //Check for invalid arguements
    if (!config_file || !graph_out || !first_room_out || !num_rooms_out || !charset_out){
        return INVALID_ARGUMENT;
    }

    //Set outputs to default
    *graph_out = NULL;
    *first_room_out = NULL;
    *num_rooms_out = 0;

    //Start datagen
    int datagen_status = start_datagen(config_file);

    //Return if datagen fails
    if (datagen_status != DG_OK){
        if (datagen_status == DG_ERR_CONFIG){
            return WL_ERR_CONFIG;
        }
        if (datagen_status == DG_ERR_OOM){
            return NO_MEMORY;
        }
        return WL_ERR_DATAGEN;
    }

    loader_copy_charset(charset_out, dg_get_charset());

    //Create graph and store its status
    Graph *g = NULL;
    GraphStatus graph_status = graph_create(compare_rooms, destroy_room, &g);

    //Check if fails and stop the datagen
    if (graph_status != GRAPH_STATUS_OK){
        stop_datagen();
        if (graph_status == GRAPH_STATUS_NO_MEMORY){
            return NO_MEMORY;
        }
        return INTERNAL_ERROR;
    }

    //Create variables to track rooms
    Room *first_room = NULL;
    int room_count = 0;

    //Loop through all rooms in datagen
    while (has_more_rooms()){

        //Store datagens next room
        DG_Room dg = get_next_room();

        //Create new room using values from datagen
        Room *r = room_create(dg.id, NULL, dg.width, dg.height);

        //Check if fails to free and stop datagen
        if (!r){
            graph_destroy(g);
            stop_datagen();
            return NO_MEMORY;
        }

        Status status = loader_populate_room(r, &dg);
        if (status != OK){
            room_destroy(r);
            graph_destroy(g);
            stop_datagen();
            return status;
        }

        graph_status = graph_insert(g, r);
        if (graph_status != GRAPH_STATUS_OK){
            room_destroy(r);
            graph_destroy(g);
            stop_datagen();
            if (graph_status == GRAPH_STATUS_NO_MEMORY){
                return NO_MEMORY;
            }
            return INTERNAL_ERROR;
        }

        if (!first_room){
            first_room = r;
        }

        room_count++;
    }
        
    Status status = loader_connect_rooms(g);
    if (status != OK){
        graph_destroy(g);
        stop_datagen();
        return status;
    }
            
    //Stop datagen after copies made
    stop_datagen();

    //Set outputs
    *graph_out = g;
    *first_room_out = first_room;
    *num_rooms_out = room_count;

    return OK;
}