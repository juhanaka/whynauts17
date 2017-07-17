import rtmidi_python as rtmidi

class MidiPublisher():
  kMidiPortNumber = 0
  kFirstMidiControlIndex = 28
  kFirstMidiNoteIndex = 0
  kMaxValue = 127
  kStatusByteControl = 0xb0
  kStatusByteNote = 0x90

  def __init__(self):
    self.midi_out = rtmidi.MidiOut()
    self.midi_out.open_port(self.kMidiPortNumber)

  def _to_midi_value(self, value):
    assert(value >= 0.0 and value <= 1.0)
    return int(value * self.kMaxValue)

  def publish_note_on(self, fsr_index, note_velocity):
    note_number = self.kFirstMidiNoteIndex + fsr_index
    midi_value = self._to_midi_value(note_velocity)
    self.midi_out.send_message([self.kStatusByteNote, note_number, midi_value])
    print 'Note: {}, velocity: {}'.format(note_number, midi_value)

  def publish_control_change(self, fsr_index, value):
    midi_control_n = self.kFirstMidiControlIndex + fsr_index
    midi_value = self._to_midi_value(value)
    self.midi_out.send_message([self.kStatusByteControl, midi_control_n, midi_value])
    print 'Control number: {}, value: {}'.format(midi_control_n, midi_value)


