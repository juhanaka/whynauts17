#define NUM_PINS    4

const int kBaudRate = 9600;
const char kDelimiter = ',';
const int kStartPin = 8;
const int kDelayMs = 50;

int values[NUM_PINS];

void setup()
{    
  for (int i = 0; i < NUM_PINS; ++i) {
    values[i] = 0;
  }
  
  Serial.begin(kBaudRate);
}

void loop() {
  for (int i = 0; i < NUM_PINS; ++i) {
    int fsr_reading = analogRead(kStartPin + i);
    int oldValue = values[i];
    if (oldValue != fsr_reading) {
      Serial.println(String(kStartPin + i) + String(kDelimiter) + String(fsr_reading));
      values[i] = fsr_reading;
    }
  }
  delay(kDelayMs);
}
