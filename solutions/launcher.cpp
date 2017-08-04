#include "launcher.h"

namespace {

json setup(const json& request) {
  std::cerr << "setup: " << request << std::endl;
  Map map(request);
  json response;
  response["ready"] = map.punter;
  response["state"] = map.to_json();
  std::cerr << "setup: " << map.punter << std::endl;
  return response;
}

json move(MakeMove make_move, const json& request) {
  std::cerr << "move: " << request << std::endl;
  Map map(request["state"], request["move"]["moves"]);
  json response;
  response["state"] = map.to_json();
  // TODO(artem): measure time
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

}  // namespace

int main_launcher(MakeMove make_move) {
  json request = read_json();
  if (request.count("punter")) {
    write_json(setup(request));
  } else if (request.count("move")) {
    write_json(move(make_move, request));
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