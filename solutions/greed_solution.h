#pragma once
#include "launcher.h"
#include <algorithm>

inline River make_move_greed(const Map& map) {
  auto river_owners = map.river_owners;

  int64_t max_score = -1;
  River best_river;

  for (size_t river_index = 0; river_index < river_owners.size(); ++river_index) {
    if (river_owners[river_index] != kNoOwner) {
      continue;
    }

    river_owners[river_index] = map.punter;
    auto score = map.get_score_by_river_owners(river_owners, map.punter);
    river_owners[river_index] = kNoOwner;
    if (score > max_score) {
      max_score = score;
      best_river = map.get_river(river_index);
    }
  }
  assert(max_score >= 0);
  return best_river;
}

