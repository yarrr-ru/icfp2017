//
#pragma once
#include "json.hpp"
#include <vector>
#include <map>
#include <iostream>
#include <cstddef>
#include <cassert>

using json = nlohmann::json;

using River = std::pair<int32_t, int32_t>;
using Punter = int32_t;
using Site = int32_t;
using Vertex = size_t;

static constexpr Punter kNoOwner = -1;

struct Edge {
  Vertex from;
  Vertex to;
  Punter owner;
  size_t river_index;

  Edge() : from(0), to(0), owner(kNoOwner), river_index(0) {}

  Edge(Vertex from, Vertex to, Punter owner, size_t river_index):
    from(from),
    to(to),
    owner(owner),
    river_index(river_index) {
  }
};

json read_json();

void write_json(const json& result);

inline River make_river(const json& river) {
    return {river["source"], river["target"]};
}

class Map {
public:
  Punter punter;
  size_t punters;
  std::vector<Site> sites;
  std::vector<Site> mines;
  std::vector<River> rivers;
  std::map<River, Punter> claims;
  std::vector<json> moves;

  std::vector<std::vector<Edge>> graph;
  std::vector<char> is_mine;

  explicit Map(const json& old_state);
  Map(const json& old_state, const json& moves);

  json to_json() {
    json_state["moves"] = moves;
    return json_state;
  }

private:
  void add_moves(const json& new_moves);

  void add_claim(const json& claim);

  size_t vertex_id(int32_t site);
  void build_graph();
  void clear_graph();

  json json_state;
};

