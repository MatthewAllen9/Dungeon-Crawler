#ifndef GAME_ENGINE_HELPER_H
#define GAME_ENGINE_HELPER_H

#include "game_engine.h"

Status game_engine_move_player_2(GameEngine *eng, Direction dir);
Status game_engine_use_portal(GameEngine *eng);
Status game_engine_get_charset_helper(const GameEngine *eng, Charset *charset_out);
Status game_engine_get_total_treasure_count(const GameEngine *eng, int *count_out);

#endif