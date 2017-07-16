import rtmidi_python as rtmidi

class MidiPublisher():
  kMidiPortNumber = 0
  kFirstMidiControlIndex = 28
  kMaxValue = 127
  kStatusByte = 0xb0

  def __init__(self):
    self.midi_out = rtmidi.MidiOut()
    self.midi_out.open_port(self.kMidiPortNumber)

  def publish(self, fsr_index, output_index, value):
    midi_control_n = self.kFirstMidiControlIndex + fsr_index
    status = self.kStatusByte + output_index
    assert(value >= 0.0 and value <= 1.0)
    midi_value = int(value * self.kMaxValue)
    self.midi_out.send_message([status, midi_control_n, midi_value])
    print 'fsr_index: {}, output_index: {}, value: {}'.format(fsr_index, output_index, value)

