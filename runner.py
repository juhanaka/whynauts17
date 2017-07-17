import fsr_reader
import running_stats
import midi_publisher
import threading
import time

class Runner():
  kDotThreshold = 0.01
  kMeanIndex = 0
  kDotIndex = 1
  kDotDotIndex = 2
  kGetStatsOnEvery = 20
  kDotScale = 10

  def __init__(self):
    self.reader = fsr_reader.FsrReader()
    self.collectors = {}
    self.publisher = midi_publisher.MidiPublisher()

  def should_publish(self, pin_index, mean, dot, dot_dot):
    if dot > self.kDotThreshold:
      return True
    return False

  def run(self, window_length, min_diff, tick_seconds):
    i = 0
    while True:
      msg = self.reader.read_message()
      if not msg:
        continue
      if not msg.pin_index in self.collectors:
        self.collectors[msg.pin_index] = running_stats.RunningStatsCollector(window_length)
      self.collectors[msg.pin_index].add(msg.value)

      i += 1
      if i < self.kGetStatsOnEvery:
        continue
      i = 0

      for pin_index, collector in self.collectors.iteritems():
        s = collector.get_stats()
        if not self.should_publish(pin_index, s[self.kMeanIndex], s[self.kDotIndex], s[self.kDotDotIndex]):
          continue
        if s[self.kDotIndex] > 0.0:
          dot_value = min(1.0, self.kDotScale * s[self.kDotIndex])
          self.publisher.publish_note_on(pin_index, dot_value)
        self.publisher.publish_control_change(pin_index, s[self.kMeanIndex])
