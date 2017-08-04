#pragma once
#include "map.h"
#include <cassert>

namespace impl {

class Searcher {
public:
  Searcher(const Map& map, size_t max_depth) :
      map_(map),
      max_depth_(max_depth),
      edge_used_(map_.river_owners) {}

  River solve() {
    auto result = solve_impl(map_.punter, 0);
    std::cerr << "best found score: " << result.scores[map_.punter] << std::endl;
    std::cerr << "best found move: " << result.best_move.first << " " << result.best_move.second << std::endl;
    return result.best_move;
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
    //std::cerr << "edge score: " << edge.from << " " << edge.to << " " << score << std::endl;
    return score;
  }

  Scores solve_greedy(Punter player) const {
    //std::cerr << "solve greedy" << std::endl;
    size_t remain_edges = 0;
    auto edge_used = edge_used_;
    for (const auto& owner : edge_used) {
      if (owner == kNoOwner) {
        ++remain_edges;
      } else {
        // std::cerr << "already: " << owner << std::endl;
      }
    }
    // std::cerr << "remain edges: " << remain_edges << std::endl;
    for (size_t it = 0; it < remain_edges; it++) {
      Score best_score = -1;
      size_t best_edge = 0;
      for (size_t i = 0; i < edge_used.size(); i++) {
        auto& owner = edge_used[i];
        if (owner == kNoOwner) {
          owner = player;
          Score score = map_.get_score_by_river_owners(edge_used, player);
          // std::cerr << "candidate: " << i << " " << score << std::endl;
          if (score > best_score) {
            best_score = score;
            best_edge = i;
          }
          owner = kNoOwner;
        }
      }
      // std::cerr << "choose edge: " << player << " " << best_score << std::endl;
      assert(best_score >= 0);
      edge_used[best_edge] = player;
      player = (player + 1) % map_.punters;
    }
    Scores scores(map_.punters, 0);
    for (player = 0; player < static_cast<Punter>(map_.punters); ++player) {
      scores[player] = map_.get_score_by_river_owners(edge_used, player);
    }
    return scores;
  }

  State solve_impl(Punter player, size_t depth) {
    static constexpr size_t kMaxEdgesPerNode = 3;

    /*std::cerr << "solve iteration" <<
        " player: " << player << 
        " depth: " << depth << 
        " max_depth: " << max_depth_ << std::endl;*/

    if (depth == max_depth_) {
      auto result = State{{}, solve_greedy(player)};
      /*std::cerr << "terminate state:";
      for (size_t i = 0; i < map_.punters; i++) {
        std::cerr << " " << result.scores[i];
      }
      std::cerr << std::endl;*/
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
      if (scored_edges.empty()) {
        return State{{}, solve_greedy(player)};
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
  Bruteforcer(const Map& map) : map_(map), start_time_(clock()) {}

  River solve() {
    River best_move;

    for (size_t depth = 1; !timeout(); ++depth) {
      std::cerr << "bruteforce depth: " << depth << std::endl;
      Searcher searcher(map_, depth);
      best_move = searcher.solve();
    }

    return best_move;
  }

private:
  bool timeout() const {
    auto elapsed_time = clock() - start_time_;
    return elapsed_time > CLOCKS_PER_SEC;
  }

  const Map& map_;
  clock_t start_time_;
};

}

inline River make_bruteforce_move(const Map& map) {
  impl::Bruteforcer bruteforcer(map);
  return bruteforcer.solve();
}

