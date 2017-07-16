import fsr_reader
import midi_publisher

def main():
  reader = fsr_reader.FsrReader()
  publisher = midi_publisher.MidiPublisher(reader)

  publisher.run()

if __name__ == "__main__":
  main()
