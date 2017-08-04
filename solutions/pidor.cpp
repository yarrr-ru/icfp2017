#include "pidor.h"
#include "greed.h"
#include <vector>
#include <algorithm>

class Dsu {
public:
  Dsu(size_t n): color(n) {
    std::iota(color.begin(), color.end(), 0);
  }

  size_t get(size_t u) {
    if (color[u] == u) {
      return u;
    }
    return color[u] = get(color[u]);
  }

  bool join(size_t u, size_t v) {
    u = get(u);
    v = get(v);
    if (u == v) {
      return false;
    }
    color[u] = v;
    return true;
  }

private:
  std::vector<size_t> color;
};

River make_move_as_a_pidor(const Map& map) {
  for (Punter player = 0; static_cast<size_t>(player) < map.punters; ++player) {
    if (player == map.punter) {
      continue;
    }
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
    for (size_t u = 0; u < map.graph.size(); ++u) {
      for (auto edge : map.graph[u]) {
        if (edge.owner != kNoOwner) {
          continue;
        }
        size_t v = edge.to;
        if (!useful_component[dsu.get(u)] || !useful_component[dsu.get(v)]) {
          continue;
        }
        if (dsu.get(u) == dsu.get(v)) {
          continue;
        }
        std::cerr << "pidor move against player " << player << " detected" << std::endl;
        return map.get_river(edge);
      }
    }
  }
  return make_move_greed_st(map);
}