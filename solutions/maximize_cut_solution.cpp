#include "launcher.h"
#include "greed.h"
#include <algorithm>
#include <vector>

enum VertexType {
  UNREACHABLE,
  REACHABLE,
  BORDER
};

bool reserved_edge(const Map& map, Edge edge) {
  return edge.owner != map.punter && edge.owner != kNoOwner;
}

static void mark_reachable(const Map& map, std::vector<VertexType> &vertex_type, size_t u) {
  vertex_type[u] = REACHABLE;
  for (auto edge : map.graph[u]) {
    if (reserved_edge(map, edge)) {
      continue;
    }
    size_t v = edge.to;
    if (vertex_type[v] != UNREACHABLE) {
      continue;
    }
    vertex_type[v] = BORDER;
    if (edge.owner == map.punter) {
      mark_reachable(map, vertex_type, v);
    }
  }
}

River make_move_maximizing_cut(const Map& map) {
  std::vector<VertexType> vertex_type(map.graph.size(), UNREACHABLE);
  for (size_t u = 0; u < map.graph.size(); ++u) {
    if (map.is_lambda[u]) {
      mark_reachable(map, vertex_type, u);
    }
  }

  int32_t max_score = -1;
  River best_river;

  for (size_t u = 0; u < map.graph.size(); ++u) {
    if (vertex_type[u] != REACHABLE) {
      continue;
    }
    for (auto edge : map.graph[u]) {
      if (edge.owner != kNoOwner) {
        continue;
      }
      size_t v = edge.to;
      if (vertex_type[v] == REACHABLE) {
        continue;
      }
      assert(vertex_type[v] == BORDER);
      int32_t score = 0;
      for (auto new_edge : map.graph[v]) {
        if (edge.owner != kNoOwner) {
          continue;
        }
        size_t w = new_edge.to;
        if (vertex_type[w] == UNREACHABLE) {
          ++score;
        }
      }
      if (score > max_score) {
        max_score = score;
        best_river = map.get_river(edge);
      }
    }
  }
  std::cerr << "best score " << max_score << std::endl;
  if (max_score <= 1) {
    return make_move_greed_st(map);
  }

  return best_river;
}

int main() {
  return main_launcher(make_move_maximizing_cut);
}
