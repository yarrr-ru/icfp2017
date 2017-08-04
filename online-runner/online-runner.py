#!/usr/bin/env python3

import argparse
import json
import socket
import subprocess
import sys
import threading

BUFFER_SIZE = 4*1024


def json_to_bytearray(json_data):
  s = json.dumps(json_data)
  s = str(len(s)) + ":" + s
  return bytearray(s, "utf-8")


class OnlineRunner:
  def __init__(self, port, name, binary, index):
    self.port = port
    self.name = name + "." + str(index)
    self.binary = binary
    self.index = index
    self.protocol_log_file = open("protocol." + str(index) + ".log", "w")
    self.runner_log_file = open("runner." + str(index) + ".log", "w")
    self.strategy_log_file = open("strategy." + str(index) + ".log", "w")
    self.data_queue = bytearray()


  def send_json(self, json_data):
    print('sending json to server:', json_data, file=self.protocol_log_file)
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
    print('received json from server:', json_data, file=self.protocol_log_file)
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
    print('received json from strategy:', json_data, file=self.protocol_log_file)
    return json_data

  def run_strategy(self, json_data):
    proc = subprocess.Popen([self.binary],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print('sending json to strategy:', json_data, file=self.protocol_log_file)
    stdout, stderr = proc.communicate(json_to_bytearray(json_data))
    print(stderr.decode('utf-8'), file=self.strategy_log_file)
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
    print('received setup json our_id: {} total players: {}'.format(self.our_id, self.total_players),
        file=self.runner_log_file)
    ready_json = self.run_strategy(setup_json) 
    assert ready_json["ready"] == self.our_id
    self.send_json({"ready": self.our_id})
    return ready_json["state"]


  def __call__(self):
    self.sock = socket.socket()
    self.sock.connect((SERVER, self.port))
    self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    state = self.setup()
    print('strategy state size:', len(json.dumps(state)), file=self.runner_log_file)

    while True:
      moves_json = self.receive_json()
      print('received moves', file=self.runner_log_file)
      moves_json["state"] = state
      new_move_json = self.run_strategy(moves_json)
      if "stop" in moves_json:
        print("our id:", self.our_id)
        scores = [0 for x in range(self.total_players)]
        for score in moves_json["stop"]["scores"]:
          cur_id = score["punter"]
          cur_score = score["score"]
          scores[cur_id] = cur_score
          if score["punter"] == self.our_id:
            print("our score:", cur_score)
        print("all scores:", scores)
        break
      state = new_move_json.pop("state")
      print('strategy state size:', len(json.dumps(state)), file=self.runner_log_file)
      self.send_json(new_move_json)

    self.protocol_log_file.close()
    self.runner_log_file.close()
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


if __name__ == "__main__":
  main()
