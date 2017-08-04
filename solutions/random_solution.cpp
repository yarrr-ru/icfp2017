#include "launcher.h"
#include <algorithm>

River make_move_random(const Map& map) {
  std::vector<bool> connected(map.graph.size(), false);
  for (Vertex v = 0; v < map.graph.size(); ++v) {
    connected[v] = map.is_lambda[v];
    for (auto& e : map.graph[v]) {
      if (e.owner == map.punter) {
        connected[v] = true;
      }
    }
  }

  std::vector<size_t> connected_rivers;
  std::vector<size_t> empty_rivers;
  for (auto& a : map.graph) {
    for (auto& e : a) {
      if (e.owner != kNoOwner) {
        continue;
      }
      if (connected[e.from] && !connected[e.to]) {
        connected_rivers.push_back(e.river_index);
      } else {
        empty_rivers.push_back(e.river_index);
      }
    }
  }
  std::cerr << "connected_rivers: " << connected_rivers.empty() <<
      " empty_rivers: " << empty_rivers.empty() <<
      std::endl;
  if (connected_rivers.empty()) {
    assert(!empty_rivers.empty());
    return map.get_river(empty_rivers[rand() % empty_rivers.size()]);
  }
  return map.get_river(connected_rivers[rand() % connected_rivers.size()]);
}

int main() {
  return main_launcher(make_move_random);
}
