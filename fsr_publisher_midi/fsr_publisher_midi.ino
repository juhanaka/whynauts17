#include "circular_buffer.h"
#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToNote.h>


#define NUM_PINS        4
#define BUFFER_LENGTH   7

// MIDI Config
#define CHANNEL   1
#define CC_START        28

const int kBaudRate = 115200;
const int kStartPin = 8;
const int kDelayMs = 10;
const int kDiffThreshold = 8;
const int kMaxDiff = 100;
const char kDelimiter = ',';

int values[NUM_PINS];
static CircularBufferI<int, BUFFER_LENGTH> value_buffers[NUM_PINS];
static float weights[BUFFER_LENGTH];

//RingBuf buffers[NUM_PINS];


// MARK: - MIDI Methods

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}


// MARK: - Input Processing

int factorial_sum(int n) {
  if (n <= 0) { return 0; }
  return n + factorial_sum(n - 1);
}

int movingAverage(int reading, int pinNumber) {
    int tmp;
    value_buffers[pinNumber].Pop(&tmp, 1);
    value_buffers[pinNumber].Push(&reading, 1);
    int sum = 0;
    for (int n = 0; n < BUFFER_LENGTH; ++n) {
      sum += value_buffers[pinNumber].data_[n];
    }
    int average = sum / BUFFER_LENGTH;
    return average;
}

int weightedAverage(int reading, int pinNumber) {
    int tmp;
    value_buffers[pinNumber].Pop(&tmp, 1);
    value_buffers[pinNumber].Push(&reading, 1);
    float weightedAvg = 0;
    for (int n = 0; n < BUFFER_LENGTH; ++n) {
      weightedAvg += weights[n] * (float)value_buffers[pinNumber].data_[n];
    }
    
    return (int)weightedAvg;
}


// MARK: - Board Setup

void setup()
{    
  Serial.begin(kBaudRate);  
  
  for (int i = 0; i < NUM_PINS; ++i) {
    values[i] = 0;
//    buffers[i] = RingBuf_new(sizeof(int), BUFFER_LENGTH);
    while(value_buffers[i].PushAvailable()) {
      value_buffers[i].Push(0, 1);
    }
  }

  float weight_sum = (float)factorial_sum(BUFFER_LENGTH);
  float sum = 0;
  for (int i = 0; i < BUFFER_LENGTH; ++i) {
    weights[i] = float(i + 1) / weight_sum;
    sum += weights[i];
    Serial.println(weights[i]);
  }
  Serial.println(sum);
  
  
}


// MARK: - Main Loop

void loop() {
  
  for (int i = 0; i < NUM_PINS; ++i) {
    
    int fsr_reading = analogRead(kStartPin + i);
    int oldValue = values[i];

    int diff = fsr_reading - oldValue;

    // Filter out crazy big jumps
    if (abs(diff) > kMaxDiff) {
     if (diff > 0) {
       fsr_reading = oldValue + kMaxDiff;
     } else {
       fsr_reading = oldValue - kMaxDiff;
     }
    }

    int newValue = weightedAverage(fsr_reading, i);
//    int newValue = movingAverage(fsr_reading, i);
//    int newValue = fsr_reading;
    

//  Serial.println(String(kStartPin + i) + String(kDelimiter) + String(newValue));  
    
    if (oldValue != newValue && abs(diff) >= kDiffThreshold) {
      
//      Serial.println(String(kStartPin + i) + String(kDelimiter) + String(newValue));
      
      int controlValue = map(newValue, 0, 1023, 0, 128);
//      controlChange(CHANNEL, CC_START + i, controlValue);

      Serial.println(String(kStartPin + i) + String(kDelimiter) + String(controlValue));
      
      values[i] = newValue;
      
    }
  }
  
  delay(kDelayMs);
}
