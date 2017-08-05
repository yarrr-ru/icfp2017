#include "launcher.h"
#include "maximize_cut.h"
#include "greed.h"

int main() {
  return main_launcher(make_move_maximizing_cut, make_futures_shortest_path);
}
