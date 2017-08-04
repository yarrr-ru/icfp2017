#include "launcher.h"

River make_move_random(const Map& map) {
  for (auto& edges : map.graph) {
    for (auto& e : edges) {
      if (e.owner == kNoOwner) {
        return map.get_river(e);
      }
    }
  }
  assert(false);
}

int main() {
  return main_launcher(make_move_random);
}
