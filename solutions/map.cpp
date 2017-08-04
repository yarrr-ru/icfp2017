#include "map.h"
#include <algorithm>

Map::Map(const json& old_state) :
    punter(old_state["punter"]),
    punters(old_state["punters"]),
    json_state(old_state) {
  assert(punters > 1);
  assert(static_cast<size_t>(punter) < punters);
  add_moves(json_state["moves"]);
  for (auto& site : json_state["map"]["sites"]) {
    sites.push_back(site["id"]);
  }
  std::sort(sites.begin(), sites.end());
  for (auto& river : json_state["map"]["rivers"]) {
    rivers.push_back(make_river(river));
  }
  for (auto& mine : json_state["map"]["mines"]) {
    mines.push_back(mine);
  }
  assert(mines.size() > 0);
  assert(sites.size() >= mines.size());
  assert(rivers.size() >= sites.size());
  std::cerr << "map punter: " << punter <<
      " punters: " << punters <<
      " mines: " << mines.size() <<
      " sites: " << sites.size() <<
      " rivers: " << rivers.size() <<
      " claims: " << claims.size() <<
      " moves: " << moves.size() <<
      std::endl;
  build_graph();
}

Map::Map(const json& old_state, const json& moves) : Map(old_state) {
  add_moves(moves);
  clear_graph();
  build_graph();
}

size_t Map::vertex_id(int32_t size) {
  return std::lower_bound(sites.begin(), sites.end(), size) - sites.begin();
}

void Map::clear_graph() {
  graph.clear();
  is_lambda.clear();
}

void Map::build_graph() {
  graph.resize(sites.size());
  for (size_t i = 0; i < rivers.size(); ++i) {
    size_t u = vertex_id(rivers[i].first);
    size_t v = vertex_id(rivers[i].second);
    auto iterator = claims.find({u, v});
    Punter owner = (iterator != claims.end()) ? iterator->second : kNoOwner;
    graph[u].emplace_back(u, v, owner, i);
    graph[v].emplace_back(v, u, owner, i);
    // std::cerr << "add edge: " << u << " " << v << " owner: " << owner << std::endl;
  }
  is_lambda.assign(sites.size(), false);
  for (auto site : mines) {
    is_lambda[vertex_id(site)] = true;
  }
}

void Map::add_claim(const json& claim) {
  River r = make_river(claim);
  Punter p = claim["punter"];
  assert(static_cast<size_t>(p) < punters);
  bool added = claims.emplace(r, p).second;
  assert(added);
  std::swap(r.first, r.second);
  added = claims.emplace(r, p).second;
  assert(added);
}

void Map::add_moves(const json& new_moves) {
  for (auto& move: new_moves) {
    if (move.count("claim")) {
      add_claim(move["claim"]);
    } else {
      assert(move.count("pass"));
    }
    moves.push_back(move);
  }
}

