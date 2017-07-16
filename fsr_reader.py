import serial
import sys
import collections

# Pin index will be row * n_cols + col. For example: row 2 pin 14 will have index
# 2 * 16 + 14 == 46
ReaderOutput = collections.namedtuple('ReaderOutput', ['pin_index', 'value'])

class FsrReader():
  kPortName = '/dev/cu.usbmodem1421'
  kBaudRate = 9600
  kTimeoutSeconds = 1e-1

  # The FSR value will be clamped between kMinReading and kMaxReading and normalized to be within
  # [0, kMaxOutput]
  kMinReading = 50
  kMaxReading = 800
  kMaxOutput = 127

  def __init__(self):
    self.reader = serial.Serial(self.kPortName, self.kBaudRate, timeout=self.kTimeoutSeconds)

  # Reads a single message from serial and returns it.
  def read_message(self):
    raw = self.reader.readline().decode('ascii')
    stripped = raw.strip()
    elements = stripped.split(',')
    if len(elements) != 3:
      print('Could not read three elements (row, column, reading) from Arduino message.', file=sys.stderr)
      print(raw, file=sys.stderr)
      return False
    try:
      row, col, reading = map(int, elements)
      reading_clamped = min(max(0, reading - kMinReading), kMaxReading - kMinReading)
      reading_normalized = int(kMaxOutput * (reading_clamped / (kMaxReading - kMinReading)))
      pin_index = row * kNumColumns + col
      return ReaderOutput(pin_index=pin_index, reading=reading_normalized)
    except ValueError as e:
      print('Could not read one of row, col or reading to integer.', file=sys.stderr)
      print(e, file=sys.stderr)
      return False

