#include "lights.h"

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
  initLightStrip();
}

void loop() {
  for (int i = 0; i < kNumPins; ++i) {
    int fsr_reading = analogRead(kStartPin + i);
    values[i] = smoothFsrValueTriggerTile(i,values[i],fsr_reading); // trigger light changes and retrieve the smoothed FSR value
    Serial.println(String(kStartPin + i) + String(kDelimiter) + String(fsr_reading));
  }

  drawPalettes(); // show the lights
  delay(kDelayMs);
}
