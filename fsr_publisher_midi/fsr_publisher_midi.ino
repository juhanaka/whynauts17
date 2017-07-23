#include "circular_buffer.h"
#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToNote.h>


#define NUM_PINS                      4
#define BUFFER_LENGTH                 20
#define DEFAULT_PRESSURE_TRIGGER      40;
#define DEFAULT_OFF_TRIGGER           30;

// MIDI Config
#define CHANNEL         1
#define CC_BASE         24
#define PITCH_BASE      52


const int kBaudRate = 115200;
const int kStartPin = 8;
const int kDelayMs = 10;
const uint16_t kDiffThreshold = 10;
const uint16_t kMaxDiff = 80;
const char kDelimiter = ',';

uint16_t m_values[NUM_PINS];
static bool m_notes_on[NUM_PINS];
static uint16_t m_on_pressure_triggers[NUM_PINS];
static uint16_t m_off_pressure_triggers[NUM_PINS];
static float m_weights[BUFFER_LENGTH];
static CircularBufferI<uint16_t, BUFFER_LENGTH> m_value_buffers[NUM_PINS];

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
//  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
//  MidiUSB.flush();
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
//  MidiUSB.flush();
}


// MARK: - Input Processing

uint16_t factorial_sum(int n) {
  if (n <= 0) { return 0; }
  return n + factorial_sum(n - 1);
}

uint16_t movingAverage(uint16_t reading, uint16_t pinNumber) {
    uint16_t tmp;
    uint16_t value = reading;
    m_value_buffers[pinNumber].Pop(&tmp, 1);
    m_value_buffers[pinNumber].Push(&value, 1);
    uint16_t sum = 0;
    for (int n = 0; n < BUFFER_LENGTH; ++n) {
      sum += m_value_buffers[pinNumber].data_[n];
    }
    uint16_t average = sum / BUFFER_LENGTH;
    return average;
}

uint16_t weightedAverage(uint16_t reading, uint16_t pinNumber) {
    uint16_t tmp;
    uint16_t value = reading;
    m_value_buffers[pinNumber].Pop(&tmp, 1);
    m_value_buffers[pinNumber].Push(&value, 1);
    float weightedAvg = 0;
    for (int n = 0; n < BUFFER_LENGTH; ++n) {
      weightedAvg += m_weights[n] * (float)m_value_buffers[pinNumber].data_[n];
    }
    
    return (int)weightedAvg;
}


// MARK: - Board Setup

void setup()
{    
  Serial.begin(kBaudRate);  

  // Fill state variables with sensible defaults
  for (int i = 0; i < NUM_PINS; ++i) {
    m_values[i] = 0;
    m_notes_on[i] = false;
    m_on_pressure_triggers[i] = DEFAULT_PRESSURE_TRIGGER;
    m_off_pressure_triggers[i] = DEFAULT_OFF_TRIGGER;
    
    while(m_value_buffers[i].PushAvailable()) {
      m_value_buffers[i].Push(0, 1);
    }
    
  }


  float weight_sum = (float)factorial_sum(BUFFER_LENGTH);
  float sum = 0;
  for (int i = 0; i < BUFFER_LENGTH; ++i) {
    m_weights[i] = float(i + 1) / weight_sum;
    sum += m_weights[i];
    Serial.println(m_weights[i]);
  }
  Serial.println(sum);
  
  
}


// MARK: - Main Loop

void loop() {
  
  for (int i = 0; i < NUM_PINS; ++i) {
    
    uint16_t fsr_reading = analogRead(kStartPin + i);
    uint16_t oldValue = m_values[i];

    // Filter out crazy big jumps
//    if (abs(diff) > kMaxDiff) {
//     if (diff > 0) {
//       fsr_reading = oldValue + kMaxDiff;
//     } else {
//       fsr_reading = oldValue - kMaxDiff;
//     }
//    }

    uint16_t newValue = weightedAverage(fsr_reading, i);
    uint16_t diff = newValue - oldValue;
//    int newValue = movingAverage(fsr_reading, i);
//    int newValue = fsr_reading;
    

//  Serial.println(String(kStartPin + i) + String(kDelimiter) + String(newValue));  
    
    if (oldValue != newValue && abs(diff) >= kDiffThreshold) {
      
//      Serial.println(String(kStartPin + i) + String(kDelimiter) + String(newValue));
      
      int controlValue = map(newValue, 0, 1023, 0, 128);

      if (m_notes_on[i] && controlValue <= m_off_pressure_triggers[i]) {
        m_notes_on[i] = false;
        noteOff(CHANNEL, PITCH_BASE + i, 64);
//        Serial.print("Note Off: ");
//        Serial.println(i + PITCH_BASE);
        
      } else if (!m_notes_on[i] && controlValue >= m_on_pressure_triggers[i]) {
        m_notes_on[i] = true;
        noteOn(CHANNEL, PITCH_BASE + i, 64);
//        Serial.print("Note On:  ");
//        Serial.println(i + PITCH_BASE);
      }

      controlChange(CHANNEL, CC_BASE + i, controlValue);
      MidiUSB.flush();

//      Serial.println(String(kStartPin + i) + String(kDelimiter) + String(controlValue));
      
      m_values[i] = newValue;
      
    }
  }

  delay(kDelayMs);
}
