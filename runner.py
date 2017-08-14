import fsr_reader
import running_stats
import midi_publisher
import threading
from time import time

class Runner():
  kDotThreshold = 0.1
  kDotMovingAvgThreshold = 0.002
  kGetStatsOnEvery = 10
  kDotScale = 10
  kDotDotScale = 10
  kDotMovingAvgScale = 10
  kNumNotes = 16
  kNumChannels = 3
  kNoteOnThreshold = 0.2
  kControlPinIndex = 7

  def __init__(self):
    self.reader = fsr_reader.FsrReader()
    self.collectors = {}
    self.publisher = midi_publisher.MidiPublisher()
    self.notes_on = [[False] * self.kNumNotes for _ in range(self.kNumChannels)]
    self.note_times = [[time()-60] * self.kNumNotes for _ in range(self.kNumChannels)]
    self.last_dot_moving_avg = [0] * self.kNumNotes

  def _clip_and_scale_stats(self, stats):
    mean = max(min(1.0, stats.mean), 0.0)
    mean_dot = max(min(1.0, stats.mean_dot * self.kDotScale), -1.0)
    mean_dot_dot = max(min(1.0, stats.mean_dot_dot * self.kDotDotScale), -1.0)
    dot_moving_avg = max(min(1.0, stats.dot_moving_avg * self.kDotMovingAvgScale), 0.0)
    return running_stats.RunningStats(
        mean=mean, mean_dot=mean_dot, mean_dot_dot=mean_dot_dot,
        dot_moving_avg=dot_moving_avg)

  def should_publish_control(self, pin_index, mean, dot, dot_dot):
    if abs(dot) > self.kDotThreshold:
      return True
    return False

  def should_publish_dot_moving_avg(self, pin_index, value):
    old_value = self.last_dot_moving_avg[pin_index]
    if old_value == value:
      return False
    if abs(value - old_value) > self.kDotMovingAvgThreshold:
      self.last_dot_moving_avg[pin_index] = value
      return True
    return False

  def check_for_basic_note_off(self, pin_index, mean, dot, ch):
    if not self.notes_on[ch-1][pin_index]:
      return False

    time_since = time()-self.note_times[ch-1][pin_index]
    if mean <= 0.3 or time_since > 6:
      self.notes_on[ch-1][pin_index] = False
      self.publisher.publish_note_off(pin_index, 0.5, ch)
      print 'Basic Note Off: ', pin_index, ' Channel: ', ch
      return True
    return False

  def check_for_activity_based_note_off(self, pin_index, mean, dot, ch):
    if not self.notes_on[ch-1][pin_index]:
      return False

    if mean <= 0.3 or self.last_dot_moving_avg[pin_index] < 0.05:
      self.notes_on[ch-1][pin_index] = False
      self.publisher.publish_note_off(pin_index, 0.5, ch)
      print 'Activity Note Off: ', pin_index, ' Channel: ', ch
      return True
    return False

  def check_for_time_based_note_off(self, pin_index, delay, ch):
    if not self.notes_on[ch-1][pin_index]:
      return False

    time_since = time()-self.note_times[ch-1][pin_index]
    if time_since > delay:
      self.notes_on[ch-1][pin_index] = False
      self.publisher.publish_note_off(pin_index, 0.5, ch)
      print 'Time Based Note Off: ', pin_index, ' Channel: ', ch
      return True
    return False

  def check_for_pressure_note_on(self, pin_index, mean, dot, ch):
    if self.notes_on[ch-1][pin_index]:
      return False

    time_since_activity_trigger = time()-self.note_times[2][pin_index]
    if mean >= self.kNoteOnThreshold and dot >= 0.16 and time_since_activity_trigger > 12:
      self.notes_on[ch-1][pin_index] = True
      self.note_times[ch-1][pin_index] = time()
      self.publisher.publish_note_on(pin_index, dot, ch)
      print 'Pressure Note On: ', pin_index, ' Channel: ', ch
      return True
    return False

  def check_for_velocity_note_on(self, pin_index, mean, dot, ch):
    if self.notes_on[ch-1][pin_index]:
      return False

    if mean >= 0.15 and dot >= 0.2:
      self.notes_on[ch-1][pin_index] = True
      self.note_times[ch-1][pin_index] = time()
      self.publisher.publish_note_on(pin_index, dot, ch)
      print 'Velocity Note On: ', pin_index, ' Channel: ', ch
      return True
    return False

  def check_for_activity_based_note_on(self, pin_index, dot_moving_avg, ch):
    if self.notes_on[ch-1][pin_index]:
      return False

    time_since = time()-self.note_times[ch-1][pin_index]
    if dot_moving_avg > 0.45 and time_since > 6:
      self.notes_on[ch-1][pin_index] = True
      self.note_times[ch-1][pin_index] = time()
      self.publisher.publish_note_on(pin_index, 0.7, ch)
      print 'Activity Note On: ', pin_index, ' Channel: ', ch
      return True
    return False

  def run(self):
    i = 0
    while True:
      # Read message and accumulate stats.
      msg = self.reader.read_message()
      if not msg:
        continue
      if not msg.pin_index in self.collectors:
        self.collectors[msg.pin_index] = running_stats.RunningStatsCollector()
      self.collectors[msg.pin_index].add(msg.value)

      # Get and publish stats only at an interval.
      i += 1
      if i < self.kGetStatsOnEvery:
        continue
      i = 0

      control_pin_on = False
      for pin_index, collector in self.collectors.iteritems():
        if pin_index == self.kControlPinIndex:
          s = self._clip_and_scale_stats(collector.get_stats())
          if s.mean > 0.4:
            control_pin_on = True

      channel = 1 if control_pin_on else 2

      for pin_index, collector in self.collectors.iteritems():
        s = self._clip_and_scale_stats(collector.get_stats())

        #if not self.check_for_pressure_note_on(pin_index, s.mean, s.mean_dot, ch = 1):
          #self.check_for_activity_based_note_off(pin_index, s.mean, s.mean_dot, ch = 1)

        if not self.check_for_velocity_note_on(pin_index, s.mean, s.mean_dot, ch = channel):
          # self.check_for_activity_based_note_off(pin_index, s.mean, s.mean_dot, ch = 2)
          self.check_for_time_based_note_off(pin_index, delay = 1, ch = channel)

        if not self.check_for_activity_based_note_on(pin_index, s.dot_moving_avg, ch = 3):
          self.check_for_time_based_note_off(pin_index, delay = 4, ch = 3)
        else:
          if self.notes_on[0][pin_index]:
            self.notes_on[0][pin_index] = False
            self.publisher.publish_note_off(pin_index, 0.8, 1)
          if self.notes_on[1][pin_index]:
            self.notes_on[1][pin_index] = False
            self.publisher.publish_note_off(pin_index, 0.8, 2)


        if self.should_publish_dot_moving_avg(pin_index, s.dot_moving_avg):
          self.publisher.publish_control_change(self.kNumNotes + pin_index, s.dot_moving_avg)

        if not self.should_publish_control(pin_index, s.mean, s.mean_dot, s.mean_dot_dot):
          continue
        self.publisher.publish_control_change(pin_index, s.mean)
