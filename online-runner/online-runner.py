#! /usr/bin/python3

import argparse
import json 
import socket 
import subprocess

BUFFER_SIZE = 4*1024


def json_to_bytearray(json_data):
  s = json.dumps(json_data)
  s = str(len(s)) + ":" + s
  return bytearray(s, "utf-8")


def send_json(sock, json_data):
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
    if len(data_queue) <= need_length:
      data.extend(data_queue)
      data_queue = sock.recv(BUFFER_SIZE)
    else:
      data.extend(data_queue[:need_length])
      data_queue = data_queue[need_length:]

  return json.loads(data.decode("utf-8"))


def receive_json_from_strategy(strategy):
  expected_length = 0
  while True:
    b = strategy.stdout.read(1)[0]
    if b == ord(':'):
      break
    else:
      expected_length = 10 * expected_length + b - ord('0')
  data = strategy.stdout.read(expected_length)
  return json.loads(data.decode('utf-8'))


def online_handshake(sock, name):
  send_json(sock, {"me": name})
  answer = receive_json(sock)
  assert answer["you"] == name


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("server")
  parser.add_argument("port")
  parser.add_argument("name")
  parser.add_argument("binary")
  args = parser.parse_args()

  sock = socket.socket()
  sock.connect((args.server, int(args.port)))
  
  online_handshake(sock, args.name)

  state = {}
  with subprocess.Popen([args.binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE) as strategy:
    setup_json = receive_json(sock)
    our_id = setup_json["punter"]
    send_json_to_strategy(strategy, setup_json)
    ready_json = receive_json_from_strategy(strategy)
    state = ready_json["state"]
    assert ready_json["ready"] == our_id
    send_json(sock, {"ready": our_id})


  while True:
    moves_json = receive_json(sock)
    moves_json["state"] = state
    with subprocess.Popen([args.binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE) as strategy:
      send_json_to_strategy(strategy, moves_json)
      if "stop" in move_json:
        break
      new_move_json = receive_json_from_strategy(strategy)
      state = new_move_json.pop("state")
      send_json(sock, new_move_json)


if __name__ == "__main__":
  main()
