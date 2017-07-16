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
  kMinReading = 0
  kMaxReading = 1023
  kMaxOutput = 127
  kNumColumns = 16

  def __init__(self):
    self.reader = serial.Serial(self.kPortName, self.kBaudRate, timeout=self.kTimeoutSeconds)

  # Reads a single message from serial and returns it.
  def read_message(self):
    raw = self.reader.readline().decode('ascii')
    stripped = raw.strip()
    elements = stripped.split(',')
    if len(elements) != 3:
      sys.stderr.write('Could not read three elements (row, column, reading) from Arduino message.')
      sys.stderr.write(raw)
      return False
    try:
      row, col, reading = map(int, elements)
      reading_clamped = min(max(0, reading - self.kMinReading), self.kMaxReading - self.kMinReading)
      reading_normalized = int(self.kMaxOutput * (reading_clamped / float(self.kMaxReading - self.kMinReading)))
      pin_index = row * self.kNumColumns + col
      return ReaderOutput(pin_index=pin_index, value=reading_normalized)
    except ValueError as e:
      sys.stderr.write('Could not read one of row, col or reading to integer.')
      sys.stderr.write(str(e))
      return False

