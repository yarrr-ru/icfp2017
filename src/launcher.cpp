#include "launcher.h"
#include <sstream>

namespace {

json read_json() {
  size_t length = 0;
  char delim = 0;
  std::cin >> length >> delim;
  assert(delim == ':' && length > 0);
  // std::cerr << "read_json length: " << length << std::endl;
  std::vector<char> bytes(length);
  std::cin.read(bytes.data(), length);
  std::stringstream v;
  v.write(bytes.data(), length);
  json result;
  v >> result;
  return result;
}

void write_json(const json& result) {
  std::string s = result.dump();
  std::cout << s.length() << ':' << s;
  // std::cerr << "write_json length: " << s.length() << std::endl;
}

json setup(MakeFutures make_futures, const json& request) {
  // std::cerr << "setup: " << request << std::endl;
  Map map(request);
  json response;
  response["ready"] = map.punter;
  response["state"] = map.to_json();
  if (make_futures && map.futures_supported) {
    std::cerr << "try to get futures from strategy" << std::endl;
    auto futures = make_futures(map);
    if (!futures.empty()) {
      std::vector<json> json_futures;
      for (auto future : futures) {
        json json_future;
        json_future["source"] = future.first;
        json_future["target"] = future.second;
        json_futures.push_back(json_future);
      }
      response["futures"] = json_futures;
      response["state"]["futures"] = json_futures;
      std::cerr << "futures ready: " << json_futures.size() << std::endl;
    }
  }
  // std::cerr << "setup: " << map.punter << std::endl;
  return response;
}

json move(MakeMove make_move, const json& request) {
  // std::cerr << "move: " << request << std::endl;
  Map map(request["state"], request["move"]["moves"]);
  json response;
  response["state"] = map.to_json();
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

bool handshake() {
  json me;
  me["me"] = "aimtech";
  write_json(me);
  json you = read_json();
  return you["you"] == "aimtech";
}

}  // namespace

int main_launcher(MakeMove make_move, MakeFutures make_futures) {
  if (!handshake()) {
    std::cerr << "bad handshake" << std::endl;
    return 1;
  }
  json request = read_json();
  if (request.count("punter")) {
    write_json(setup(make_futures, request));
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
