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

  def __lt__(self, rhs):
    return compare_servers(self, rhs)


def has_mm_players(server):
  for player in server.player_names:
    if len(player) > 3 and player[:3] == 'mm:':
      return True
  return False


def compare_servers(s1, s2):
  has_mm_1 = has_mm_players(s1)
  has_mm_2 = has_mm_players(s2)
  if has_mm_1 != has_mm_2:
    if has_mm_1:
      return -1
    else:
      return 1
  return len(s2.player_names) - len(s1.player_names)


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

all_points = []
all_scores = []

def avg(v):
  if len(v) == 0:
    return 0.0
  else:
    return sum(v) / len(v)

def run_match(server, strategy, name):
  script = os.path.join(".", os.path.dirname(sys.argv[0]), SCRIPT_NAME)
  run_line = [script, str(server.port), name, strategy] 
  print("running match", run_line, file=sys.stderr)
  proc = subprocess.Popen(run_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  stdout, stderr = proc.communicate()
  if proc.returncode != 0:
     print("returned with nonzero exit code:", proc.returncode, file=sys.stderr)
     return 

  lines = stdout.decode("utf-8").strip().split("\n")
  lines = lines[-server.max_players:]
  scores = []
  for line in lines:
    score_string, name = line.split()
    score = int(score_string)
    name = name[:name.rfind('.')]
    points = server.max_players
    for prev_name, prev_score in scores:
      if prev_score > score:
        points -= 1
    scores.append((name, score))
    if name == strategy:
      all_points.append(points)
      all_scores.append(score)
      break

  print("\n".join(["match results:"] + lines))


class ServerFilter:
  def __init__(self, target_map):
    self.target_map = target_map

  def __call__(self, server):
    return server.map_name == self.target_map and 'eager punter' not in server.player_names


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("-m", "--map", help="map json name", required=True)
  parser.add_argument("-s", "--strategy", required=True)
  parser.add_argument("-t", "--threads", type=int, default="8")
  parser.add_argument("-n", "--name", required=True)
  args = parser.parse_args()

  random.seed()

  threads = []
  while True:
      print("strategy: {}, total games played: {}, avg points: {:.3f}, avg score: {:.3f}".format(
          args.strategy, len(all_points), avg(all_points), avg(all_scores)))
      new_threads = []
      for thread in threads:
        if thread.is_alive():
          new_threads.append(thread)
      threads = new_threads
      if len(threads) == args.threads:
        time.sleep(10)
        continue

      servers = parse_server_list()
      free_servers = sorted(filter(ServerFilter(args.map), servers))
      if len(free_servers) == 0:
        print("no free servers found for map:", args.map, file=sys.stderr)
        time.sleep(10)
        continue

      random.shuffle(free_servers)

      matches_to_run = min(len(free_servers), args.threads - len(threads)) 
      for i in range(matches_to_run):
        server = free_servers[i]
        thread = threading.Thread(
            target=run_match,
            args=(server, args.strategy, "mm:" + args.name))
        thread.start()
        threads.append(thread)


if __name__ == "__main__":
  main()
