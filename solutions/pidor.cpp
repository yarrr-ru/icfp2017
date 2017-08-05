#include "pidor.h"
#include "greed.h"
#include "dsu.h"

River make_move_as_a_pidor(const Map& map) {
  std::vector<std::pair<int64_t, Punter>> players;
  for (Punter player = 0; static_cast<size_t>(player) < map.punters; ++player) {
    if (player == map.punter) {
      continue;
    }
    players.emplace_back(map.get_score_by_river_owners(map.river_owners, player), player);
  }
  std::sort(players.begin(), players.end());
  std::reverse(players.begin(), players.end());
  for (auto pair : players) {
    Punter player = pair.second;
    Dsu dsu(map.graph.size());
    for (size_t u = 0; u < map.graph.size(); ++u) {
      for (auto edge : map.graph[u]) {
        if (edge.owner != player) {
          continue;
        }
        dsu.join(edge.from, edge.to);
      }
    }
    std::vector<char> useful_component(map.graph.size());
    for (size_t u = 0; u < map.graph.size(); ++u) {
      if (map.is_lambda[u]) {
        useful_component[dsu.get(u)] = true;
      }
    }
    std::map<std::pair<size_t, size_t>, std::vector<Edge>> connect_ways;
    for (size_t u = 0; u < map.graph.size(); ++u) {
      for (auto edge : map.graph[u]) {
        if (edge.owner != kNoOwner) {
          continue;
        }
        size_t v = edge.to;
        size_t comp1 = dsu.get(u);
        size_t comp2 = dsu.get(v);
        if (!useful_component[comp1] || !useful_component[comp2]) {
          continue;
        }
        if (comp1 >= comp2) {
          continue;
        }
        connect_ways[std::make_pair(comp1, comp2)].push_back(edge);
      }
    }
    for (auto way : connect_ways) {
      if (way.second.size() > 1) {
        continue;
      }
      auto edge = way.second[0];
      std::cerr << "pidor move against player " << player << " detected" << std::endl;
      return map.get_river(edge);
    }
  }
  return make_move_greed_st(map);
}
