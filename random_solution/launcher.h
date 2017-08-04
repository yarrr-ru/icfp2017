#pragma once
#include "map.h"

using MakeMove = std::function<River(const Map&)>;

int main_launcher(MakeMove make_move);
