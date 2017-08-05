#pragma once
#include "map.h"

using MakeMove = std::function<River(const Map&)>;
using MakeFutures = std::function<Futures(const Map&)>;

int main_launcher(MakeMove make_move, MakeFutures make_futures = MakeFutures{});

