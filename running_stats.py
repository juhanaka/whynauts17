import collections
import numpy as np

class RunningStatsCollector():
  def __init__(self, window_length):
    self.values = collections.deque([], window_length)
    self.prev_exp_moving_avg = 0.0
    self.kAlpha = 0.005
    self.kBeta = 2.0

  def add(self, value):
    self.values.append(value)

  def get_stats(self):
    mean = sum(self.values) / float(len(self.values))
    dot = np.gradient(self.values) if len(self.values) > 1 else [0]
    mean_dot = float(sum(dot)) / len(dot)
    dot_dot = np.gradient(dot) if len(dot) > 1 else [0]
    mean_dot_dot = float(sum(dot_dot)) / len(dot_dot)

    self.prev_exp_moving_avg = (self.kAlpha * self.kBeta * abs(mean_dot) +
      (1 - self.kAlpha) * self.prev_exp_moving_avg)

    return (mean, mean_dot, mean_dot_dot, self.prev_exp_moving_avg)

