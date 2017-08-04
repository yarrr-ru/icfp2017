#include "greed.h"
#include <algorithm>
#include <queue>
#include <cassert>

std::pair<int64_t, std::vector<Edge>> get_path(Vertex from, Vertex to, const Map& map, const std::vector<Punter>& river_owners) {
  std::vector<Edge> path;
  if (map.get_distance_from_lambda(from, to) < 0) {
    return {-1, path};
  }

  std::deque<Vertex> q;
  std::vector<int64_t> dist(map.graph.size(), 1e9);
  dist[from] = 0;
  std::vector<Edge> go(map.graph.size());
  q.push_back(from);
  while (!q.empty()) {
    Vertex v = q.front();
    q.pop_front();
    for (auto& e : map.graph[v]) {
      int64_t w = 0;
      if (river_owners[e.river_index] == kNoOwner) {
        w = 1;
      } else if (river_owners[e.river_index] == map.punter) {
        w = 0;
      } else {
        continue;
      }
      if (dist[e.to] == -1 || dist[v] + w < dist[e.to]) {
        dist[e.to] = dist[v] + w;
        go[e.to] = e;
        if (w == 0) {
          q.push_front(e.to);
        } else {
          q.push_back(e.to);
        }
      }
    }
  }
  auto d = dist[to];
  if (d == 1e9) {
    return {-1, path};
  }

  while (to != from) {
    if (river_owners[go[to].river_index] == kNoOwner) {
      path.push_back(go[to]);
    }
    to = go[to].from;
  }
  assert(static_cast<size_t>(d) == path.size());
  return {d, path};
}

Edge get_worst_edge(Vertex from, Vertex to, const Map& map) {
  auto river_owners = map.river_owners;
  auto p = get_path(from, to, map, river_owners);
  if (p.first <= 0) {
    return {};
  }
  Edge e;
  int64_t worst_dist = 0;
  for (auto& e_to_remove : p.second) {
    river_owners[e_to_remove.river_index] = map.punters;
    auto new_dist = get_path(from, to, map, river_owners).first;
    if (new_dist == -1 || worst_dist < new_dist) {
      worst_dist = new_dist;
      e = e_to_remove;
    }
    river_owners[e_to_remove.river_index] = kNoOwner;
  }
  return e;
}

River make_move_greed_only_st(const Map& map) {
  std::vector<std::tuple<int64_t, Vertex, Vertex, std::vector<Edge>>> paths;
  auto river_owners = map.river_owners;
  for (Vertex u : map.lambda_vertices) {
    for (Vertex v : map.lambda_vertices) {
      if (u >= v) {
        continue;
      }
      auto p = get_path(u, v, map, river_owners);
      if (p.first <= 0) {
        continue;
      }
      paths.emplace_back(p.first, u, v, std::move(p.second));
    }
  }
  std::sort(paths.begin(), paths.end(),
      [](auto& a, auto& b) { return std::get<0>(a) < std::get<0>(b); }
  );

  for (auto& p : paths) {
    if (map.is_lambda[std::get<3>(p).front().from] ||
        map.is_lambda[std::get<3>(p).front().to]) {
      return map.get_river(std::get<3>(p).front());
    }
    if (map.is_lambda[std::get<3>(p).back().from] ||
        map.is_lambda[std::get<3>(p).back().to]) {
      return map.get_river(std::get<3>(p).back());
    }
  }

  if (paths.empty()) {
    return {0,0};
  }
  auto u = std::get<1>(paths.front());
  auto v = std::get<2>(paths.front());
  auto shortest = std::get<3>(paths.front());
  River r = {0, 0};
  int64_t worst_dist = 0;
  for (auto& e_to_remove : shortest) {
    river_owners[e_to_remove.river_index] = map.punters;
    auto new_dist = get_path(u, v, map, river_owners).first;
    if (new_dist == -1 || worst_dist < new_dist) {
      worst_dist = new_dist;
      r = map.get_river(e_to_remove);
    }
    river_owners[e_to_remove.river_index] = kNoOwner;
  }
  return r;
}

River make_move_greed(const Map& map) {
  auto river_owners = map.river_owners;

  int64_t max_score = -1;
  River best_river;

  for (size_t river_index = 0; river_index < river_owners.size(); ++river_index) {
    if (river_owners[river_index] != kNoOwner) { continue;
    }

    river_owners[river_index] = map.punter;
    auto score = map.get_score_by_river_owners(river_owners, map.punter);
    river_owners[river_index] = kNoOwner;
    if (score > max_score) {
      max_score = score;
      best_river = map.get_river(river_index);
    }
  }
  assert(max_score >= 0);
  return best_river;
}
