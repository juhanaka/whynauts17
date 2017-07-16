import collections
import numpy as np
import threading
import copy

class RunningStatsCollector():
  def __init__(self, window_length):
    self.values = collections.deque([], window_length)
    self.m = threading.Lock()

  def add(self, value):
    with self.m:
      self.values.append(value)

  def get_stats(self):
    values = None
    with self.m:
      values = copy.deepcopy(self.values)

    mean = float(sum(values)) / len(values)
    dot = map(abs, np.gradient(values) if len(values) > 1 else [0])
    mean_dot = float(sum(dot)) / len(dot)
    dot_dot = map(abs, np.gradient(dot) if len(dot) > 1 else [0])
    mean_dot_dot = float(sum(dot_dot)) / len(dot_dot)
    return (mean, abs(mean_dot), abs(mean_dot_dot))

