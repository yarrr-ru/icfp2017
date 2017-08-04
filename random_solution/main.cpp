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


json setup(const json& request) {
  std::cerr << "setup: " << request << std::endl;
  return request;
}

json move(const json& request) {
  std::cerr << "move: " << request << std::endl;
  return request;
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
