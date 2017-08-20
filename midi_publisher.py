import rtmidi_python as rtmidi

kNoteMap = {2: 48, 3: 50, 14: 52, 15: 53, 12: 55, 13: 57, 1: 59, 0: 60, 11: 62, 8: 64, 5: 65, 4: 67, 7: 69, 6: 71, 9: 72, 10: 74}
 
class MidiPublisher():
  kMidiPortNumber = 0
  kFirstMidiControlIndex = 20
  kMaxValue = 127
  kStatusByteControl = 0xb0
  kStatusByteNoteOn = 0x90
  kStatusByteNoteOff = 0x80

  def __init__(self):
    print("initialized MIDI")
    self.midi_out = rtmidi.MidiOut()
    self.midi_out.open_port(self.kMidiPortNumber)
    self.channel = 1

  def _to_midi_value(self, value):
    assert(value >= 0.0 and value <= 1.0)
    return int(value * self.kMaxValue)

  def publish_note_on(self, fsr_index, note_velocity, ch=None):
    note_number = kNoteMap[fsr_index]
    midi_value = self._to_midi_value(note_velocity)
    cmd = (self.kStatusByteNoteOn & 0xf0) | ((ch if ch else self.channel) - 1 & 0xf)
    self.midi_out.send_message([cmd, note_number, midi_value])
    # print 'Note On: ', note_number, ', Velocity: ', midi_value

  def publish_note_off(self, fsr_index, note_velocity, ch=None):
    note_number = kNoteMap[fsr_index]
    midi_value = self._to_midi_value(note_velocity)
    cmd = (self.kStatusByteNoteOff & 0xf0) | ((ch if ch else self.channel) - 1 & 0xf)
    self.midi_out.send_message([cmd, note_number, midi_value])
    # print 'Note Off: ', note_number, ', Velocity: ', midi_value

  def publish_control_change(self, fsr_index, value, ch=None):
    midi_control_n = self.kFirstMidiControlIndex + fsr_index
    midi_value = self._to_midi_value(value)
    cmd = (self.kStatusByteControl & 0xf0) | ((ch if ch else self.channel) - 1 & 0xf)
    self.midi_out.send_message([cmd, midi_control_n, midi_value])
    # print 'Control number: ', midi_control_n, ', Value: ', midi_value


