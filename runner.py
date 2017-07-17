import fsr_reader
import running_stats
import midi_publisher
import threading
import time

class Runner():
  kDotThreshold = 0.001
  kMeanIndex = 0
  kDotIndex = 1
  kDotDotIndex = 2

  def __init__(self):
    self.reader = fsr_reader.FsrReader()
    self.collectors = {}
    self.collectors_mutex = threading.Lock()
    self.publisher = midi_publisher.MidiPublisher()

  def should_publish(self, pin_index, mean, dot, dot_dot):
    if dot > self.kDotThreshold:
      return True
    return False

  def reader_loop(self, window_length):
    while True:
      msg = self.reader.read_message()
      if not msg:
        continue
      with self.collectors_mutex:
        if not msg.pin_index in self.collectors:
          self.collectors[msg.pin_index] = running_stats.RunningStatsCollector(window_length)
        self.collectors[msg.pin_index].add(msg.value)

  def run(self, window_length, min_diff, tick_seconds):
    reader = threading.Thread(target=self.reader_loop, args=(window_length,))
    reader.daemon = True
    reader.start()

    while True:
      time.sleep(tick_seconds)
      stats = {}
      with self.collectors_mutex:
        for pin_index, collector in self.collectors.iteritems():
          stats[pin_index] = collector.get_stats()

      for pin_index, stats in stats.iteritems():
        if not self.should_publish(pin_index, stats[self.kMeanIndex], stats[self.kDotIndex], stats[self.kDotDotIndex]):
          continue
        self.publisher.publish(pin_index, self.kMeanIndex, stats[self.kMeanIndex])
        self.publisher.publish(pin_index, self.kDotIndex, stats[self.kDotIndex])
        self.publisher.publish(pin_index, self.kDotDotIndex, stats[self.kDotDotIndex])
