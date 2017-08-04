#pragma once
#include "map.h"
#include <cassert>
#include <vector>
#include <queue>
#include <cstdint>

namespace impl {

class Searcher {
public:
  Searcher(const Map& map, size_t max_depth) :
      map_(map),
      max_depth_(max_depth),
      river_owners_(map_.river_owners) {
      for (Vertex u = 0; u < map_.is_lambda.size(); u++) {
        if (map_.is_lambda[u]) {
          lambdas_.push_back(u);
        }
      }
    }

  River solve() {
    auto result = solve_impl(map_.punter, 0);
    auto score = result.scores[map_.punter];
    auto move = result.best_move;
    std::cerr << "best found score: " << score << std::endl;
    std::cerr << "best found move: " << move.first << " " << move.second << std::endl;
    return move;
  }

private:
  using Scores = std::vector<Score>;

  struct State {
    River best_move;
    Scores scores;

    bool better(Punter punter, const State& other) const {
      if (other.scores.empty()) {
        return true;
      }
      assert(!scores.empty());
      return scores[punter] > other.scores[punter];
    }
  };

  size_t get_best_edge(std::vector<Punter>& river_owners, Punter player) const {
    Score best_score = -1;
    size_t best_edge = 0;
    for (size_t i = 0; i < river_owners.size(); i++) {
      auto& owner = river_owners[i];
      if (owner == kNoOwner) {
        owner = player;
        Score score = map_.get_score_by_river_owners(river_owners, player);
        if (score > best_score) {
          best_score = score;
          best_edge = i;
        }
        owner = kNoOwner;
      }
    }
    assert(best_score >= 0);
    return best_edge;
  }

  Scores build_current_scores(const std::vector<Punter>& river_owners) const {
    Scores scores(map_.punters, 0);
    for (Punter player = 0; player < static_cast<Punter>(map_.punters); ++player) {
      scores[player] = map_.get_score_by_river_owners(river_owners, player);
    }
    return scores;
  }

  Punter next_player(Punter player) const {
    return (player + 1) % map_.punters;
  }

  size_t get_remain_edges(const std::vector<Punter>& river_owners) const {
    size_t remain_edges = 0;
    for (const auto& owner : river_owners) {
      if (owner == kNoOwner) {
        ++remain_edges;
      }
    }
    return remain_edges;
  }

  State solve_greedy(Punter player) const {
    auto river_owners = river_owners_;
    size_t remain_edges = get_remain_edges(river_owners);
    assert(remain_edges > 0);
    River move;
    //std::cerr << "greedy: " << remain_edges << std::endl;
    for (size_t it = 0; it < remain_edges; it++) {
      auto best_edge = get_best_edge(river_owners, player);
      //std::cerr << "it: " << it << " " << best_edge << std::endl;
      if (it == 0) {
        move = map_.get_river(best_edge);
      }
      river_owners[best_edge] = player;
      player = next_player(player);
    }
    return State{move, build_current_scores(river_owners)};
  }

  struct VertexWithDistances {
    Vertex vertex;
    std::vector<int64_t> distances;

    bool operator<(const VertexWithDistances& other) const {
      return distances < other.distances;
    }
  };

  std::vector<int64_t> build_personal_distances(Punter player, Vertex start) {
    auto vertices = map_.graph.size();
    std::vector<int64_t> distances(vertices, -1);
    distances[start] = 0;
    std::queue<Vertex> queue;
    queue.push(start);
    while (!queue.empty()) {
      auto u = queue.front();
      queue.pop();
      for (auto edge : map_.graph[u]) {
        auto v = edge.to;
        auto new_distance = distances[u];
        auto owner = river_owners_[edge.river_index];
        if (owner == kNoOwner) {
          ++new_distance;
        } else if (owner != player) {
          continue;
        }
        if (distances[v] == -1 || distances[v] > new_distance) {
          distances[v] = new_distance;
          queue.push(v);
        }
      }
    }
    return distances;
  }

