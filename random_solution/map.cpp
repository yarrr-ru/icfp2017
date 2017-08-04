#include "map.h"

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

Map::Map(const json& old_state) : json_state(old_state) {
  punter = json_state["punter"];
  punters = json_state["punters"];
  assert(punters > 1);
  assert(static_cast<size_t>(punter) < punters);
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

void Map::add_claim(const json& claim) {
  River r = make_river(claim);
  Punter p = claim["punter"];
  assert(static_cast<size_t>(p) < punters);
  bool added = claims.emplace(r, p).second;
  assert(added);
}

void Map::add_moves(const json& new_moves) {
  for (auto& move: moves) {
    if (move.count("claim")) {
      add_claim(move["claim"]);
    } else {
      assert(move.count("pass"));
    }
    moves.push_back(move);
  }
}

