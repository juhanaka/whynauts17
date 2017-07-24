#include "circular_buffer.h"

#define THROTTLE_SERIAL               0

#define NUM_PINS                      16
#define BASE_PIN                      0
#define BUFFER_LENGTH                 20
#define MIN_DIFF_THRESHOLD            10
#define DEFAULT_PRESSURE_TRIGGER      40
#define DEFAULT_OFF_TRIGGER           30
#define BAUD_RATE                     115200
#define DELAY_MS                      12


const char kDelimiter = ',';

uint16_t m_values[NUM_PINS];
float m_weights[BUFFER_LENGTH];
CircularBufferI<uint16_t, BUFFER_LENGTH> m_value_buffers[NUM_PINS];

// Change Triggers
//static bool m_notes_on[NUM_PINS];
//static uint16_t m_on_pressure_triggers[NUM_PINS];
//static uint16_t m_off_pressure_triggers[NUM_PINS];

// MARK: - Input Processing

uint16_t factorial_sum(int n) {
  if (n <= 0) { return 0; }
  return n + factorial_sum(n - 1);
}

uint16_t movingAverage(uint16_t reading, uint16_t pin_number) {
    uint16_t tmp;
    uint16_t value = reading;
    m_value_buffers[pin_number].Pop(&tmp, 1);
    m_value_buffers[pin_number].Push(&value, 1);
    uint16_t sum = 0;
    for (int n = 0; n < BUFFER_LENGTH; ++n) {
      sum += m_value_buffers[pin_number].data_[n];
    }
    uint16_t average = sum / BUFFER_LENGTH;
    return average;
}

uint16_t weightedAverage(uint16_t reading, uint16_t pin_number) {
    uint16_t tmp;
    uint16_t value = reading;
    m_value_buffers[pin_number].Pop(&tmp, 1);
    m_value_buffers[pin_number].Push(&value, 1);
    float weightedAvg = 0;
    for (int n = 0; n < BUFFER_LENGTH; ++n) {
      weightedAvg += m_weights[n] * (float)m_value_buffers[pin_number].data_[n];
    }
    
    return (int)weightedAvg;
}


// MARK: - Serial Communication

void write_reading(int reading, int pin_number) {
  Serial.println(String(pin_number) + String(kDelimiter) + String(reading));
}


// MARK: - Board Setup

void setup()
{    
  Serial.begin(BAUD_RATE);

  // Fill state variables with sensible defaults
  for (int i = 0; i < NUM_PINS; ++i) {
    m_values[i] = 0;
//    m_notes_on[i] = false;
//    m_on_pressure_triggers[i] = DEFAULT_PRESSURE_TRIGGER;
//    m_off_pressure_triggers[i] = DEFAULT_OFF_TRIGGER;
    
    while(m_value_buffers[i].PushAvailable()) {
      m_value_buffers[i].Push(0, 1);
    }
  }

  float weight_sum = (float)factorial_sum(BUFFER_LENGTH);
  for (int i = 0; i < BUFFER_LENGTH; ++i) {
    m_weights[i] = float(i + 1) / weight_sum;
  }
  
}


// MARK: - Main Loop

void loop() {

  for (int i = 0; i < NUM_PINS; ++i) {
    
    uint16_t pin_number = BASE_PIN + i;
    uint16_t fsr_reading = analogRead(pin_number);
    
    if (THROTTLE_SERIAL) {

      // Process the input and only output changes over a minimum threshold
      
      uint16_t old_value = m_values[i];
      uint16_t new_value = weightedAverage(fsr_reading, i);
      uint16_t diff = new_value - old_value;  

      if (old_value != new_value && abs(diff) >= MIN_DIFF_THRESHOLD) {
        write_reading(new_value, pin_number);
        m_values[i] = new_value;
      }
    
    } else {

      // Write the raw FSR reading directly to serial
      write_reading(fsr_reading, pin_number);
      
    }
    

    
  }

  delay(DELAY_MS);
}
