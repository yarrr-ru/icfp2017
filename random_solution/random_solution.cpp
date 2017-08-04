#include "launcher.h"

River make_move_random(const Map& map) {
  for (auto r : map.rivers) {
    if (!map.claims.count(r)) {
      return r;
    }
  }
  assert(false);
}

int main() {
  return main_launcher(make_move_random);
}
