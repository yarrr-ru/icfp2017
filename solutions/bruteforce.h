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
    edge_used_.assign(max_river_index + 1, kNoOwner);
    for (const auto& edges : map_.graph) {
      for (const auto& edge : edges) {
        edge_used_[edge.river_index] = edge.owner;
      }
    }
    std::cerr << "max_river_index: " << max_river_index << std::endl;

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
    std::cerr << "edge score: " << edge.from << " " << edge.to << " " << score << std::endl;
    return score;
  }

  Score evalutate(Punter) const {
    return 0;
  }

  Scores solve_greedy(Punter player) const {
    size_t remain_edges = 0;
    auto edge_used = edge_used_;
    for (const auto& owner : edge_used) {
      if (owner == kNoOwner) {
        ++remain_edges;
      }
    }
    for (size_t it = 0; it < remain_edges; it++) {
      Score best_score = -1;
      size_t best_edge = 0;
      for (size_t i = 0; i < edge_used.size(); i++) {
        auto& owner = edge_used[i];
        if (owner == kNoOwner) {
          owner = player;
          Score score = evalutate(player);
          if (score > best_score) {
            best_score = score;
            best_edge = i;
          }
          owner = kNoOwner;
        }
      }
      assert(best_score >= 0);
      edge_used[best_edge] = player;
      player = (player + 1) % map_.punters;
    }
    Scores scores(0, map_.punters);
    for (player = 0; player < static_cast<Punter>(map_.punters); ++player) {
      scores[player] = evalutate(player);
    }
    return scores;
  }

  State solve_impl(Punter player, size_t depth) {
    static constexpr size_t kMaxEdgesPerNode = 5;

    std::cerr << "solve iteration" <<
        " player: " << player << 
        " depth: " << depth << 
        " max_depth: " << max_depth_ << std::endl;

    if (depth == max_depth_) {
      auto result = State{{}, solve_greedy(player)};
      std::cerr << "terminate state:";
      for (size_t i = 0; i < map_.punters; i++) {
        std::cerr << " " << result.scores[i];
      }
      std::cerr << std::endl;
      return result;
    } else {
      std::vector<EdgeWithScore> scored_edges;
      for (const auto& edges : map_.graph) {
        for (const auto& edge : edges) {
          bool unused_edge = edge_used_[edge.river_index] == kNoOwner;
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
        edge_used_[edge.river_index] = player;
        size_t next_player = (player + 1) % map_.punters;
        auto state = solve_impl(next_player, depth + 1);
        if (state.better(player, result)) {
          //  std::cout << "better, update" << std::endl;
          result.best_move = map_.get_river(edge);
          result.scores = state.scores;
        }
        edge_used_[edge.river_index] = kNoOwner;
      }
      
      return result;
    }
  }

  const Map& map_;
  const size_t max_depth_;
  std::vector<Punter> edge_used_;
};

class Bruteforcer {
public:
  Bruteforcer(const Map& map) : map_(map) {}

  River solve() {
    River best_move;

    for (size_t depth = 1; !timeout(depth); ++depth) {
      std::cerr << "bruteforce depth: " << depth << std::endl;
      Searcher searcher(map_, depth);
      best_move = searcher.solve();
    }

    return best_move;
  }

private:
  bool timeout(size_t depth) const {
    return depth > map_.punters;
  }

  const Map& map_;
};

}

inline River make_bruteforce_move(const Map& map) {
  impl::Bruteforcer bruteforcer(map);
  return bruteforcer.solve();
}

