#include "launcher.h"

namespace {

json read_json() {
  size_t length = 0;
  char delim = 0;
  std::cin >> length >> delim;
  assert(delim == ':' && length > 0);
  // std::cerr << "read_json length: " << length << std::endl;
  json result;
  std::cin >> result;
  return result;
}

void write_json(const json& result) {
  std::string s = result.dump();
  std::cout << s.length() << ':' << s;
  // std::cerr << "write_json length: " << s.length() << std::endl;
}


json setup(const json& request) {
  // std::cerr << "setup: " << request << std::endl;
  Map map(request);
  json response;
  response["ready"] = map.punter;
  response["state"] = map.to_json();
  // std::cerr << "setup: " << map.punter << std::endl;
  return response;
}

json move(MakeMove make_move, const json& request) {
  // std::cerr << "move: " << request << std::endl;
  Map map(request["state"], request["move"]["moves"]);
  json response;
  response["state"] = map.to_json();
  // TODO(artem): measure time
  River r = make_move(map);
  response["claim"]["punter"] = map.punter;
  response["claim"]["source"] = r.first;
  response["claim"]["target"] = r.second;
  std::cerr << "made move: " << r.first << " " << r.second << std::endl;
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
