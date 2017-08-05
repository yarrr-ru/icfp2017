#!/usr/bin/env python3

import argparse
import os
import progressbar
import queue 
import random
import subprocess
import sys
import threading
import time 
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

matches_lock = threading.Lock()
matches_count = 0

points_by_strategy = {}
scores_by_strategy  = {}

def run_match(server, strategies):
  script = os.path.join(".", os.path.dirname(sys.argv[0]), SCRIPT_NAME)
  run_line = [script, str(server.port), random.choice(FUNNY_NAMES)]
  run_line.extend(strategies)
  print("running match on map", server.map_name, run_line, file=sys.stderr)
  proc = subprocess.Popen(run_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  stdout, stderr = proc.communicate()
  if proc.returncode != 0:
     print("returned with nonzero exit code:", proc.returncode, file=sys.stderr)
     return 

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

  print("\n".join(["match results:"] + lines))

  global matches_count
  with matches_lock: matches_count += 1


def match_map(server, map_type):
  if map_type == "small":
    return server.max_players <= 4
  elif map_type == "medium":
    return server.max_players == 8
  elif map_type == "large":
    return server.max_players == 16
  else:
    return server.map_name == map_type


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


def get_min_run_count():
  min_run_count = len(list(points_by_strategy.values())[0])
  for points_list in points_by_strategy.values():
    min_run_count = min(min_run_count, len(points_list))
  return min_run_count

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("-m", "--map", help="map json name or 'small, 'medium' or 'large'", required=True)
  parser.add_argument("-s", "--strategies", nargs='+', required=True)
  parser.add_argument("-r", "--rounds", type=int, required=True)
  parser.add_argument("-t", "--threads", type=int, default="8")
  args = parser.parse_args()

  global points_by_strategy
  for strategy in args.strategies:
    points_by_strategy[strategy] = []
    scores_by_strategy[strategy] = []

  random.seed()

  threads = []
  while matches_count < args.rounds or get_min_run_count() == 0:
      new_threads = []
      for thread in threads:
        if thread.is_alive():
          new_threads.append(thread)
      threads = new_threads
      if len(threads) == args.threads:
        time.sleep(1)
        continue

      servers = parse_server_list()
      free_servers = list(filter(
          lambda server: 
            len(server.player_names) == 0 and match_map(server, args.map),
          servers))
      if len(free_servers) == 0:
        print("no free servers found for map:", args.map, file=sys.stderr)
        time.sleep(1)
        continue

      random.shuffle(free_servers)

      matches_to_run = min(len(free_servers), args.threads - len(threads)) 
      for i in range(matches_to_run):
        server = free_servers[i]
        thread = threading.Thread(
            target=run_match,
            args=(server, get_strategies_to_run(server.max_players, args.strategies)))
        thread.start()
        threads.append(thread)

  for thread in threads:
    thread.join()

  min_run_count = get_min_run_count()
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
