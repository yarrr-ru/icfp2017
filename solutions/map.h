//
#pragma once
#include "json.hpp"
#include "draw.h"
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
using Score = int64_t;

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

inline River make_river(const json& river) {
  return {river["source"], river["target"]};
}

class Map {
public:
  const Punter punter;
  const size_t punters;

  std::vector<std::vector<Edge>> graph;
  std::vector<char> is_lambda;
  std::vector<Vertex> lambda_vertices;
  std::vector<Punter> river_owners;

  explicit Map(const json& old_state);
  Map(const json& old_state, const json& moves);

  json to_json() {
    json_state["moves"] = moves;
    return json_state;
  }

  River get_river(const Edge& edge) const {
    return rivers[edge.river_index];
  }

  River get_river(size_t river_index) const {
    return rivers[river_index];
  }

  int64_t get_distance_from_lambda(Vertex lambda, Vertex target) const {
    assert(is_lambda[lambda]);
    return distance_from_lambda[lambda][target];
  }

  int64_t get_score_from_lambda(Vertex lambda, Vertex target) const {
    auto d = get_distance_from_lambda(lambda, target);
    if (d == -1) {
      return 0;
    }
    return d * d;
  }

  int64_t get_lambda_max_score(Vertex lambda) const;

  int64_t get_score_by_river_owners(const std::vector<Punter>& river_owners, Punter owner) const;

  size_t get_river_count() const {
    return rivers.size();
  }

  void draw_graph(const std::vector<std::pair<size_t, size_t>>& claimed_rivers);

  bool has_adjactent_owner(Vertex v, Punter owner) const;

private:
  std::vector<std::pair<size_t, size_t>> add_moves(const json& new_moves);

  void add_claim(const json& claim);

  Vertex vertex_id(Site site);
  void build_graph();
  void fill_distances();
  void fill_distances(Vertex lambda);

  std::vector<Site> sites;
  std::vector<Site> mines;
  std::vector<River> rivers;
  std::map<River, Punter> claims;
  std::vector<json> moves;
  std::vector<std::vector<int64_t>> distance_from_lambda;
  std::vector<Point> coordinates;

  json json_state;
};

