#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 1
double TAU = PI * 2;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, 5, NEO_GRBW + NEO_KHZ800);

int count = 0;

void setup()
{
  strip.begin();
}

void loop()
{
  count++;

  double pos = count / 40.0;
  strip.setPixelColor(0, strip.Color(floor((sin(pos) / 2.0 + 0.5) * 255), floor((sin(pos + TAU * 0.33) / 2.0 + 0.5) * 255), floor((sin(pos + TAU * 0.66) / 2.0 + 0.5) * 255), 0));
  strip.show();
  delay(10);
}
