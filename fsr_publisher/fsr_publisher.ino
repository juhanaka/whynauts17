#include <MuxShield.h>

MuxShield muxShield;
const int kBaudRate = 9600;
const char kDelimiter = ',';
const int kNumRows = 1;
const int kNumCols = 16;

int vals[kNumRows][kNumCols];

void setup()
{
    muxShield.setMode(1,ANALOG_IN);
//    muxShield.setMode(2,ANALOG_IN);
//    muxShield.setMode(3,ANALOG_IN);
    
    Serial.begin(kBaudRate);
}

void loop() {
  for (int row = 0; row < kNumRows; ++row) {
    for (int col = 0; col < kNumCols; ++col) {
      // Arduino IOs start from 1 not 0.
      vals[row][col] = muxShield.analogReadMS(row + 1, col);
      if (row == 0 && col == 1) {
        Serial.println(vals[row][col]);
      }
    }
  }
  for (int row = 0; row < kNumRows; ++row) {
    for (int col = 0; col < kNumCols; ++col) {
      //Serial.println(String(row) + String(kDelimiter) + String(col) + String(kDelimiter) + String(vals[row][col]));
    }
  }
}
