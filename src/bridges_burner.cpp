#include "bridges_burner.h"
#include "greed.h"
#include "greed.h"

static bool reachable_lambda(const Map& map, std::vector<char>& reached, size_t banned_index, size_t u) {
  reached[u] = true;
  if (map.is_lambda[u]) {
    return true;
  }
  for (auto edge : map.graph[u]) {
    if (edge.river_index == banned_index || edge.owner != kNoOwner) {
      continue;
    }
    size_t v = edge.to;
    if (!reached[v]) {
      if (reachable_lambda(map, reached, banned_index, v)) {
        return true;
      }
    }
  }
  return false;
}

River make_move_burn_bridge(const Map& map) {
  std::vector<Punter> fake_owners = map.river_owners;
  for (Punter& punter : fake_owners) {
    if (punter != kNoOwner) {
      punter = map.punters;
    }
  }

  int64_t best_score = -1;
  River best_river;
  std::vector<char> reached;
  for (size_t u = 0; u < map.graph.size(); ++u) {
    for (auto edge : map.graph[u]) {
      if (edge.owner != kNoOwner) {
        continue;
      }
      size_t v = edge.to;
      reached.assign(map.graph.size(), false);
      if (!reachable_lambda(map, reached, edge.river_index, u)) {
        continue;
      }
      reached.assign(map.graph.size(), false);
      if (!reachable_lambda(map, reached, edge.river_index, v)) {
        continue;
      }
      fake_owners[edge.river_index] = map.punters;

      int64_t length = get_path(u, v, map, fake_owners).first;
      if (length == -1) {
        length = 1e9;
      }
      if (length > best_score) {
        best_score = length;
        best_river = map.get_river(edge);
      }

      fake_owners[edge.river_index] = kNoOwner;
    }
  }
  if (best_score == -1) {
    return make_move_greed(map);
  }
  return best_river;
}
