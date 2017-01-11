/*
    Arduino LED strip - Controls LED strip using Arduino

    To the extent possible under law, the author(s) have dedicated all copyright and related
    and neighboring rights to this software to the public domain worldwide.
    This software is distributed without any warranty.

    You should have received a copy of the CC0 Public Domain Dedication along with this software.
    If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

*/

// Install FastLED library using Library manager
#include <FastLED.h>

#define DEBUG 0

#define NUM_LEDS 100
// Define the array of leds
CRGB leds[NUM_LEDS];

// Arduino pins
#define STRIP_PIN 6     // PIN for sending data to LED strip
#define INTERRUPT_PIN 2 // Mode switch button
#define INDICATOR_PIN 3 // LED diode for start-up and mode switch indication

enum all_modes {
  off,
  solid_white,
  solid_red,
  solid_blue,
  solid_purple,
  moving_bars,
  fade,
  fire,
  twinkle,
  twinkle_random,
  twinkle_single,
  snow_sparkle,
  // Disabled:
  one_at_a_time,
  bars,
  strobe,
  solid_green,
  solid_yellow,
  sparkle,
  // Last
  last = snow_sparkle
};

volatile all_modes run_mode = off;  // LED effects mode setting
long ttime = 0;                     // the last time the output pin was toggled
long debounce = 200;                // the debounce time

// Setup
void setup() {
  // indicator pin on:
  pinMode(INDICATOR_PIN, OUTPUT);
  digitalWrite(INDICATOR_PIN, HIGH);
  // Setup console
  if (DEBUG) {
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.println();
  }
  // LED strip
  FastLED.addLeds<NEOPIXEL, STRIP_PIN>(leds, NUM_LEDS);
  // Mode switcher
  pinMode(INTERRUPT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), changeMode, CHANGE);
  digitalWrite(INDICATOR_PIN, LOW);
}

// Change mode interrupt button
void changeMode() {
  volatile byte state = digitalRead(INTERRUPT_PIN);
  if (state == HIGH && millis() - ttime > debounce) {
    if (run_mode == last) {
      run_mode = all_modes(0);
    } else {
      run_mode = all_modes(run_mode + 1);
    }
    if (DEBUG) {
      Serial.print("Mode changed: ");
      Serial.println(run_mode);
    }
    ttime = millis();
  }
  digitalWrite(INDICATOR_PIN, state);
}

// Make whole LED strip in single color
void runSingleColor(CRGB color) {
  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    leds[i] = color;
  }
  FastLED.show();
  delay(200);
}

// Loop red, green or blue single LED
void runOneAtATime() {
  // one at a time
  for (int j = 0; j < 3; j++) {
    for (int i = 0 ; i < NUM_LEDS; i++ ) {
      if (run_mode != one_at_a_time) return;
      memset(leds, 0, NUM_LEDS * 3);
      switch (j) {
        case 0: leds[i].r = 255; break;
        case 1: leds[i].g = 255; break;
        case 2: leds[i].b = 255; break;
      }
      FastLED.show();
      delay(15);
    }
  }
}

// Bar effect in red, green or blue color
void runBars() {
  for (int j = 0; j < 3; j++) {
    memset(leds, 0, NUM_LEDS * 3);
    for (int i = 0 ; i < NUM_LEDS; i++ ) {
      if (run_mode != bars) return;
      switch (j) {
        case 0: leds[i].r = 255; break;
        case 1: leds[i].g = 255; break;
        case 2: leds[i].b = 255; break;
      }
      FastLED.show();
      delay(10);
    }
    for (int i = NUM_LEDS - 1 ; i >= 0; i-- ) {
      if (run_mode != bars) return;
      switch (j) {
        case 0: leds[i].r = 0; break;
        case 1: leds[i].g = 0; break;
        case 2: leds[i].b = 0; break;
      }
      FastSPI_LED.show();
      delay(1);
    }
  }
}

// Fade in and fade out in red, green or blue color
void runFade() {
  for (int j = 0; j < 1; j++ ) { // only red
    memset(leds, 0, NUM_LEDS * 3);
    for (int k = 0; k < 256; k++) {
      for (int i = 0; i < NUM_LEDS; i++ ) {
        if (run_mode != fade) return;
        switch (j) {
          case 0: leds[i].r = k; break;
          case 1: leds[i].g = k; break;
          case 2: leds[i].b = k; break;
        }
      }
      FastLED.show();
      delay(3);
    }
    for (int k = 255; k >= 0; k--) {
      for (int i = 0; i < NUM_LEDS; i++ ) {
        if (run_mode != fade) return;
        switch (j) {
          case 0: leds[i].r = k; break;
          case 1: leds[i].g = k; break;
          case 2: leds[i].b = k; break;
        }
      }
      FastLED.show();
      delay(3);
    }
  }
}

