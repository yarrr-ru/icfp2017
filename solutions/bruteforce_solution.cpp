#include "launcher.h"
#include "bruteforce.h"
#include <algorithm>

int main() {
  return main_launcher(make_bruteforce_move);
}
