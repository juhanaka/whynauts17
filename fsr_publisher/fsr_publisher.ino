const long kBaudRate = 115200;
const char kDelimiter = ',';
const int kStartPin = 0;
const int kDelayMs = 12;
const int kNumPins = 16;

int values[kNumPins];

void setup()
{
  for (int i = 0; i < kNumPins; ++i) {
    values[i] = 0;
  }

  Serial.begin(kBaudRate);
}

void loop() {
  for (int i = 0; i < kNumPins; ++i) {
    int fsr_reading = analogRead(kStartPin + i);
    int oldValue = values[i];
    if (oldValue != fsr_reading) {
      Serial.println(String(kStartPin + i) + String(kDelimiter) + String(fsr_reading));
      values[i] = fsr_reading;
    }
  }
  delay(kDelayMs);
}
