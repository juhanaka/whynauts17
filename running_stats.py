import collections
import numpy as np
import threading
import copy

class RunningStatsCollector():
  def __init__(self, window_length):
    self.values = collections.deque([], window_length)

  def add(self, value):
    self.values.append(value)

  def get_stats(self):
    mean = sum(self.values) / float(len(self.values))
    dot = map(abs, np.gradient(self.values) if len(self.values) > 1 else [0])
    mean_dot = float(sum(dot)) / len(dot)
    dot_dot = map(abs, np.gradient(dot) if len(dot) > 1 else [0])
    mean_dot_dot = float(sum(dot_dot)) / len(dot_dot)
    return (mean, abs(mean_dot), abs(mean_dot_dot))

