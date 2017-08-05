#include "greed.h"
#include "wise.h"
#include "pidor.h"
#include <algorithm>
#include <queue>
#include <cassert>

namespace {

std::vector<double> get_probabilities(const Map& map) {
  auto river_owners = map.river_owners;
  size_t players = map.punters;
  size_t remain_rivers = 0;
  for (auto owner : river_owners) {
    if (owner == kNoOwner) {
      ++remain_rivers;
    }
  }
  size_t remain_moves = (remain_rivers + players - 1) / players;

  std::vector<double> probabilities(remain_moves + 1, 1.0);
  for (size_t move = 1; move <= remain_moves; move++) {
    double probability = 1.0;
    for (size_t i = 0; i < players - 1; i++) {
      if (remain_rivers > 0) {
       probability *= double(remain_rivers - 1) / double(remain_rivers);
      }
      --remain_rivers;
    }
    --remain_rivers;
    probabilities[move] = (probabilities[move - 1] * probability);
  }

  return probabilities;
}

}

std::vector<int64_t> get_distances(Vertex from, const Map& map, const std::vector<Punter>& river_owners) {
  size_t vertices = map.graph.size();
  std::vector<int64_t> distances(vertices, -1);
  distances[from] = 0;
  std::deque<Vertex> deque;
  deque.push_front(from);
  //std::cerr << "distances: " << from << std::endl;
  while (!deque.empty()) {
    auto u = deque.front();
    deque.pop_front();
    for (auto edge : map.graph[u]) {
      auto owner = river_owners[edge.river_index];
      auto v = edge.to;
      if (owner == map.punter) {
        auto new_distance = distances[u];
        if (new_distance < distances[v] || distances[v] == -1) {
          //std::cerr << "relax self: " << u << " " << v << " " << new_distance << std::endl;
          distances[v] = new_distance;
          deque.push_front(v);
        }
      } else if (owner == kNoOwner) {
        auto new_distance = distances[u] + 1;
        if (new_distance < distances[v] || distances[v] == -1) {
          //std::cerr << "relax empty: " << u << " " << v << " " << new_distance << std::endl;
          distances[v] = new_distance;
          deque.push_back(v);
        }
      }
    }
  }
  return distances;
}

River make_move_wise(const Map& map) {
  auto river_owners = map.river_owners;
  auto probabilities = get_probabilities(map);

  auto evaluate = [&map, &probabilities](Vertex lambda,
    const std::vector<int64_t>& old_distances,
    const std::vector<int64_t>& distances
  ) {
    //std::cerr << "evaluate: " << lambda << std::endl;
    double score = 0;
    for (Vertex i = 0; i < distances.size(); i++) {
      auto old_distance = old_distances[i];
      auto distance = distances[i];
      if (old_distance != distance &&
          distance >= 0 &&
          old_distance < static_cast<int64_t>(probabilities.size())
      ) {
        assert(old_distance > distance);
        //std::cerr << i << " " << old_distance << " " << distance << " ";
        //std::cerr << probabilities[distance] << " " << map.get_score_from_lambda(lambda, i);
        score += probabilities[distance] * map.get_score_from_lambda(lambda, i);
        score -= probabilities[old_distance] * map.get_score_from_lambda(lambda, i);
        //std::cerr << std::endl;
      }
    }
    return score;
  };

  std::vector<std::vector<int64_t>> old_distances;

  //std::cerr << "think wise" << std::endl;

  for (auto lambda : map.lambda_vertices) {
    auto distances = get_distances(lambda, map, river_owners);
    old_distances.push_back(distances);
  }

  constexpr double kEps = 1e-9;
  double best_score = kEps;
  River best_river = {0, 0};

  for (size_t river_id = 0; river_id < river_owners.size(); river_id++) {
    auto& owner = river_owners[river_id];
    if (owner == kNoOwner) {
      owner = map.punter;

      double edge_score = 0.0;

      for (size_t lambda_id = 0; lambda_id < map.lambda_vertices.size(); lambda_id++) {
        auto lambda = map.lambda_vertices[lambda_id];
        auto new_distances = get_distances(lambda, map, river_owners);
        edge_score += evaluate(lambda, old_distances[lambda_id], new_distances);
      }

      if (edge_score > best_score) {
        best_score = edge_score;
        best_river = map.get_river(river_id);
      }

      //auto river = map.get_river(river_id);
      //std::cerr << "score: " << river.first << " " << river.second << " " << edge_score << std::endl;

      owner = kNoOwner;
    }
  }

  return best_river;
}

River make_move_wise_st(const Map& map) {
  River r = make_move_scored_only_st(map);
  if (r != River{0, 0}) {
    return r;
  }
  r = make_move_wise(map);
  if (r != River{0, 0}) {
    return r;
  }
  return make_move_as_an_endspiel_pidor(map);
}