  State solve_impl(Punter player, size_t depth) {

    auto remain_edges = get_remain_edges(river_owners_);
    auto remain_player_moves = static_cast<int64_t>(
      (remain_edges + map_.punters - 1) / map_.punters
    );


    //std::cerr << "solve_impl: " << player << " " << depth << " " << remain_edges << " " << remain_player_moves << std::endl;

    if (remain_edges == 0) {
      return State{{}, build_current_scores(river_owners_)};
    } else if (depth == max_depth_) {
      return solve_greedy(player);
    } else {
      //std::cerr << "go this" << std::endl;
      std::vector<VertexWithDistances> interesting_lambdas;
      for (auto lambda : lambdas_) {
        std::vector<int64_t> lambda_distances;
        auto distances = build_personal_distances(player, lambda);
        for (auto other_lambda : lambdas_) {
          auto distance = distances[other_lambda];
          if (distance > 0 && distance <= remain_player_moves) {
            lambda_distances.push_back(distance);
          }
        }
        if (!lambda_distances.empty()) {
          //std::cerr << "interesting lambda: " << lambda << std::endl;
          std::sort(lambda_distances.begin(), lambda_distances.end());
          interesting_lambdas.push_back({lambda, lambda_distances});
          /*for (auto d : lambda_distances) {
            std::cerr << d << " ";
          }
          std::cerr << std::endl;*/
        }
      }
      //std::cerr << "done" << std::endl;
      if (!interesting_lambdas.empty()) {
        //std::cerr << "not empty" << std::endl;
        std::random_shuffle(interesting_lambdas.begin(), interesting_lambdas.end());
        std::stable_sort(interesting_lambdas.begin(), interesting_lambdas.end());
        auto interesting_lambda = interesting_lambdas.front();
        std::vector<VertexWithDistances> interesting_edges;
        auto old_distances = build_personal_distances(player, interesting_lambda.vertex);

        for (size_t edge = 0; edge < river_owners_.size(); edge++) {
          auto& owner = river_owners_[edge];
          if (owner == kNoOwner) {
            owner = player;
            auto new_distances = build_personal_distances(player, interesting_lambda.vertex);
            std::vector<int64_t> new_lambda_distances;
            for (auto other_lambda : lambdas_) {
              auto old_distance = old_distances[other_lambda];
              auto new_distance = new_distances[other_lambda];
              if (old_distance > 0 && new_distance >= 0 && new_distance < remain_player_moves) {
                new_lambda_distances.push_back(new_distance);
              }
            }
            if (!new_lambda_distances.empty()) {
              /*std::cerr << "candidate: " << edge << " " << interesting_lambda.vertex << std::endl;
              for (auto d : new_lambda_distances) {
                std::cerr << d << " ";
              }
              std::cerr << std::endl;*/

              std::sort(new_lambda_distances.begin(), new_lambda_distances.end());
              interesting_edges.push_back({edge, new_lambda_distances});
            }
            owner = kNoOwner;
          }
        }

        if (interesting_edges.empty()) {
          return solve_greedy(player);
        }

        std::random_shuffle(interesting_edges.begin(), interesting_edges.end());
        std::stable_sort(interesting_edges.begin(), interesting_edges.end());

        static constexpr size_t kBruteforceWidth = 3;
        if (interesting_edges.size() > kBruteforceWidth) {
          interesting_edges.resize(kBruteforceWidth);
        }

        State best_state;

        for (auto interesting_edge : interesting_edges) {
          //std::cerr << "watch: " << interesting_edge.vertex << std::endl;
          auto edge = interesting_edge.vertex;
          auto& owner = river_owners_[edge];
          assert(owner == kNoOwner);
          owner = player;
          State state{map_.get_river(edge), solve_impl(next_player(player), depth + 1).scores};
          if (state.better(player, best_state)) {
            best_state = state;
          }
          owner = kNoOwner;
        }

        return best_state;
      } else {
        //std::cerr << "go gready" << std::endl;
        return solve_greedy(player);
      }
    }
  }

  const Map& map_;
  const size_t max_depth_;
  std::vector<Punter> river_owners_;
  std::vector<Vertex> lambdas_;
};

class Bruteforcer {
public:
  Bruteforcer(const Map& map) : map_(map), start_time_(clock()) {}

  River solve() {
    srand(time(0));

    River best_move;

    for (size_t depth = 1; !timeout() && depth <= 1; ++depth) {
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

