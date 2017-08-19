#ifndef __LIB_LIGHTS_H_
#define __LIB_LIGHTS_H_

#include <StandardCplusplus.h>
#include <Adafruit_NeoPixel.h>
#include <vector>

// strip configuration
#define LIGHT_PIN 10
#define INPUT_SIZE 200
const int segments = 6;
const int segmentSize = 43;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(segments * segmentSize, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

// palette definitions.  some colors that look nice together
const int numPalettes = 7;
const int paletteSize = 5;
const uint8_t palettes[numPalettes][paletteSize][3] = {
  { // Red Green Blue White Black (test palette)
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 },
    { 255, 255, 255 },
    { 0, 0, 0 }
  },
   { //vive
     { 49,51,204 },
     { 53,162,255},
     { 233,233,255 },
     { 255,164,81 },
     { 204,76,13 }
   },
   { //gecko
     { 0,171,255 },
     { 233,255,0 },
     { 0,255,78 },
     { 0,232,193 },
     { 62,232,0 }
   },
   { //richard
     { 255,202,0 },
     { 232,141,4 },
     { 255,95,14 },
     { 232,24,10 },
     { 255,4,184 }
   },
   { //drank
    { 255,9,167 },
    { 161,10,232 },
    { 62,24,255 },
    { 18,91,232 },
    { 11,216,255 }
   },
  { //moorhead
    { 255,255,227 },
    { 162,242,255 },
    { 24,21,204 },
    { 204,0,58 },
    { 255,171,123 }
  },
  { //altj
    { 254,255,17 },
    { 232,142,12 },
    { 255,0,0 },
    { 108,12,232 },
    { 13,170,255 }
  }
};

// palette painting vars
int paletteIdx = 2;
int colorCount = 1;
int targetColorCount = colorCount;
float colorDistance;
int assignedColors[16][2] = {
  {999,0},
//  {999,1},
//  {999,2},
//  {999,3}
};
float explosionPct = 0.0;
float curve;
float offset = 0.0;
float offsetLights;
float segmentOffset;
int loc1, loc2;
uint8_t r1,g1,b1,r2,g2,b2,r,g,b;
float interpPctGradient;
float brightness;
float explosion = -1.0;
float explosionOverlay;
int idxColor;
int idxSegment;

// fsr vars
const float maxFsrPercDelta = 0.1;
int newValueSmoothed;
const int tileOnThreshold = 500;
const int tileOffThreshold = 300;

// fsr vars (for debugging)
const int fsrPin = 3;
int rawFsrValue;
float fsrPerc;
float oldFsrPerc;

// turn all pixels off
void initLightStrip() {
  strip.begin();
  
  for( int k = 0; k < strip.numPixels(); k++ ) {
    strip.setPixelColor(k,strip.Color(0,0,0));
  }

  strip.show();  
}

// set single row in the light column
void setRowColor(int k, uint32_t pixelColor) {
  for( int segment = 0; segment < segments; segment++ ) {
    // strips are connected up and down the column; so every other strip is in reverse
    if ( segment % 2 == 0 ) {
      strip.setPixelColor(segment * segmentSize + k,pixelColor);
    } else {
      strip.setPixelColor(segment * segmentSize + segmentSize - 1 - k,pixelColor);
    }
  }
}

// add a color to the palette, identified by some integer
void addTileColor(int tile) {
  // find tile, if it exists
  int existingColor = -1;
  int maxColor = -1;
  for( int n = 0; n < targetColorCount; n++ ) {
    if( assignedColors[n][0] == tile ) {
      existingColor = n;
    }

    maxColor = std::max(assignedColors[n][1],maxColor); // track max color that's been assigned so far
  }

  // give up if tile already exists
  if( existingColor != -1 ) {
    return;
  }

  // assign this tile a new color (current maxColor + 1)
  assignedColors[targetColorCount][0] = tile;
  assignedColors[targetColorCount][1] = maxColor + 1;
  targetColorCount += 1;
  explosion = 0.0;  
}

// remove a color that has been added to the palette
void removeTileColor(int tile) {
  // find tile
  int existingColor = -1;
  for( int n = 0; n < targetColorCount; n++ ) {
    if( assignedColors[n][0] == tile ) {
      existingColor = n;
    }
  }

  // give up if tile isn't assigned
  if( existingColor == -1 ) {
    return;
  }

  // remove from assigned colors

  // de-initialize the color
  assignedColors[existingColor][0] = 0;
  assignedColors[existingColor][1] = 0;
  // for each row after the existing row, move it up one row
  for ( idxColor = existingColor + 1; idxColor < colorCount; idxColor++ ) {
    assignedColors[idxColor-1][0] = assignedColors[idxColor][0];
    assignedColors[idxColor-1][1] = assignedColors[idxColor][1];
  }
  // decrease total color count as we've just removed one
  targetColorCount -= 1;
  colorCount = targetColorCount;
}

