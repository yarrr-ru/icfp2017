#pragma once
#include "launcher.h"

std::pair<int64_t, std::vector<Edge>> get_path(
    Vertex from,
    Vertex to,
    const Map& map,
    const std::vector<Punter>& river_owners
);

Edge get_worst_edge(Vertex from, Vertex to, const Map& map);

River make_move_greed_only_st(const Map& map);
River make_move_greed(const Map& map);
River make_move_greed_st(const Map& map);
