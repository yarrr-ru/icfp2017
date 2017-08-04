#include "json.hpp"
#include <iostream>
#include <vector>
#include <cstddef>
#include <cassert>

using json = nlohmann::json;

json read_json() {
  size_t length = 0;
  char delim = 0;
  std::cin >> length >> delim;
  assert(delim == ':' && length > 0);
  std::cerr << "read_json length: " << length << std::endl;
  json result;
  std::cin >> result;
  return result;
}

void write_json(const json& result) {
  std::string s = result.dump();
  std::cout << s.length() << ':' << s;
  std::cerr << "write_json length: " << s.length() << std::endl;
}

using River = std::pair<size_t, size_t>;
using Punter = size_t;
using Site = size_t;

River make_river(const json& river) {
    return {river["source"], river["target"]};
}

struct Map {
  Punter punter;
  size_t punters;
  std::vector<Site> sites;
  std::vector<Site> mines;
  std::vector<River> rivers;
  std::map<River, Punter> claims;
  std::vector<json> moves;

  json json_state;

  explicit Map(const json& old_state) : json_state(old_state) {
    punter = json_state["punter"];
    punters = json_state["punters"];
    assert(punters > 1);
    assert(punter < punters);
    add_moves(json_state["moves"]);
    for (auto& site : json_state["map"]["sites"]) {
      sites.push_back(site["id"]);
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
    std::cerr << "map punter: " << punter <<
        " punters: " << punters <<
        " mines: " << mines.size() <<
        " sites: " << sites.size() <<
        " rivers: " << rivers.size() <<
        " claims: " << claims.size() <<
        " moves: " << moves.size() <<
        std::endl;
  }

  void add_claim(const json& claim) {
    River r = make_river(claim);
    Punter p = claim["punter"];
    assert(p < punters);
    bool added = claims.emplace(r, p).second;
    assert(added);
  }

  void add_moves(const json& new_moves) {
    for (auto& move: moves) {
      if (move.count("claim")) {
        add_claim(move["claim"]);
      } else {
        assert(move.count("pass"));
      }
      moves.push_back(move);
    }
  }

  json to_json() {
    json_state["moves"] = moves;
    return json_state;
  }
};

json setup(const json& request) {
  std::cerr << "setup: " << request << std::endl;
  Map map(request);
  json response;
  response["ready"] = map.punter;
  response["state"] = map.to_json();
  std::cerr << "setup: " << map.punter << std::endl;
  return response;
}

River make_move(const Map& map) {
  for (auto r : map.rivers) {
    if (!map.claims.count(r) && !map.claims.count({r.second, r.first})) {
      return r;
    }
  }
  assert(false);
}

json move(const json& request) {
  std::cerr << "move: " << request << std::endl;
  Map map(request["state"]);
  map.add_moves(request["move"]["moves"]);
  json response;
  response["state"] = map.to_json();
  River r = make_move(map);
  json move;
  move["claim"]["punter"] = map.punter;
  move["claim"]["source"] = r.first;
  move["claim"]["target"] = r.second;
  std::cerr << "made move: " << move << std::endl;
  response["move"] = move;
  return response;
}

void stop(const json& request) {
  std::cerr << "stop: " << request << std::endl;
}

void timeout(const json& request) {
  std::cerr << "timeout: " << request << std::endl;
}

int main() {
  json request = read_json();
  if (request.count("punter")) {
    write_json(setup(request));
  } else if (request.count("move")) {
    write_json(move(request));
  } else if (request.count("stop")) {
    stop(request);
  } else if (request.count("timeout")) {
    timeout(request);
  } else {
    std::cerr << "unknown request: " << request << std::endl;
    return 1;
  }
  return 0;
}
