#include "futures.h"

River make_move_futures(const Map& map) {
  if (map.futures.empty()) {
    return River{-1, -1};
  }
  for (size_t i = 0; i < map.futures.size(); ++i) {
    size_t u = map.futures[i].first;
    size_t v = map.futures[i].second;

    int64_t dist = get_path(u, v, map, map.river_owners).first;
    if (dist == -1) {
      std::cerr << "futures " << i << " failed" << std::endl;
      continue;
    }
    Edge edge;
    int64_t worst_dist;
    std::tie(worst_dist, edge) = get_worst_edge_all(u, v, map);
    if (worst_dist != -1 && worst_dist <= 3) {
      std::cerr << "futures " << i << " ne gorit" << std::endl;
      continue;
    }
    return map.get_river(edge);
  }
  return River{-1, -1};
}
