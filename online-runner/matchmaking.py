#!/usr/bin/env python3

import argparse
import os
import progressbar
import random
import subprocess
import sys
import urllib.request
from bs4 import BeautifulSoup

SCRIPT_NAME = "online-runner.py"
SERVER_LIST_ADDR = "http://punter.inf.ed.ac.uk/status.html"

class GameServer:
  def __init__(self, port, max_players, player_names, map_name):
    self.port = port
    self.max_players = max_players
    self.player_names = player_names
    self.map_name = map_name

  def __str__(self):
    return "GameServer(port={}, max_players={}, player_names={}, map_name={})".format(
        self.port, self.max_players, str(self.player_names), self.map_name)


def parse_server_list():
  html = urllib.request.urlopen(SERVER_LIST_ADDR)
  soup = BeautifulSoup(html.read(), 'html.parser')
  servers = []
  for row in soup.find_all('tr'):
    port = 0,
    max_players = 0
    player_names = []
    map_name = ""
    i = 0
    for col in row.find_all('td'):
      if i == 0:
        status = col.string
        if 'Waiting for punters.' in status:
          max_players = int(status[status.rfind('/') + 1:status.rfind(')')])
      elif i == 1 and col.string != None:
        player_names = col.string.split(', ')
      elif i == 3:
        port = int(col.string)
      elif i == 4:
        map_name = col.string
      i += 1
    if max_players != 0:
      game_server = GameServer(port, max_players, player_names, map_name)
      servers.append(game_server)
  return servers

FUNNY_NAMES = ["eager.punter", "puntercalc3000", "test_punter_please_ignore", "GoGoPunterRanges"]

points_by_strategy = {}
scores_by_strategy  = {}

def run_match(server, strategies):
  script = os.path.join(".", os.path.dirname(sys.argv[0]), SCRIPT_NAME)
  while True:
    run_line = [script, str(server.port), random.choice(FUNNY_NAMES)]
    run_line.extend(strategies)
    print("running match on map", server.map_name, run_line, file=sys.stderr)
    proc = subprocess.Popen(run_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if proc.returncode != 0:
       print("returned with nonzero exit code, retying", file=sys.stderr)
       continue

    lines = stdout.decode("utf-8").strip().split("\n")
    lines = lines[-len(strategies):]
    scores = []
    was_updated = set()
    for line in lines:
      score, name = line.split()
      name = name[:name.rfind('.')]
      points = len(strategies)
      for prev_score, prev_name in scores:
        if prev_score > score:
          points -= 1
      scores.append((name, score))
      
      points_by_strategy[name].append(points)
      scores_by_strategy[name].append(score)

    print("match results:")
    print("\n".join(lines))
    break

def match_map_size(server_max_players, target_map_type):
  if target_map_type == "small":
    return server_max_players <= 4
  elif target_map_type == "medium":
    return server_max_players == 8
  elif target_map_type == "large":
    return server_max_players == 16
  raise RuntimeError("unknown map type:" + target_map_type) 


def get_strategies_to_run(slots, strategies):
  strategies_set = set()
  for strategy in strategies:
    strategies_set.add(strategy)

  strategies_to_run = []
  multiplier = 1
  while len(strategies) * multiplier < slots or slots % multiplier != 0:
    multiplier += 1
  assert multiplier < slots
  for _ in range(slots // multiplier):
    strategies_to_run.extend([strategies_set.pop()]*multiplier) 
  return strategies_to_run

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("-m", "--maps", choices=["small","medium","large"], required=True)
  parser.add_argument("-s", "--strategies", nargs='+', required=True)
  parser.add_argument("-r", "--rounds", type=int, required=True)
  args = parser.parse_args()

  global points_by_strategy
  for strategy in args.strategies:
    points_by_strategy[strategy] = []
    scores_by_strategy[strategy] = []

  random.seed()

  for _ in range(args.rounds):
      servers = parse_server_list()
      free_server = random.choice(list(filter(
          lambda server: 
            len(server.player_names) == 0 and match_map_size(server.max_players, args.maps),
          servers)))
      print("found free server:", free_server, file=sys.stderr)
      run_match(free_server, get_strategies_to_run(free_server.max_players, args.strategies))

  min_run_count = args.rounds
  for points_list in points_by_strategy.values():
    min_run_count = min(min_run_count, len(points_list))
  if min_run_count == 0:
    print("not enought round")
    return

  print()
  print("matchmaking results")
  print("run count per strategy:", min_run_count)
  for strategy in args.strategies:
    points = points_by_strategy[strategy][:min_run_count]
    scores = scores_by_strategy[strategy]

    total_points = sum(points)
    average_score = sum(score) / len(score)
    max_score = max(score)
    min_score = min(score)
    
    print(strategy, total_poins, average_score, min_score, max_score)


if __name__ == "__main__":
  main()
