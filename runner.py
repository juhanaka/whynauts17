import fsr_reader
import running_stats
import midi_publisher
import threading
import time

class Runner():
  kDotThreshold = 0.01
  kChangeDecayThreshold = 0.002
  kMeanIndex = 0
  kDotIndex = 1
  kDotDotIndex = 2
  kChangeDecayIndex = 3
  kGetStatsOnEvery = 10
  kDotScale = 10
  kChangeDecayScale = 8
  kNumNotes = 16
  kNumChannels = 3
  kNoteOnThreshold = 0.3
  kNoteOffThreshold = 0.3


  def __init__(self):
    self.reader = fsr_reader.FsrReader()
    self.collectors = {}
    self.publisher = midi_publisher.MidiPublisher()
    self.notesOn = [[False] * self.kNumNotes] * self.kNumChannels
    self.last_decay = [0] * self.kNumNotes

  def should_publish_control(self, pin_index, mean, dot, dot_dot):
    if abs(dot) > self.kDotThreshold:
      return True
    return False

  def should_publish_change_decay(self, pin_index, value):
    old_value = self.last_decay[pin_index]
    if old_value == value:
      return False
    if abs(value - old_value) > self.kChangeDecayThreshold:
      self.last_decay[pin_index] = value
      return True
    return False

  # def is_note_on(self, pin_index, mean, dot, dot_dot):
  #   if self.notesOn[0][pin_index]:
  #     return False
  #   if mean >= self.kNoteOnThreshold and dot >= 0.02:
  #     return True
  #   return False

  # def is_note_off(self, pin_index, mean, dot, dot_dot):
  #   if not self.notesOn[0][pin_index]:
  #     return False
  #   if mean <= self.kNoteOffThreshold or self.last_decay[pin_index] < 0.05:
  #     return True
  #   return False

  def check_for_basic_note_off(self, pin_index, mean, dot, ch):
    if not self.notesOn[ch-1][pin_index]:
      return False
    if mean <= self.kNoteOffThreshold or self.last_decay[pin_index] < 0.05:
      self.notesOn[ch-1][pin_index] = False
      self.publisher.publish_note_off(pin_index, 0.5, ch)
      return True
    return False

  def check_for_activity_based_note_off(self, pin_index, change_decay, ch):
    if not self.notesOn[ch-1][pin_index]:
      return False
    if change_decay < 0.05:
      self.notesOn[ch-1][pin_index] = False
      self.publisher.publish_note_off(pin_index, 0.5, ch)
      return True
    return False

  def check_for_pressure_note_on(self, pin_index, mean, dot, ch=1):
    if self.notesOn[ch-1][pin_index]:
      return False
    if mean >= self.kNoteOnThreshold and dot >= 0.016:
      self.notesOn[ch-1][pin_index] = True
      velocity = min(1.0, self.kDotScale * dot)
      self.publisher.publish_note_on(pin_index, velocity, ch)
      return True
    return False

  def check_for_velocity_note_on(self, pin_index, mean, dot, ch=2):
    if self.notesOn[ch-1][pin_index]:
      return False
    if mean >= 0.15 and dot >= 0.06:
      self.notesOn[ch-1][pin_index] = True
      velocity = min(1.0, self.kDotScale * dot)
      self.publisher.publish_note_on(pin_index, velocity, ch)
      return True
    return False

  def check_for_activity_based_note_on(self, pin_index, change_decay, ch=3):
    if self.notesOn[ch-1][pin_index]:
      return False
    if change_decay > 0.6:
      self.notesOn[ch-1][pin_index] = True
      self.publisher.publish_note_on(pin_index, 0.7, ch)
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
        decay_value = min(1.0, self.kChangeDecayScale * s[self.kChangeDecayIndex])

        if not self.check_for_pressure_note_on(pin_index, s[self.kMeanIndex], s[self.kDotIndex], ch = 1):
          self.check_for_basic_note_off(pin_index, s[self.kMeanIndex], s[self.kDotIndex], ch = 1)

        if not self.check_for_velocity_note_on(pin_index, s[self.kMeanIndex], s[self.kDotIndex], ch = 2):
          # self.check_for_activity_based_note_off(pin_index, decay_value, ch = 2)
          self.check_for_basic_note_off(pin_index, s[self.kMeanIndex], s[self.kDotIndex], ch = 2)
        
        if not self.check_for_activity_based_note_on(pin_index, decay_value, ch = 3):
          # self.check_for_activity_based_note_off(pin_index, decay_value, ch = 3)
          self.check_for_basic_note_off(pin_index, s[self.kMeanIndex], s[self.kDotIndex], ch = 3)


        if self.should_publish_change_decay(pin_index, decay_value):
          self.publisher.publish_control_change(self.kNumNotes + pin_index, decay_value)

        if not self.should_publish_control(pin_index, s[self.kMeanIndex], s[self.kDotIndex], s[self.kDotDotIndex]):
          continue
        # if s[self.kDotIndex] > 0.0:
        #   dot_value = min(1.0, self.kDotScale * s[self.kDotIndex])
        #   self.publisher.publish_note_on(pin_index, dot_value)
        self.publisher.publish_control_change(pin_index, s[self.kMeanIndex])
