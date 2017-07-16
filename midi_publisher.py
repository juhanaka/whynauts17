import fsr_reader
import rtmidi_python as rtmidi

class MidiPublisher():
  kMidiPortNumber = 0
  kFirstMidiControlIndex = 28
  kStatusByte = 0xb0
  def __init__(self, reader):
    self.reader = reader
    self.midi_out = rtmidi.MidiOut()
    self.midi_out.open_port(self.kMidiPortNumber)
  def run(self):
    while True:
      msg = self.reader.read_message()
      if msg:
        midi_control_n = self.kFirstMidiControlIndex + msg.pin_index
        assert(msg.value >= 0 and msg.value < 128)
        self.midi_out.send_message([self.kStatusByte, midi_control_n, msg.value])