// return a smoothed FSR value.
// trigger tile corresponding to tileIdx when smoothed value goes above or below the threshold
int smoothFsrValueTriggerTile(int tileIdx, int oldValue, int newValue) {
  newValueSmoothed = constrain( newValue, oldValue - maxFsrPercDelta * 1024.0, oldValue + maxFsrPercDelta * 1024.0 );

  if( oldValue < tileOnThreshold && newValueSmoothed >= tileOnThreshold ) {
    // turn on
    addTileColor(tileIdx);
  }
  else if( oldValue >= tileOffThreshold && newValueSmoothed < tileOffThreshold ) {
    // turn off
    removeTileColor(tileIdx);
  }

  return newValueSmoothed;
}

// DEBUGGING: read a single FSR's value, smooth the data, and add/remove a single tile accordingly if it's above/below a threshold
void readSmoothFsr() {
  rawFsrValue = analogRead(fsrPin);
  oldFsrPerc = fsrPerc;
  fsrPerc = constrain( rawFsrValue / 1024.0, oldFsrPerc - maxFsrPercDelta, fsrPerc + maxFsrPercDelta );

  if( oldFsrPerc < 0.5 && fsrPerc >= 0.5 ) {
    // turn on
    addTileColor(1);
  }
  else if( oldFsrPerc >= 0.3 && fsrPerc < 0.3 ) {
    // turn off
    removeTileColor(1);
  }
}

// DEBUGGING: accept on/off commands via Serial rather than process FSR data
// A,B,C,... to turn on tile A,B,C,...
// a,b,c,... to turn off tile A,BC,...
void readProcessSerialCommands() {
    if( Serial.available() > 0 ) {
    byte size = Serial.read();

    Serial.println("Read byte");
    Serial.println(size);
    Serial.println("As integer:");
    Serial.println(int(size));

    Serial.println("Current Color count");
    Serial.println(colorCount);
    Serial.println("Current Target Color count");
    Serial.println(targetColorCount);

    if( int(size) >= 97 ) {
      Serial.println("remove tile color for:");
      Serial.println(int(size) - 32);
      removeTileColor(int(size) - 32);
    }
    else {
      Serial.println("adding tile color for:");
      Serial.println(int(size));
      addTileColor(int(size));
    }

    Serial.println("Color count");
    Serial.println(colorCount);
    Serial.println("Target Color count");
    Serial.println(targetColorCount);
  }
}

// set pixels on the strip
void drawPalettes() {
  colorDistance = segmentSize / float(colorCount);
  offsetLights = offset * segmentSize;

  // if we're transitioning, progress the "explosion" (i.e. a fade to white before settling into the new state)
  if( explosion >= 0.0 && explosion < 1.0 ) {
    explosionOverlay = -0.5 * cos(2 * PI * (1-explosion)*(1-explosion)) + 0.5; // use a skewed cosine for the shape of the transition

    // when the explosion is at its peak, add the new color
    if( explosion > 0.29 && targetColorCount > colorCount ){
      colorCount = targetColorCount;
      colorDistance = segmentSize / float(colorCount);
    }
    
    explosion += 0.07; // progress the explosion
  }
  else {
    explosionOverlay = 0.0;
  }

  // set each row
  for( idxSegment = 0; idxSegment < segmentSize; idxSegment++ ) {
    
    // for this row, find the two palette colors it lies between
    segmentOffset = fmod(idxSegment + offsetLights,segmentSize);
    loc1 = floor(segmentOffset / colorDistance);
    loc2 = fmod( floor(segmentOffset / colorDistance) + 1, colorCount );

    r1 = palettes[paletteIdx][assignedColors[loc1][1] % paletteSize][0];
    g1 = palettes[paletteIdx][assignedColors[loc1][1] % paletteSize][1];
    b1 = palettes[paletteIdx][assignedColors[loc1][1] % paletteSize][2];

    r2 = palettes[paletteIdx][assignedColors[loc2][1] % paletteSize][0];
    g2 = palettes[paletteIdx][assignedColors[loc2][1] % paletteSize][1];
    b2 = palettes[paletteIdx][assignedColors[loc2][1] % paletteSize][2];

    // interpolate between the two palette colors
    interpPctGradient = ( segmentOffset - loc1 * colorDistance ) / colorDistance;
    brightness = 0.4 * cos(2 * PI * interpPctGradient ) + 0.6;

    r = ((r2 - r1)*interpPctGradient + r1)*brightness;
    g = ((g2 - g1)*interpPctGradient + g1)*brightness;
    b = ((b2 - b1)*interpPctGradient + b1)*brightness;

    // add the explosion (i.e. interpolate between target color and white, based on the explosionOverlay, which is between 0 and 1.0)
    r = ( 255.0 - r )*explosionOverlay + r;
    g = ( 255.0 - g )*explosionOverlay + g;
    b = ( 255.0 - b )*explosionOverlay + b;

    // set the color
    setRowColor(idxSegment,strip.Color(r,g,b));
  }
  
  strip.show();

  offset += 0.01; // move the light pulse along the strip
}

#endif // __LIB_LIGHTS_H_
