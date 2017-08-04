#include "launcher.h"
#include <algorithm>

River make_move_random(const Map& map) {
  std::vector<Vertex> vertices(map.graph.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    vertices[i] = i;
  }
  std::random_shuffle(vertices.begin(), vertices.end());
  for (auto v : vertices) {
    for (auto& e : map.graph[v]) {
      if (e.owner == kNoOwner) {
        return map.get_river(e);
      }
    }
  }
  assert(false);
}

int main() {
  return main_launcher(make_move_random);
}
