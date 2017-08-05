#include "launcher.h"
#include "maximize_cut.h"

int main() {
  return main_launcher(make_move_maximizing_cut);
}
