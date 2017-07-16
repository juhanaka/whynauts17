import collections
import numpy as np

Stats = collections.namedtuple('Stats', ['mean', 'mean_dot', 'mean_dot_dot'])

class RunningStatsCollector():
  def __init__(self, window_length):
    self.values = collections.deque([], window_length)

  def add(self, value):
    self.values.append(value)

  def get_stats(self):
    mean = float(sum(self.values)) / len(self.values)
    dot = np.gradient(self.values)
    mean_dot = float(sum(dot)) / len(dot)
    dot_dot = np.gradient(dot)
    mean_dot_dot = float(sum(dot_dot)) / len(dot_dot)
    return Stats(mean=mean, mean_dot=mean_dot, mean_dot_dot=mean_dot_dot)

