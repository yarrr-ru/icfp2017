#pragma once
#include "map.h"
#include <cassert>

namespace impl {

class Searcher {
public:
  Searcher(const Map& map, size_t max_depth) : map_(map), max_depth_(max_depth) {}

  River solve() {
    size_t max_river_index = 0;
    for (const auto& edges : map_.graph) {
      for (const auto& edge : edges) {
        max_river_index = std::max(max_river_index, edge.river_index);
      }
    }
    edge_used_.assign(max_river_index + 1, false);
    std::cout << "max_river_index: " << max_river_index << std::endl;

    return solve_impl(map_.punter, 0).best_move;
  }

private:
  using Scores = std::vector<Score>;

  struct State {
    River best_move;
    Scores scores;

    bool better(size_t player, const State& other) const {
      if (other.scores.empty())
        return true;
      assert(!scores.empty());
      return scores[player] > other.scores[player];
    }
  };

  struct EdgeWithScore {
    Edge edge;
    double score;
  };

  double score_edge(const Edge& edge) {
    bool is_lambda = map_.is_lambda[edge.from] || map_.is_lambda[edge.to];
    double score = is_lambda ? 1 : 0;
    std::cout << "edge score: " << edge.from << " " << edge.to << " " << score << std::endl;
    return score;
  }

  Scores solve_greedy(size_t) {
    return Scores(0, map_.punters);
  }

  State solve_impl(size_t player, size_t depth) {
    static constexpr size_t kMaxEdgesPerNode = 3;

    std::cout << "solve iteration" <<
        " player: " << player << 
        " depth: " << depth << 
        " max_depth: " << max_depth_ << std::endl;

    if (depth == max_depth_) {
      return State{{}, solve_greedy(player)};
    } else {
      std::vector<EdgeWithScore> scored_edges;
      for (const auto& edges : map_.graph) {
        for (const auto& edge : edges) {
          bool unused_edge = edge.owner == kNoOwner && !edge_used_[edge.river_index];
          if (unused_edge) {
            double score = score_edge(edge);
            scored_edges.push_back(EdgeWithScore{edge, score});
          }
        }
      }
      std::sort(scored_edges.begin(), scored_edges.end(), [](auto lhs, auto rhs) {
        return lhs.score > rhs.score;
      });
      scored_edges.resize(std::min(scored_edges.size(), kMaxEdgesPerNode));
      State result;
      for (const auto& edge_with_score : scored_edges) {
        const auto& edge = edge_with_score.edge;
        //  std::cout << "try get edge: " << edge.from << " " << edge.to << std::endl;
        edge_used_[edge.river_index] = true;
        size_t next_player = (player + 1) % map_.punters;
        auto state = solve_impl(next_player, depth + 1);
        if (state.better(player, result)) {
          //  std::cout << "better, update" << std::endl;
          result.best_move = map_.get_river(edge);
          result.scores = state.scores;
        }
        edge_used_[edge.river_index] = false;
      }
      
      return result;
    }
  }

  const Map& map_;
  const size_t max_depth_;
  std::vector<bool> edge_used_;
};

class Bruteforcer {
public:
  Bruteforcer(const Map& map) : map_(map) {}

  River solve() {
    River best_move;

    for (size_t depth = 1; !timeout(depth); ++depth) {
      std::cout << "bruteforce depth: " << depth << std::endl;
      Searcher searcher(map_, depth);
      best_move = searcher.solve();
    }

    return best_move;
  }

private:
  bool timeout(size_t depth) const {
    return depth >= 2;
  }

  const Map& map_;
};

}

inline River make_bruteforce_move(const Map& map) {
  impl::Bruteforcer bruteforcer(map);
  return bruteforcer.solve();
}

