#pragma once
#include "map.h"

River make_move_as_an_endspiel_pidor(const Map& map);
River make_move_as_a_pidor_only(const Map& map);
River make_move_as_a_pidor(const Map& map);

River make_move_surround_pidor_endspiel(const Map& map);
River make_move_scored_pidor_endspiel(const Map& map);
River make_move_greed_st_pidor_endspiel(const Map& map);