// "Fire" effect, makes LED strip bright Yellow/Red with some
// flickering color deviations every now and than
void runFire() {
  int r = 255;
  int g = r - 40;
  int b = 40;
  memset(leds, 0, NUM_LEDS * 3);
  if (run_mode != fire) return;
  for (int x = 0; x < NUM_LEDS; x++) {
    int flicker = random(0, 150);
    int r1 = r - flicker;
    int g1 = g - flicker;
    int b1 = b - flicker;
    if (g1 < 0) g1 = 0;
    if (r1 < 0) r1 = 0;
    if (b1 < 0) b1 = 0;
    leds[x].r = r1;
    leds[x].g = g1;
    leds[x].b = b1;
  }
  FastLED.show();
  delay(random(50, 150));
}

// Run "strobe" effect
void runStrobe() {
  for (int i = 0; i <= random(2, 6); i++) {
    memset(leds, 120, NUM_LEDS * 3);
    FastLED.show();
    delay(50);
    memset(leds, 0, NUM_LEDS * 3);
    FastLED.show();
    delay(50);
  }
  if (run_mode != strobe) return;
  delay(1000);
}

// "twinkle" effect.
// single LED or 40 leds, random color or red
void runTwinkle(boolean single_led, boolean random_color) {
  int amount = 1;
  int temp_leds[40];
  amount = (single_led ? 1 : 40);
  memset(leds, 0, NUM_LEDS * 3);
  memset(temp_leds, 0, 40 * 2);
  while (true) {
    for (int i = 0; i < amount; i++) {
      leds[temp_leds[i]] = CRGB::Black;
      int random_led = random(NUM_LEDS);
      temp_leds[i] = random_led;
      if (random_color) {
        leds[random_led] = CHSV(random(0, 255), random(200, 255), random(0, 255)); // I prefer high saturation
      } else {
        leds[random_led] = CRGB::Red;
      }
      FastLED.show();
      delay(110);
      if (random_color) {
        if (run_mode != twinkle_random) return;
      } else {
        if (run_mode != twinkle && single_led == false) return;
        if (run_mode != twinkle_single && single_led == true) return;
      }
    }
    delay(40);
  }
}

// randomly set one LED white and than turn it off
void runSparkle() {
  int led = random(NUM_LEDS);
  leds[led] = CRGB::White;
  FastLED.show();
  leds[led] = CRGB::Black;
  delay(5);
}

// turn whole strip almost full white (0x101010)
// randomly turn one led to full white and than turn it back
// to original color
void runSnowSparkle() {
  memset(leds, 0x10, NUM_LEDS * 3);
  int led = random(NUM_LEDS);
  leds[led] = CRGB::White;
  FastLED.show();
  leds[led] = 0x101010;
  FastLED.show();
  delay(200);
}

// Moving bars
void runMovingBars() {
  boolean on = false;
  int start = 0;
  while (true) {
    memset(leds, 0x00, NUM_LEDS * 3);
    for (int i = (0-start); i < (NUM_LEDS-start); i++ ) {
      if (run_mode != moving_bars) return;
      if (i % 25 == 0) on = !on;
      if (on) leds[i + start] = CRGB::DarkViolet;
    }
    FastLED.show();
    start++;
    if (start == 25) { 
      start = 0;
      on = !on;
    }
    delay(10);
  }
}

// Main loop
void loop() {
  switch (run_mode) {
    // solid colors
    case off: runSingleColor(CRGB::Black); break;
    case solid_white: runSingleColor(CRGB::White); break;
    case solid_red: runSingleColor(CRGB::Red); break;
    case solid_blue: runSingleColor(CRGB::Blue); break;
    case solid_purple: runSingleColor(CRGB::Purple); break;
    case solid_green: runSingleColor(CRGB::Green); break;
    case solid_yellow: runSingleColor(CRGB::Yellow); break;
    // effects
    case one_at_a_time: runOneAtATime(); break;
    case bars: runBars(); break;
    case moving_bars: runMovingBars(); break;
    case fade: runFade(); break;
    case fire: runFire(); break;
    case strobe: runStrobe(); break;
    case twinkle: runTwinkle(false, false); break;
    case twinkle_random: runTwinkle(false, true); break;
    case twinkle_single: runTwinkle(true, false); break;
    case sparkle: runSparkle(); break;
    case snow_sparkle: runSnowSparkle(); break;
  }
}
