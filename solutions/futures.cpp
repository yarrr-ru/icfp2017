#include "futures.h"

River make_move_futures(const Map& map) {
  size_t u = 0, v = 0;

  int64_t dist = get_path(u, v, map, map.river_owners).first;
  if (dist == -1) {
    std::cerr << "futures failed" << std::endl;
    return River{-1, -1};
  }
  Edge edge;
  int64_t worst_dist;
  std::tie(worst_dist, edge) = get_worst_edge_all(u, v, map);
  if (worst_dist != -1 && worst_dist <= 3) {
    return River{-1, -1};
  }
  return map.get_river(edge);
}
