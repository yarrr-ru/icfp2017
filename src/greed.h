#pragma once
#include "launcher.h"

using Path = std::vector<Edge>;

std::pair<int64_t, Path> get_path(
    Vertex from,
    Vertex to,
    const Map& map,
    const std::vector<Punter>& river_owners
);

Edge get_worst_edge(Vertex from, Vertex to, const Map& map);
int64_t get_worst_edge_dist(Vertex from, Vertex to, const Map& map);
std::pair<int64_t, Edge> get_worst_edge_all(Vertex from, Vertex to, const Map& map);
std::vector<std::pair<int64_t, River>> get_greed_moves(const Map& map, Punter owner);

River make_move_surround_all_lamdas(const Map& map);
River make_move_greed_only_st(const Map& map);
River make_move_scored_only_st(const Map& map);
River make_move_greed(const Map& map);
River make_move_greed_st(const Map& map);
River make_move_scored_st(const Map& map);
River make_move_greed_surround_st(const Map& map);
River make_move_scored_surround_st(const Map& map);
River make_move_greed_st_maxcut(const Map& map);
Futures make_futures_random(const Map& map);
Futures make_futures_shortest_path(const Map& map);
