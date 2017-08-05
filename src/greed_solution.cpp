#include "greed.h"
#include "random.h"

River make_move_greed_with_fb(const Map& map) {
  auto river = make_move_greed(map);
  if (river.first != river.second) {
    return river;
  }
  return make_move_random(map);
}

int main() {
  return main_launcher(make_move_greed_with_fb);
}
