#!/usr/bin/env python3

import argparse
import json
import socket
import subprocess
import sys

BUFFER_SIZE = 4*1024


def json_to_bytearray(json_data):
  s = json.dumps(json_data)
  s = str(len(s)) + ":" + s
  return bytearray(s, "utf-8")


def send_json(sock, json_data):
  print('sending json to server:', json_data)
  data_to_send = json_to_bytearray(json_data)
  data_sent = sock.send(data_to_send)
  assert data_sent == len(data_to_send)


def send_json_to_strategy(strategy, json_data):
  data_to_send = json_to_bytearray(json_data)
  data_sent = strategy.stdin.write(data_to_send)
  assert data_sent == len(data_to_send)


data_queue = bytearray()

def receive_json(sock):
  global data_queue
  if len(data_queue) == 0:
    data_queue = sock.recv(BUFFER_SIZE)
  expected_length = 0
  prefix_length = 0
  for c in data_queue:
    prefix_length += 1
    if c == ord(':'):
      break
    else:
      expected_length = 10 * expected_length + c - ord('0')
  data_queue = data_queue[prefix_length:]

  data = bytearray()
  while len(data) != expected_length:
    need_length = expected_length - len(data)
    if len(data_queue) < need_length:
      data.extend(data_queue)
      data_queue = sock.recv(BUFFER_SIZE)
    else:
      data.extend(data_queue[:need_length])
      data_queue = data_queue[need_length:]

  return json.loads(data.decode("utf-8"))


def receive_json_from_strategy(stdout_data):
  expected_length = 0
  prefix_length = 0
  for b in stdout_data:
    prefix_length += 1
    if b == ord(':'):
      break
    else:
      expected_length = 10 * expected_length + b - ord('0')
  data = stdout_data[prefix_length:expected_length+prefix_length]
  return json.loads(data.decode('utf-8'))


def online_handshake(sock, name):
  send_json(sock, {"me": name})
  answer = receive_json(sock)
  assert answer["you"] == name
  print('successful handshake with server name: ' + name, file=sys.stderr)

def run_subprocess(binary):
  return subprocess.Popen([binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

SERVER = "punter.inf.ed.ac.uk"

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("port")
  parser.add_argument("name")
  parser.add_argument("binary")
  args = parser.parse_args()

  sock = socket.socket()
  sock.connect((SERVER, int(args.port)))
  sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

  online_handshake(sock, args.name)
  setup_json = receive_json(sock)
  print(
      'received setup json our_id: {} total players: {}'.format(
          setup_json["punter"], setup_json["punters"]),
      file=sys.stderr)
  our_id = setup_json["punter"]
  setup_stdout, setup_stderr = run_subprocess(args.binary).communicate(json_to_bytearray(setup_json))
  print('strategy stderr:\n' + setup_stderr.decode('utf-8'), file=sys.stderr)
  ready_json = receive_json_from_strategy(setup_stdout)
  assert ready_json["ready"] == our_id
  state = ready_json["state"]
  print('strategy state size:', len(json.dumps(state)), file=sys.stderr)
  send_json(sock, {"ready": our_id})
  
  while True:
    moves_json = receive_json(sock)
    print('received moves:', moves_json, file=sys.stderr)
    moves_json["state"] = state
    new_move_stdout, new_move_stderr = run_subprocess(args.binary).communicate(json_to_bytearray(moves_json))
    print('strategy stderr:\n' + new_move_stderr.decode('utf-8'), file=sys.stderr)
    if "stop" in moves_json:
      break
    new_move_json = receive_json_from_strategy(new_move_stdout)
    state = new_move_json.pop("state")
    print('strategy state size:', len(json.dumps(state)), file=sys.stderr)
    send_json(sock, new_move_json)


if __name__ == "__main__":
  main()
