#include "greed.h"

River make_move_greed_st(const Map& map) {
  River r = make_move_greed_only_st(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_greed(map);
}

int main() {
  return main_launcher(make_move_greed_st);
}
