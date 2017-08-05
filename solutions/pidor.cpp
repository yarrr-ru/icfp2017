#include "pidor.h"
#include "greed.h"
#include "dsu.h"

auto get_top_player_scores(const Map& map) {
  std::vector<std::pair<int64_t, Punter>> players;
  for (Punter player = 0; static_cast<size_t>(player) < map.punters; ++player) {
    players.emplace_back(map.get_score_by_river_owners(map.river_owners, player), player);
  }
  std::sort(players.begin(), players.end());
  std::reverse(players.begin(), players.end());
  return players;
}

River make_move_as_an_endspiel_pidor(const Map& map) {
  auto players = get_top_player_scores(map);
  std::map<River, double> river_scores;

  int64_t our_score = 0;
  for (auto pair : players) {
    if (pair.second == map.punter) {
      our_score = pair.first;
    }
  }

  for (auto pair : players) {
    Punter player = pair.second;
    auto moves = get_greed_moves(map, player);
    if (moves.empty()) {
      continue;
    }
    if (player == map.punter) {
      double score = moves.front().first * std::max(2ul, map.punters / 3);
      river_scores[moves.front().second] += score;
      continue;
    }

    double score = moves.front().first;
    if (moves.size() > 1) {
      score -= moves[1].first;
    }
    score *= std::max(2.0, score / std::abs(our_score - pair.first));
    river_scores[moves.front().second] += score;
  }

  River r = {0, 0};
  int64_t max_score = 0;
  for (auto& p : river_scores) {
    if (p.second > max_score) {
      max_score = p.second;
      r = p.first;
    }
  }
  return r;
}

River make_move_as_a_pidor_only(const Map& map) {
  auto players = get_top_player_scores(map);
  for (auto pair : players) {
    Punter player = pair.second;
    if (player == map.punter) {
      continue;
    }
    Dsu dsu(map.graph.size());
    for (size_t u = 0; u < map.graph.size(); ++u) {
      for (auto edge : map.graph[u]) {
        if (edge.owner != player) {
          continue;
        }
        dsu.join(edge.from, edge.to);
      }
    }
    std::vector<char> useful_component(map.graph.size());
    for (size_t u = 0; u < map.graph.size(); ++u) {
      if (map.is_lambda[u]) {
        useful_component[dsu.get(u)] = true;
      }
    }
    std::map<std::pair<size_t, size_t>, std::vector<Edge>> connect_ways;
    for (size_t u = 0; u < map.graph.size(); ++u) {
      for (auto edge : map.graph[u]) {
        if (edge.owner != kNoOwner) {
          continue;
        }
        size_t v = edge.to;
        size_t comp1 = dsu.get(u);
        size_t comp2 = dsu.get(v);
        if (!useful_component[comp1] || !useful_component[comp2]) {
          continue;
        }
        if (comp1 >= comp2) {
          continue;
        }
        connect_ways[std::make_pair(comp1, comp2)].push_back(edge);
      }
    }
    for (auto way : connect_ways) {
      if (way.second.size() > 1) {
        continue;
      }
      auto edge = way.second[0];
      std::cerr << "pidor move against player " << player << " detected" << std::endl;
      return map.get_river(edge);
    }
  }
  return {0,0};
}

River make_move_as_a_pidor(const Map& map) {
  River r = make_move_as_a_pidor_only(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_greed_st(map);
}

River make_move_surround_pidor_endspiel(const Map& map) {
  River r = make_move_surround_all_lamdas(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_as_an_endspiel_pidor(map);
}

River make_move_scored_pidor_endspiel(const Map& map) {
  River r = make_move_scored_only_st(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_as_an_endspiel_pidor(map);
}

River make_move_greed_st_pidor_endspiel(const Map& map) {
  River r = make_move_greed_only_st(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_as_an_endspiel_pidor(map);
}

