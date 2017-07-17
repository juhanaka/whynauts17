const int kBaudRate = 9600;
const char kDelimiter = ',';
const int kNumPins = 4;
const int kDelayMs = 5;

void setup()
{    
  Serial.begin(kBaudRate);
}

void loop() {
  for (int i = 0; i < kNumPins; ++i) {
    int fsr_reading = analogRead(i);
    Serial.println(String(i) + String(kDelimiter) + String(fsr_reading));
  }
  delay(kDelayMs);
}
