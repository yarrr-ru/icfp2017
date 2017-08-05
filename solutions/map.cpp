#include "map.h"
#include <algorithm>
#include <queue>

Map::Map(const json& old_state) :
    punter(old_state["punter"]),
    punters(old_state["punters"]),
    json_state(old_state) {
  assert(punters > 1);
  assert(static_cast<size_t>(punter) < punters);
  add_moves(json_state["moves"]);
  for (auto& site : json_state["map"]["sites"]) {
    sites.push_back(site["id"]);
  }
  std::sort(sites.begin(), sites.end());

  coordinates.resize(sites.size());
  for (auto& site : json_state["map"]["sites"]) {
    if (site.count("x") && site.count("y")) {
      size_t index = vertex_id(site["id"]);
      if (site.count("x") && site.count("y")) {
        coordinates[index] = Point(site["x"], site["y"]);
      } else {
        coordinates[index] = Point(0, 0);
      }
    }
  }

  for (auto& river : json_state["map"]["rivers"]) {
    rivers.push_back(make_river(river));
  }
  for (auto& mine : json_state["map"]["mines"]) {
    mines.push_back(mine);
  }
  assert(mines.size() > 0);
  assert(sites.size() >= mines.size());
  assert(rivers.size() >= sites.size());
}

Map::Map(const json& old_state, const json& moves) : Map(old_state) {
  const auto claimed_rivers = add_moves(moves);
  build_graph();
  draw_graph(claimed_rivers);
  fill_distances();
  std::cerr << "map punter: " << punter <<
      " punters: " << punters <<
      " mines: " << mines.size() <<
      " sites: " << sites.size() <<
      " rivers: " << rivers.size() <<
      " claims: " << claims.size() <<
      " moves: " << moves.size() <<
      std::endl;
}

int64_t Map::get_lambda_max_score(Vertex lambda) const {
  int64_t score = 0;
  for (Vertex to = 0; to < graph.size(); ++to) {
    score += get_score_from_lambda(lambda, to);
  }
  return score;
}


int64_t Map::get_score_by_river_owners(const std::vector<Punter>& river_owners, Punter owner) const {
  assert(river_owners.size() == rivers.size());
  std::vector<char> reached;
  std::queue<Vertex> q;
  int64_t score = 0;
  for (Vertex lambda : lambda_vertices) {
    reached.assign(graph.size(), false);
    q.push(lambda);
    while (!q.empty()) {
      Vertex v = q.front();
      q.pop();
      for (auto& e : graph[v]) {
        if (!reached[e.to] && river_owners[e.river_index] == owner) {
          score += get_score_from_lambda(lambda, e.to);
          reached[e.to] = true;
          q.push(e.to);
        }
      }
    }
  }
  return score;
}

void Map::draw_graph(const std::vector<std::pair<size_t, size_t>>& claimed_rivers) {
  auto return_code = system("mkdir -p svg");
  if (return_code != 0) {
    std::cerr << "unable to mkdir svg, return code: " << return_code;
    return;
  }

  std::string dir = std::string("svg/") + std::to_string(punter);
  Drawer drawer(dir);
  for (size_t i = 0; i < sites.size(); ++i) {
    if (is_lambda[i]) {
      drawer.point(coordinates[i], "red");
    } else {
      drawer.point(coordinates[i], "blue");
    }
    for (auto edge : graph[i]) {
      if (edge.to > edge.from) {
        continue;
      }
      std::string color = "lightgray";
      if (edge.owner == punter) {
        color = "black";
      } else if (edge.owner != kNoOwner) {
        color = "red";
      }

      const size_t width = std::binary_search(
          claimed_rivers.begin(), claimed_rivers.end(), std::make_pair(edge.to, edge.from)) ? 5 : 2;

      drawer.line(coordinates[edge.from], coordinates[edge.to], color, width);
    }
  }
  drawer.close();
}

bool Map::has_adjactent_owner(Vertex v, Punter owner) const {
  for (auto& e : graph[v]) {
    if (e.owner == owner) {
      return true;
    }
  }
  return false;
}

Vertex Map::vertex_id(Site size) {
  return std::lower_bound(sites.begin(), sites.end(), size) - sites.begin();
}

void Map::build_graph() {
  graph.clear();
  is_lambda.clear();
  lambda_vertices.clear();
  river_owners.assign(rivers.size(), kNoOwner);
  graph.resize(sites.size());
  std::cerr << "rebuild" << std::endl;
  for (size_t i = 0; i < rivers.size(); ++i) {
    Vertex u = vertex_id(rivers[i].first);
    Vertex v = vertex_id(rivers[i].second);
    auto iterator = claims.find(rivers[i]);
    Punter owner = (iterator != claims.end()) ? iterator->second : kNoOwner;
    graph[u].emplace_back(u, v, owner, i);
    graph[v].emplace_back(v, u, owner, i);
    river_owners[i] = owner;
  }
  is_lambda.assign(sites.size(), false);
  for (auto site : mines) {
    is_lambda[vertex_id(site)] = true;
    lambda_vertices.push_back(vertex_id(site));
  }
}

void Map::fill_distances() {
  distance_from_lambda.clear();
  distance_from_lambda.resize(graph.size());
  for (Vertex lambda : lambda_vertices) {
    fill_distances(lambda);
  }
}

void Map::fill_distances(Vertex lambda) {
  distance_from_lambda[lambda].assign(graph.size(), -1);
  std::queue<Vertex> q;
  distance_from_lambda[lambda][lambda] = 0;
  q.push(lambda);
  while (!q.empty()) {
    Vertex v = q.front();
    q.pop();
    auto new_distance = distance_from_lambda[lambda][v] + 1;
    for (auto& e : graph[v]) {
      auto& old_distance = distance_from_lambda[lambda][e.to];
      if (old_distance == -1 || old_distance > new_distance) {
        old_distance = new_distance;
        q.push(e.to);
      }
    }
  }
}

void Map::add_claim(const json& claim) {
  River r = make_river(claim);
  Punter p = claim["punter"];
  assert(static_cast<size_t>(p) < punters);
  bool added = claims.emplace(r, p).second;
  assert(added);
  std::swap(r.first, r.second);
  added = claims.emplace(r, p).second;
  assert(added);
}

std::vector<std::pair<size_t, size_t>> Map::add_moves(const json& new_moves) {
  std::vector<std::pair<size_t, size_t>> claimed_rivers;
  claimed_rivers.reserve(new_moves.size());
  for (auto& move: new_moves) {
    if (move.count("claim")) {
      const auto& claim = move["claim"];
      add_claim(claim);

      River river = make_river(claim);
      std::pair<size_t, size_t> our_river(vertex_id(river.first), vertex_id(river.second));
      if (our_river.first > our_river.second) {
        std::swap(our_river.first, our_river.second);
      }
      claimed_rivers.emplace_back(our_river);
    } else {
      assert(move.count("pass"));
    }
    moves.push_back(move);
  }
  std::sort(claimed_rivers.begin(), claimed_rivers.end());
  return claimed_rivers;
}

