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
#include "StudioLightingPattern.h"
#include <IRremote.h>

#define PIN_LEDS 10
#define PIN_REMOTE 6
#define LEDS 65

CRGB leds[LEDS];
StudioLightingPattern lighting = StudioLightingPattern(LEDS, leds);

IRrecv irrecv(PIN_REMOTE);
decode_results results;

bool LightsActive = true;

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP");

  irrecv.enableIRIn(); // Start the receiver
  
  FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, LEDS);
  FastLED.show();
  lighting.Init(NORMAL, 255);

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
    lighting.Update();
  }
}

void remoteCommand() {
  if (results.value == 0xC || results.value == 0x80C) { // -PWR-
    Serial.println("Power");
    LightsActive = !LightsActive;
    if(!LightsActive) { lighting.ClearLights(); }
    if (LightsActive) { Serial.println("ON"); } else { Serial.println("OFF"); }
  }
  else if (results.value == 0xD || results.value == 0x80D) { // -Mute/Unmute- (only unlocks because choosing a pattern will lock it)
    Serial.println("Rotate Pattern");
    lighting.UnlockPattern();
  }
  else if (results.value == 0x20 || results.value == 0x820) { // -CH+-
    Serial.println("NextPattern");
    lighting.NextPattern();
  }
  else if (results.value == 0x21 || results.value == 0x821) { // -CH--
    Serial.println("PreviousPattern");
    lighting.PreviousPattern();
  }
  else if (results.value == 0x1 || results.value == 0x801) { // -1-
    Serial.println("Rainbow");
    lighting.LockPattern();
    lighting.SetRainbow();
  }
  else if (results.value == 0x2 || results.value == 0x802) { // -2-
    Serial.println("Color Wipe");
    lighting.LockPattern();
    lighting.SetColorWipe();
  }
  else if (results.value == 0x3 || results.value == 0x803) { // -3-
    Serial.println("Theater Chase");
    lighting.LockPattern();
    lighting.SetTheaterChase();
  }
  else if (results.value == 0x4 || results.value == 0x804) { // -4-
    Serial.println("Slow Fade");
    lighting.LockPattern();
    lighting.SetSlowFade();
  } 
  else if (results.value == 0x5 || results.value == 0x805) { // -5-
    Serial.println("Wave");
    lighting.LockPattern();
    lighting.SetWave();
  }
  else if (results.value == 0x6 || results.value == 0x806) { // -6-
//    Serial.println("Clap");
    lighting.LockPattern();
//    lighting.SetClap();
  }
  else if (results.value == 0x7 || results.value == 0x807) { // -7-
    Serial.println("Party");
    lighting.LockPattern();
    lighting.SetParty();
  }
}

void test() {
//  Serial.println("Entering test mode");
  // Test Patterns
  lighting.LockPattern();

  // Test methods
//  lighting.SetRainbow();
//  lighting.SetColorWipe();
//  lighting.SetTheaterChase();
//  lighting.SetWave();
//  lighting.SetParty();
  lighting.SetSlowFade();
}

