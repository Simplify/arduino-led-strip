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

#define NUM_LEDS 100
// Define the array of leds
CRGBArray<NUM_LEDS> leds;//[NUM_LEDS];

// Arduino pins
#define STRIP_PIN 6     // PIN for sending data to LED strip
#define INTERRUPT_PIN 2 // Mode switch button
#define INDICATOR_PIN 3 // LED diode for start-up and mode switch indication

enum all_modes {
  off,
  solid_white,
  first_half,
  second_half,
  solid_red,
  solid_blue,
  solid_purple,
  moving_bars,
  fade,
  fire,
  twinkle,
  twinkle_random,
  snow_sparkle,
  strobe,
  rainbow_cycle,
  rainbow_loop,
  // Disabled:
  solid_green,
  solid_yellow,
  twinkle_single,
  one_at_a_time,
  bars,
  // Last
  last = rainbow_loop
};

volatile all_modes run_mode = off;  // LED effects mode setting
long ttime = 0;                     // the last time the output pin was toggled
long debounce = 200;                // the debounce time

// Setup
void setup() {
  // indicator pin on:
  pinMode(INDICATOR_PIN, OUTPUT);
  digitalWrite(INDICATOR_PIN, HIGH);
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
    ttime = millis();
  }
  digitalWrite(INDICATOR_PIN, state);
}

// Make whole LED strip in single color
void runSingleColor(CRGB color) {
  FastLED.showColor(color);
  delay(200);
}

void runFirstHalf() {
  memset(leds, 0, NUM_LEDS * 3);
  leds(10, 39) = CRGB::White;
  FastLED.show();
  while (true) {
    delay(30);
    if (run_mode != first_half) return;
  }
}

void runSecondHalf() {
  memset(leds, 0, NUM_LEDS * 3);
  leds(60, 89) = CRGB::White;
  FastLED.show();
  while (true) {
    delay(30);
    if (run_mode != second_half) return;
  }
}

// Loop random color, one LED at the time
void runOneAtATime() {
  while (true) {
    CHSV color = CHSV(random(0, 255), random(200, 255), random(200, 255));
    for (int i = 0 ; i < NUM_LEDS; i++ ) {
      if (run_mode != one_at_a_time) return;
      memset(leds, 0, NUM_LEDS * 3);
      leds[i] = color;
      FastLED.show();
      delay(5);
    }
  }
}

// Bar effect in random color
void runBars() {
  while (true) {
    CHSV color = CHSV(random(0, 255), random(200, 255), random(200, 255));
    memset(leds, 0, NUM_LEDS * 3);
    for (int i = 0 ; i < NUM_LEDS; i++ ) {
      if (run_mode != bars) return;
      leds[i] = color;
      FastLED.show();
      delay(7);
    }
    for (int i = NUM_LEDS - 1 ; i >= 0; i-- ) {
      if (run_mode != bars) return;
      leds[i] = CRGB::Black;
      FastSPI_LED.show();
      delay(2);
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
  while (true) {
    CHSV color = CHSV(random(0, 255), random(200, 255), random(200, 255));
    for (int i = 0; i <= random(2, 6); i++) {
      FastLED.showColor(color);
      delay(75);
      FastLED.showColor(CRGB::Black);
      delay(75);
    }
    if (run_mode != strobe) return;
    delay(1000);
  }
}

// "twinkle" effect.
void runTwinkle(int max_leds, boolean random_color) {
  int amount = 1;
  int temp_leds[max_leds];
  memset(leds, 0, NUM_LEDS * 3);
  memset(temp_leds, 0, max_leds * 2);
  while (true) {
    for (int i = 0; i < max_leds; i++) {
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
        if (run_mode != twinkle && max_leds > 1) return;
        if (run_mode != twinkle_single && max_leds == 1) return;
      }
    }
    delay(40);
  }
}

// turn whole strip almost full white (0x101010)
// randomly turn one led to full white and than turn it back
// to original color
void runSnowSparkle() {
  memset(leds, 0x10, NUM_LEDS * 3);
  while (true) {
    if (run_mode != snow_sparkle) return;
    int led = random(NUM_LEDS);
    leds[led] = CRGB::White;
    FastLED.show();
    leds[led] = 0x101010;
    FastLED.show();
    delay(200);
  }
}

// Cycle rainbow colors on whole strip
void runRainbowCycle() {
  while (true) {
    for (uint8_t hue = 0; hue < 256; hue++) {
      if (run_mode != rainbow_cycle) return;
      FastLED.showColor(CHSV(hue, 255, 255));
      delay(10);
    }
  }
}

// Loop rainbow colors on whole strip
void runRainbowLoop() {
  uint8_t hue = 0;
  while (true) {
    if (run_mode != rainbow_loop) return;
    for (int i = 0 ; i < NUM_LEDS; i++ ) {
      leds[i] = CHSV((hue + i) % 256, 255, 255);
    }
    FastLED.show();
    delay(3);
    hue++;
    if (hue > 255) hue = 0;
  }
}

// Moving bars
void runMovingBars() {
  boolean on = false;
  int start = 0;
  uint8_t hue = 0;
  while (true) {
    CHSV color = CHSV(hue, 255, 255);
    memset(leds, 0x00, NUM_LEDS * 3);
    for (int i = (0 - start); i < (NUM_LEDS - start); i++ ) {
      if (run_mode != moving_bars) return;
      if (i % 25 == 0) on = !on;
      if (on) leds[i + start] = color;
    }
    FastLED.show();
    start++;
    if (start == 25) {
      start = 0;
      on = !on;
    }
    hue++;
    if (hue > 255) hue = 0;
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
    // "halfs"
    case first_half: runFirstHalf(); break;
    case second_half: runSecondHalf(); break;
    // effects
    case one_at_a_time: runOneAtATime(); break;
    case bars: runBars(); break;
    case rainbow_cycle: runRainbowCycle(); break;
    case rainbow_loop: runRainbowLoop(); break;
    case moving_bars: runMovingBars(); break;
    case fade: runFade(); break;
    case fire: runFire(); break;
    case strobe: runStrobe(); break;
    case twinkle: runTwinkle(30, false); break;
    case twinkle_random: runTwinkle(35, true); break;
    case twinkle_single: runTwinkle(1, false); break;
    case snow_sparkle: runSnowSparkle(); break;
  }
}
