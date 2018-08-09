/* README
 *  - Use Arduino Uno to view Serial messages for debugging
 *  - Use the board manager to ad adafruit if they don't exist
 *  - If Adafruit not available in Board Manager, add URL to preferences on additional board locations
 *  - Arduino is C++. Easier to search for C++ solutions
 * Trinket: 
 *   Board - Trinket 5V/16Hz (USB)
 *   Programmer - USBTinyISP
 * 
 * Links:
 * https://github.com/FastLED/FastLED/wiki/Pixel-reference
*/

#include <FastLED.h>
#include "GogglePattern.h"
#include <IRremote.h>

#define PIN_LEDS 10
#define PIN_REMOTE 6
#define LEDS 40

CRGB leds[LEDS];
GogglePattern goggles = GogglePattern(LEDS, leds);

IRrecv irrecv(PIN_REMOTE);
decode_results results;

bool LightsActive = true;

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");

  irrecv.enableIRIn(); // Start the receiver
  
  FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, LEDS);
  FastLED.show();
  goggles.Init(NORMAL, 255);

  test();
}

void loop() {
  if (irrecv.decode(&results))
  {
   Serial.println(results.value, HEX);
   remoteCommand();
   irrecv.resume(); // Receive the next value
  }
  if (LightsActive) {
    goggles.Update();
  }
}

void remoteCommand() {
  if (results.value == 0xC || results.value == 0x80C) { // -PWR-
    Serial.println("Power");
    LightsActive = !LightsActive;
    if(!LightsActive) { goggles.ClearLights(); }
    if (LightsActive) { Serial.println("ON"); } else { Serial.println("OFF"); }
  }
  else if (results.value == 0xD || results.value == 0x80D) { // -Mute/Unmute- (only unlocks because choosing a pattern will lock it)
    Serial.println("Rotate Pattern");
    goggles.UnlockPattern();
  }
  else if (results.value == 0x20 || results.value == 0x820) { // -CH+-
    Serial.println("NextPattern");
    goggles.NextPattern();
  }
  else if (results.value == 0x21 || results.value == 0x821) { // -CH--
    Serial.println("PreviousPattern");
    goggles.PreviousPattern();
  }
  else if (results.value == 0x1 || results.value == 0x801) { // -1-
    Serial.println("Rainbow");
    goggles.LockPattern();
    goggles.SetRainbow();
  }
  else if (results.value == 0x2 || results.value == 0x802) { // -2-
    Serial.println("Color Wipe");
    goggles.LockPattern();
    goggles.SetColorWipe();
  }
  else if (results.value == 0x3 || results.value == 0x803) { // -3-
    Serial.println("Theater Chase");
    goggles.LockPattern();
    goggles.SetTheaterChase();
  }
  else if (results.value == 0x4 || results.value == 0x804) { // -4-
    Serial.println("Loopy");
    goggles.LockPattern();
    goggles.SetLoopy();
  } 
  else if (results.value == 0x5 || results.value == 0x805) { // -5-
    Serial.println("Wave");
    goggles.LockPattern();
    goggles.SetWave();
  }
  else if (results.value == 0x6 || results.value == 0x806) { // -6-
    Serial.println("Clap");
    goggles.LockPattern();
    goggles.SetClap();
  }
  else if (results.value == 0x7 || results.value == 0x807) { // -7-
    Serial.println("Party");
    goggles.LockPattern();
    goggles.SetParty();
  }
}

void test() {
//  Serial.println("Entering test mode");
  // Test Patterns
//  goggles.LockPattern();

  // Test methods
//  goggles.SetRainbow();
//  goggles.SetColorWipe();
//  goggles.SetTheaterChase();
//  goggles.SetLoopy();
//  goggles.SetWave();
//    goggles.SetClap();
//  goggles.SetParty();
}

