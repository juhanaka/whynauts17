import collections
import numpy as np
# import threading
# import copy

class RunningStatsCollector():
  def __init__(self, window_length):
    self.values = collections.deque([], window_length)
    self.change_decay_values = collections.deque([], 50)
    self.change_decay_history = collections.deque([], 9)
    self.weights = [
      0.001, 0.002, 0.004, 0.008, 0.016,  # 0.1, 0.2, 0.3, 0.4, 0.5,
      0.032, 0.064, 0.128, 0.256   # 0.6, 0.7, 0.8, 0.9, 1
    ]

  def add(self, value):
    self.values.append(value)

  def get_stats(self):
    mean = sum(self.values) / float(len(self.values))
    dot = np.gradient(self.values) if len(self.values) > 1 else [0]
    mean_dot = float(sum(dot)) / len(dot)
    dot_dot = np.gradient(dot) if len(dot) > 1 else [0]
    mean_dot_dot = float(sum(dot_dot)) / len(dot_dot)

    self.change_decay_values.append(abs(mean_dot))
    change_avg = sum(self.change_decay_values) / float(len(self.change_decay_values))

    change_decay = change_avg
    for i in range(0, len(self.change_decay_history)):
      change_decay += self.weights[i] * self.change_decay_history[i]
    self.change_decay_history.append(change_decay)


    return (mean, mean_dot, mean_dot_dot, change_decay)

