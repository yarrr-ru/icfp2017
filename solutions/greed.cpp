#include "greed.h"
#include <algorithm>
#include <queue>
#include <cassert>

using Paths = std::vector<std::tuple<int64_t, Vertex, Vertex, Path>>;

std::pair<int64_t, Path> get_path(Vertex from, Vertex to, const Map& map, const std::vector<Punter>& river_owners) {
  Path path;
  if (map.is_lambda[from] && map.get_distance_from_lambda(from, to) < 0) {
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

Paths make_paths_between_lambdas(const Map& map) {
  Paths paths;
  for (Vertex u : map.lambda_vertices) {
    for (Vertex v : map.lambda_vertices) {
      if (u >= v) {
        continue;
      }
      auto p = get_path(u, v, map, map.river_owners);
      if (p.first <= 0) {
        continue;
      }
      paths.emplace_back(p.first, u, v, std::move(p.second));
    }
  }
  std::sort(paths.begin(), paths.end(),
      [](auto& a, auto& b) { return std::get<0>(a) < std::get<0>(b); }
  );
  return paths;
}

River make_move_surround_lamdas_with_paths(const Map& map, const Paths& paths) {
  std::map<River, size_t> rating;
  for (auto& p : paths) {
    if (map.is_lambda[std::get<3>(p).front().from] ||
        map.is_lambda[std::get<3>(p).front().to]) {
      rating[map.get_river(std::get<3>(p).front())]++;
    }
    if (map.is_lambda[std::get<3>(p).back().from] ||
        map.is_lambda[std::get<3>(p).back().to]) {
      rating[map.get_river(std::get<3>(p).front())]++;
    }
  }
  River r = {0, 0};
  size_t max_r = 0;
  for (auto& p : rating) {
    if (p.second > max_r) {
      max_r = p.second;
      r = p.first;
    }
  }
  return r;
}

River make_move_surround_all_lamdas(const Map& map) {
  auto r = make_move_surround_lamdas_with_paths(map, make_paths_between_lambdas(map));
  if (r.first != r.second) {
    return r;
  }
  std::vector<std::pair<size_t, River>> free_lambdas;
  for (Vertex lambda : map.lambda_vertices) {
    size_t empty = 0;
    River one;
    for (auto& e : map.graph[lambda]) {
      if (e.owner == kNoOwner) {
        empty++;
        one = map.get_river(e);
      }
    }
    if (empty != 0) {
      free_lambdas.emplace_back(empty, one);
    }
  }

  if (!free_lambdas.empty()) {
    std::sort(free_lambdas.begin(), free_lambdas.end());
    return free_lambdas.front().second;
  }
  return {0, 0};
}

River make_move_greed_only_st(const Map& map) {
  auto paths = make_paths_between_lambdas(map);
  auto river_owners = map.river_owners;
  if (paths.empty()) {
    return {0,0};
  }

  River r = make_move_surround_lamdas_with_paths(map, paths);
  if (r.first != r.second) {
    return r;
  }

  auto u = std::get<1>(paths.front());
  auto v = std::get<2>(paths.front());
  auto shortest = std::get<3>(paths.front());
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

River make_move_scored_only_st(const Map& map) {
  auto paths = make_paths_between_lambdas(map);
  auto river_owners = map.river_owners;
  if (paths.empty()) {
    return {0,0};
  }

  River r = make_move_surround_lamdas_with_paths(map, paths);
  if (r.first != r.second) {
    return r;
  }

  std::map<River, double> river_scores;

  for (auto& p : paths) {
    auto u = std::get<1>(p);
    auto v = std::get<2>(p);
    auto path = std::get<3>(p);
    for (auto& e_to_remove : path) {
      river_owners[e_to_remove.river_index] = map.punters;
      auto new_dist = get_path(u, v, map, river_owners).first;
      river_owners[e_to_remove.river_index] = kNoOwner;
      if (new_dist == -1) {
        new_dist = (path.size() * path.size() + river_owners.size()) / 2;
      }

      r = map.get_river(e_to_remove);
      double score = static_cast<double>(new_dist);
      if (map.has_adjactent_owner(e_to_remove.from, map.punter)) {
        score++;
      }
      if (map.has_adjactent_owner(e_to_remove.to, map.punter)) {
        score++;
      }
      score /= path.size();
      river_scores[r] += score;
    }
  }

  r = {0, 0};
  double max_score = 0;
  for (auto& p : river_scores) {
    if (p.second > max_score) {
      max_score = p.second;
      r = p.first;
    }
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

River make_move_greed_st(const Map& map) {
  River r = make_move_greed_only_st(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_greed(map);
}

River make_move_scored_st(const Map& map) {
  River r = make_move_scored_only_st(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_greed(map);
}

River make_move_scored_surround_st(const Map& map) {
  River r = make_move_surround_all_lamdas(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_scored_st(map);
}

River make_move_greed_surround_st(const Map& map) {
  River r = make_move_surround_all_lamdas(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_greed_st(map);
}

Futures make_futures_random(const Map& map) {
  Futures futures;
  auto n = map.is_lambda.size();
  for (Vertex u = 0; u < n; u++) {
    if (map.is_lambda[u]) {
      for (Vertex v = 0; v < n; v++) {
        if (!map.is_lambda[v]) {
          futures.push_back(map.get_future(u, v));
          break;
        }
      }
    }
  }
  return futures;
}

