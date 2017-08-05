#!/usr/bin/env python3

import argparse
import progressbar
import random
import subprocess
import sys
import os

SLOTS_BY_PORT = {
  9002: 2,  9003: 2,  9004: 2,  9005: 2,  9006: 2,
  9007: 2,  9008: 2,  9009: 2, 9010: 2, 9011: 4,
 9012: 4, 9013: 4, 9014: 4, 9015: 4, 9016: 4,
 9017: 4, 9018: 4, 9019: 4, 9020: 4, 9021: 3,
 9022: 3, 9023: 3, 9024: 3, 9025: 3, 9026: 3,
 9027: 3, 9028: 3, 9029: 3, 9030: 3, 9031: 4,
 9032: 4, 9033: 4, 9034: 4, 9035: 4, 9036: 4,
 9037: 4, 9038: 4, 9039: 4, 9040: 4, 9041: 4,
 9042: 4, 9043: 4, 9044: 4, 9045: 4, 9046: 4,
 9047: 4, 9048: 4, 9049: 4, 9050: 4, 9051: 4,
 9052: 4, 9053: 4, 9054: 4, 9055: 4, 9056: 4,
 9057: 4, 9058: 4, 9059: 4, 9060: 4, 9061: 8,
 9062: 8, 9063: 8, 9064: 8, 9065: 8, 9066: 8,
 9067: 8, 9068: 8, 9069: 8, 9070: 8, 9071: 8,
 9072: 8, 9073: 8, 9074: 8, 9075: 8, 9076: 8,
 9077: 8, 9078: 8, 9079: 8, 9080: 8, 9081: 16,
 9082: 16, 9083: 16, 9084: 16, 9085: 16, 9086: 16,
 9087: 16, 9088: 16, 9089: 16, 9090: 16, 9091: 16,
 9092: 16, 9093: 16, 9094: 16, 9095: 16, 9096: 16,
 9097: 16, 9098: 16, 9099: 16, 9100: 16, 9101: 16,
 9102: 16, 9103: 16, 9104: 16, 9105: 16, 9106: 16,
 9107: 16, 9108: 16, 9109: 16, 9110: 16, 9111: 16,
 9112: 16, 9113: 16, 9114: 16, 9115: 16, 9116: 16,
 9117: 16, 9118: 16, 9119: 16, 9120: 16, 9121: 2,
 9122: 2, 9123: 2, 9124: 2, 9125: 2, 9126: 2,
 9127: 2, 9128: 2, 9129: 2, 9130: 2, 9131: 4,
 9132: 4, 9133: 4, 9134: 4, 9135: 4, 9136: 4,
 9137: 4, 9138: 4, 9139: 4, 9140: 4, 9141: 3,
 9142: 3, 9143: 3, 9144: 3, 9145: 3, 9146: 3,
 9147: 3, 9148: 3, 9149: 3, 9150: 3, 9151: 4,
 9152: 4, 9153: 4, 9154: 4, 9155: 4, 9156: 4,
 9157: 4, 9158: 4, 9159: 4, 9160: 4, 9161: 4,
 9162: 4, 9163: 4, 9164: 4, 9165: 4, 9166: 4,
 9167: 4, 9168: 4, 9169: 4, 9170: 4, 9171: 4,
 9172: 4, 9173: 4, 9174: 4, 9175: 4, 9176: 4,
 9177: 4, 9178: 4, 9179: 4, 9180: 4, 9181: 8,
 9182: 8, 9183: 8, 9184: 8, 9185: 8, 9186: 8,
 9187: 8, 9188: 8, 9189: 8, 9190: 8, 9191: 8,
 9192: 8, 9193: 8, 9194: 8, 9195: 8, 9196: 8,
 9197: 8, 9198: 8, 9199: 8, 9200: 8, 9201: 16,
 9202: 16, 9203: 16, 9204: 16, 9205: 16, 9206: 16,
 9207: 16, 9208: 16, 9209: 16, 9210: 16, 9211: 16,
 9212: 16, 9213: 16, 9214: 16, 9215: 16, 9216: 16,
 9217: 16, 9218: 16, 9219: 16, 9220: 16, 9221: 16,
 9222: 16, 9223: 16, 9224: 16, 9225: 16, 9226: 16,
 9227: 16, 9228: 16, 9229: 16, 9230: 16, 9231: 16,
 9232: 16, 9233: 16, 9234: 16, 9235: 16, 9236: 16,
 9237: 16, 9238: 16, 9239: 16, 9240: 16,
}

