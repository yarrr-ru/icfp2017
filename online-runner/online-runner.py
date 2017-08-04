#!/usr/bin/env python3

import argparse
import json
import progressbar
import socket
import subprocess
import sys
import threading
import time

BUFFER_SIZE = 4*1024

def json_to_bytearray(json_data):
  s = json.dumps(json_data)
  s = str(len(s)) + ":" + s
  return bytearray(s, "utf-8")

OUR_IDS = {}
STOP_JSONS = []
PROGRESS_BAR = None

class OnlineRunner:
  def __init__(self, port, name, binary, index):
    self.port = port
    self.name = name + "." + str(index)
    self.log_name = binary + "." + str(index)
    self.binary = binary
    self.index = index
    self.protocol_log_file = open("protocol." + str(index) + ".log", "w")
    self.runner_log_file = sys.stderr
    self.strategy_log_file = open("strategy." + str(index) + ".log", "w")
    self.data_queue = bytearray()


  def send_json(self, json_data):
    print('sending json to server:', json.dumps(json_data), file=self.protocol_log_file)
    data_to_send = json_to_bytearray(json_data)
    data_sent = self.sock.send(data_to_send)
    assert data_sent == len(data_to_send)


  def receive_json(self):
    if len(self.data_queue) == 0:
      self.data_queue = self.sock.recv(BUFFER_SIZE)
    expected_length = 0
    prefix_length = 0
    for c in self.data_queue:
      prefix_length += 1
      if c == ord(':'):
        break
      else:
        expected_length = 10 * expected_length + c - ord('0')
    self.data_queue = self.data_queue[prefix_length:]

    data = bytearray()
    while len(data) != expected_length:
      need_length = expected_length - len(data)
      if len(self.data_queue) < need_length:
        data.extend(self.data_queue)
        self.data_queue = self.sock.recv(BUFFER_SIZE)
      else:
        data.extend(self.data_queue[:need_length])
        self.data_queue = self.data_queue[need_length:]

    json_data = json.loads(data.decode("utf-8"))
    print('received json from server:', json.dumps(json_data), file=self.protocol_log_file)
    return json_data

  def receive_json_from_strategy(self, stdout_data):
    if len(stdout_data) == 0:
      return None

    expected_length = 0
    prefix_length = 0
    for b in stdout_data:
      prefix_length += 1
      if b == ord(':'):
        break
      else:
        expected_length = 10 * expected_length + b - ord('0')
    data = stdout_data[prefix_length:expected_length+prefix_length]
    json_data = json.loads(data.decode("utf-8"))
    print('received json from strategy:', json.dumps(json_data), file=self.protocol_log_file)
    return json_data

  def run_strategy(self, json_data, timeout_seconds):
    start_time = time.time()
    proc = subprocess.Popen([self.binary],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print('sending json to strategy:', json.dumps(json_data), file=self.protocol_log_file)
    stdout, stderr = proc.communicate(json_to_bytearray(json_data))
    end_time = time.time()
    print(stderr.decode('utf-8'), file=self.strategy_log_file)
    if end_time - start_time > timeout_seconds / 2:
      print('strategy run time exceeds half a timeout: %.3f, %.3f'
          % (end_time - start_time, timeout_seconds), file=self.runner_log_file)
    if proc.returncode != 0:
      raise RuntimeError(self.log_name + " returned with non-zero return code: " + str(proc.returncode))
    return self.receive_json_from_strategy(stdout)

  def online_handshake(self, name):
    self.send_json({"me": name})
    answer = self.receive_json()
    assert answer["you"] == name
    print('successful handshake with server name: ' + name, file=self.runner_log_file)

  
  def setup(self):
    self.online_handshake(self.name)
    setup_json = self.receive_json()
    self.our_id = setup_json["punter"]
    self.total_players = setup_json["punters"]
    self.total_rivers = len(setup_json["map"]["rivers"])
    print(self.log_name + ': received setup json our_id: {} total players: {}'.format(self.our_id, self.total_players),
        file=self.runner_log_file)
    ready_json = self.run_strategy(setup_json, 10)
    assert ready_json["ready"] == self.our_id
    state = ready_json.pop("state")
    self.send_json(ready_json)
    return state


  def __call__(self):
    self.sock = socket.socket()
    self.sock.connect((SERVER, self.port))
    self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    state = self.setup()

    move_loop_count = 0
    total_move_loops = self.total_rivers // self.total_players
    if self.index == 0:
      PROGRESS_BAR = progressbar.ProgressBar(max_value=total_move_loops)
      PROGRESS_BAR.update(0)
    while True:
      if self.index == 0 and move_loop_count <= total_move_loops:
        PROGRESS_BAR.update(move_loop_count)
      moves_json = self.receive_json()
      moves_json["state"] = state
      new_move_json = self.run_strategy(moves_json, 1)
      if "stop" in moves_json:
        OUR_IDS[self.our_id] = self.index
        STOP_JSONS.append(moves_json)
        break
      if move_loop_count > 0:
        for move in moves_json["move"]["moves"]:
          if "pass" in move and move["pass"]["punter"] == self.our_id:
            print(self.log_name + ": passed on turn", move_loop_count - 1, file=self.runner_log_file)
      state = new_move_json.pop("state")

      self.send_json(new_move_json)
      move_loop_count += 1

    self.protocol_log_file.close()
    self.strategy_log_file.close()
    

SERVER = "punter.inf.ed.ac.uk"


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("port", type=int)
  parser.add_argument("name", help="name to log into the server")
  parser.add_argument("binaries", help="binaries to run", nargs='+')
  args = parser.parse_args()

  threads = []
  for i in range(len(args.binaries)):
   thread = threading.Thread(target=OnlineRunner(args.port, args.name, args.binaries[i], i))
   thread.start()
   threads.append(thread)

  for thread in threads:
    thread.join()

  if len(STOP_JSONS) != len(args.binaries) or len(OUR_IDS) != len(args.binaries):
    print("Some threads failed")
    sys.exit(1)

  scores_json = STOP_JSONS[0]["stop"]["scores"]
  scores = [] 
  for score in scores_json:
    punter_id = score["punter"]
    punter_name = "unknown player"
    if punter_id in OUR_IDS:
      index = OUR_IDS[punter_id]
      punter_name = args.binaries[index] + "." + str(index)
    scores.append((score["score"], punter_name))
  scores = sorted(scores, reverse=True)
  max_value = scores[0][0]

  print("\nScores:")
  for score in scores:
    print(score[0], score[1])

if __name__ == "__main__":
  main()
