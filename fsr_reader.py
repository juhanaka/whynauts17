import serial
import sys
import collections

ReaderOutput = collections.namedtuple('ReaderOutput', ['pin_index', 'value'])

class FsrReader():
  kPortName = '/dev/cu.usbmodem1421'
  kBaudRate = 9600
  kTimeoutSeconds = 1e-1

  # The FSR value will be clamped between kMinReading and kMaxReading and normalized to be within
  # [0, 1.0]
  kMinReading = 0
  kMaxReading = 1023

  def __init__(self):
    self.reader = serial.Serial(self.kPortName, self.kBaudRate, timeout=self.kTimeoutSeconds)

  # Reads a single message from serial and returns it.
  def read_message(self):
    raw = self.reader.readline().decode('ascii')
    stripped = raw.strip()
    elements = stripped.split(',')
    if len(elements) != 2:
      sys.stderr.write('Could not read two elements (pin index, reading) from Arduino message.\n')
      sys.stderr.write(raw + '\n')
      return False
    try:
      pin_index, reading = map(int, elements)
      reading_clamped = min(max(0, reading - self.kMinReading), self.kMaxReading - self.kMinReading)
      reading_normalized = reading_clamped / float(self.kMaxReading - self.kMinReading)
      return ReaderOutput(pin_index=pin_index, value=reading_normalized)
    except ValueError as e:
      sys.stderr.write('Could not read one of row, col or reading to integer.')
      sys.stderr.write(str(e))
      return False