SCRIPT_NAME = "online-runner.py"

FUNNY_NAMES = ["puntercalc3000", "test_punter_please_ignore", "GoGoPunterRanges", "Artem Pidr"]

all_points_by_strategy = {}
last_run_score_by_strategy = {}
run_count_by_strategy = {}

def run_match(port, strategies, max_rounds):
  script = os.path.join(".", os.path.dirname(sys.argv[0]), SCRIPT_NAME)
  while True:
    run_line = [script, str(port), random.choice(FUNNY_NAMES)]
    run_line.extend(strategies)
    #print("running match:", run_line, file=sys.stderr)
    proc = subprocess.Popen(run_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if proc.returncode != 0:
       #print("returned with nonzero exit code, retying", file=sys.stderr)
       continue

    lines = stdout.decode("utf-8").strip().split("\n")
    lines = lines[-len(strategies):]
    scores = []
    for line in lines:
      score, name = line.split()
      name = name[:name.rfind('.')]
      points = len(strategies)
      for prev_score, prev_name in scores:
        if prev_score > score:
          points -= 1
      scores.append((name, score))

      run_count_by_strategy[name] += 1
      all_points_by_strategy[name].append(points)
      last_run_score_by_strategy[name] = int(score)
    #print("success, scores:", scores)
    break

class StrategiesWidget:
  def __call__(self, process, data):
    strategy_lines = []
    min_run_count = min(run_count_by_strategy.values())
    for strategy, last_run_score in last_run_score_by_strategy.items():
      points = sum(all_points_by_strategy[strategy][:min_run_count])
      strategy_lines.append("%s:(%d,%d)" % (strategy, last_run_score, points))
    return " ".join(strategy_lines)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("-p", "--ports", type=int, nargs='+', required=True)
  parser.add_argument("-s", "--strategies", nargs='+', required=True)
  parser.add_argument("-r", "--rounds", type=int, required=True)
  args = parser.parse_args()

  global all_points_by_strategy
  global last_run_score_by_strategy
  global run_count_by_strategy
  for strategy in args.strategies:
    all_points_by_strategy[strategy] = []
  last_run_score_by_strategy = dict.fromkeys(args.strategies, 0)
  run_count_by_strategy = dict.fromkeys(args.strategies, 0)

  random.seed()

  progress_bar = progressbar.ProgressBar(
      max_value=args.rounds*len(args.ports), widgets=[StrategiesWidget(), ' '])
  progress_bar.widgets.extend(progress_bar.default_widgets())
  progress = 0
  progress_bar.update(0)

  for _ in range(args.rounds):
    for port in args.ports:
      slots = SLOTS_BY_PORT[port]
      strategies_by_run_count = {}
      for strategy, run_count in run_count_by_strategy.items():
        if run_count not in strategies_by_run_count:
          strategies_by_run_count[run_count] = set()
        strategies_by_run_count[run_count].add(strategy)
      
      strategies_to_run = []
      multiplier = 1
      while len(args.strategies) * multiplier < slots or slots % multiplier != 0:
        multiplier += 1
      assert multiplier < slots
      for run_count, strategies in sorted(strategies_by_run_count.items()):
        need_strategies = slots - len(strategies_to_run)
        if need_strategies >= len(strategies)*multiplier:
          strategies_to_run.extend(list(strategies)*multiplier)
        else:
          for i in range(need_strategies // multiplier):
            strategy = strategies.pop()
            strategies_to_run.extend([strategy]*multiplier)

        if len(strategies_to_run) == slots:
          break

      run_match(port, strategies_to_run, args.rounds)

      progress += 1
      progress_bar.update(progress)

  min_run_count = min(run_count_by_strategy.values())
  print()
  print("run count per strategy:", min_run_count)
  for strategy in args.strategies:
    print(strategy, sum(all_points_by_strategy[strategy][:min_run_count]))


if __name__ == "__main__":
  main()
