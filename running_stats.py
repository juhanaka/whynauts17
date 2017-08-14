import collections
import numpy as np

RunningStats = collections.namedtuple('RunningStats', ['mean', 'mean_dot', 'mean_dot_dot', 'dot_moving_avg'])

class RunningStatsCollector():
  """Holds a buffer of values and returns statistics on the
     values when queried."""
  kAlpha = 0.005  # Low values apply more smoothing.
  kBeta = 2.0  # High values make decay slower.
  kWindowLength = 10

  def __init__(self):
    self.values = collections.deque([], self.kWindowLength)
    self.prev_dot_moving_avg = 0.0

  def add(self, value):
    self.values.append(value)

  def get_stats(self):
    mean = sum(self.values) / float(len(self.values))
    dot = np.gradient(self.values) if len(self.values) > 1 else [0]
    mean_dot = float(sum(dot)) / len(dot)
    dot_dot = np.gradient(dot) if len(dot) > 1 else [0]
    mean_dot_dot = float(sum(dot_dot)) / len(dot_dot)

    # Exponential moving average with a bias kBeta.
    self.prev_dot_moving_avg = (
        self.kAlpha * self.kBeta * abs(mean_dot) +
        (1 - self.kAlpha) * self.prev_dot_moving_avg)
    return RunningStats(mean=mean, mean_dot=mean_dot,
                        mean_dot_dot=mean_dot_dot,
                        dot_moving_avg=self.prev_dot_moving_avg)

