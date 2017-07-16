import fsr_reader
import running_stats
import midi_publisher
import threading
import time

class Runner():
  def __init__(self):
    self.reader = fsr_reader.FsrReader()
    self.collectors = {}
    self.collectors_mutex = threading.Lock()
    self.previous_stats = {}
    self.publisher = midi_publisher.MidiPublisher()

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
    reader.start()

    while True:
      time.sleep(tick_seconds)
      stats = {}
      with self.collectors_mutex:
        for pin_index, collector in self.collectors.iteritems():
          stats[pin_index] = collector.get_stats()

      for pin_index, stats in stats.iteritems():
        for i, stat in enumerate(stats):
          should_publish = True
          if (pin_index, i) in self.previous_stats:
            previous_stat = self.previous_stats[(pin_index, i)]
            if abs(previous_stat - stat) < min_diff:
              should_publish = False
          if should_publish:
            self.publisher.publish(pin_index, i, stat)
          self.previous_stats[(pin_index, i)] = stat
    reader.join()
