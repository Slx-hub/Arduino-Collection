#include "FastLED.h"
#define NUM_LEDS 7
#define DATA_PIN 2
#define MS_TIMEOUT 30
#define FADE_OFFSET 300

CRGB leds[NUM_LEDS];
CRGB colors[] = {0x000000, 0x000000, 0x330000, 0xA52600, 0x0094FF, 0x0094FF};
int ms_timestamps[] = {-5000, 0, 2000, 3200, 5000, 10000};
int color_count;
int current_indices[NUM_LEDS];
int current_ms = 0;

void setup() {
  color_count = (sizeof(colors) / sizeof(colors[0]));
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}


void loop() {
  if (current_ms < ms_timestamps[color_count - 1]) {
    calculatePixel(current_ms, 0);
    FastLED.show();
    
    current_ms += MS_TIMEOUT;
    delay(MS_TIMEOUT);
  }
}

void calculatePixel(int ms, int index) {
  if (index >= NUM_LEDS) {
    return;
  }
  if (ms > ms_timestamps[ current_indices[index] ]) {
    current_indices[index] += 1;
  }
  int lastTimestamp = ms_timestamps[ current_indices[index] - 1 ];
  int nextTimestamp = ms_timestamps[ current_indices[index] ];
  int millisBetween = nextTimestamp - lastTimestamp;
  int relativeMilli = ms - lastTimestamp;
  float relativeProgress = (float)relativeMilli / millisBetween;
  setRGB(index, colors[ current_indices[index] - 1 ], colors[ current_indices[index] ], relativeProgress);
  
  calculatePixel(ms - FADE_OFFSET, ++index);
}

void setRGB(int index, CRGB start, CRGB finish, float progress) {
  leds[index].r = (int) (progress * finish.r + (1.0f - progress) * start.r );
  leds[index].g = (int) (progress * finish.g + (1.0f - progress) * start.g );
  leds[index].b = (int) (progress * finish.b + (1.0f - progress) * start.b);
}
