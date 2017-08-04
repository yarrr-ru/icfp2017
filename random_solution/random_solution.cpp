#include "map.h"

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
  Map map(request["state"], request["move"]["moves"]);
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
