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
  args = parser.parse_args():1

  sock = socket.socket()
  sock.connect((args.server, int(args.port)))
  
  online_handshake(sock, args.name)
  setup_json = receive_json(sock)

  solution = subprocess.Popen(args.binary, stdin=subprocess.PIPE, stdout=subprocess.PIPE)


if __name__ == "__main__":
  main()
