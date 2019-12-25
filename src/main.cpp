#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

// Settings
#define NUM_LEDS 1
#define KNOB_PUSH_PIN 14
#define ENCODER_PIN_A 4
#define ENCODER_PIN_B 5
#define LED_PIN 0
#define DEBOUNCE_DELAY 50
#define KNOB_MODE_IDLE_TIMEOUT 1000 * 10 // 10 seconds

// Knob modes
#define MODE_SET_H 0
#define MODE_SET_S 1
#define MODE_SET_V 2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

// rotary encoder
Encoder knobLeft(ENCODER_PIN_A, ENCODER_PIN_B);
int32_t oldEncoderValue = 0;
uint8_t knobMode = MODE_SET_V;

// button
int8_t lastButtonState = HIGH;
int8_t buttonState = HIGH;
int lastDebounceTime = 0;

// persistent state
struct PersistentState {
  uint8_t colorH;
  int16_t colorS;
  int16_t colorV;
} persistentState;
int32_t lastEepromCommit = 0;

int32_t idleSince = 0;

int32_t clamp(int32_t v, int32_t lo, int32_t hi) {
  return max(min(v, hi), lo);
}

void updateColor() {
  uint8_t brightness = max((int16_t)0, persistentState.colorV);
  uint32_t color;

  if (persistentState.colorS < 0) {
    color = strip.Color(0, 0, 0, brightness);
  }
  else {
    color = strip.ColorHSV(persistentState.colorH * 256, persistentState.colorS, brightness);
  }

  strip.setPixelColor(0, color);
  strip.show();
}

void setup() {
  // disable wifi
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  // init EEPROM
  EEPROM.begin(sizeof(PersistentState));
  EEPROM.get(0, persistentState);

  // init led strip
  strip.begin();
  updateColor();

  // setup knob button
  pinMode(KNOB_PUSH_PIN, INPUT_PULLUP);
  digitalWrite(KNOB_PUSH_PIN, HIGH);

  // init serial comm
  Serial.begin(74880);
}

void loop() {
  // button

  int8_t buttonReading = digitalRead(KNOB_PUSH_PIN);
  if (buttonReading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY && buttonReading != buttonState) {
    idleSince = millis(); // reset idle timer

    buttonState = buttonReading;
    // button changed
    if (buttonReading == LOW) {
      if (knobMode == MODE_SET_V) {
        knobMode = MODE_SET_H;
      } else if (knobMode == MODE_SET_H) {
        knobMode = MODE_SET_S;
      } else if (knobMode == MODE_SET_S) {
        knobMode = MODE_SET_V;
      }
    }
  }

  lastButtonState = buttonReading;

  // rotary encoder

  int32_t newEncoderValue = knobLeft.read();
  int32_t valueDiff = newEncoderValue - oldEncoderValue;

  if (valueDiff != 0) {
    idleSince = millis(); // reset idle timer

    if (knobMode == MODE_SET_H) {
      persistentState.colorH = (persistentState.colorH + valueDiff) % 256;
    } else if (knobMode == MODE_SET_S) {
      persistentState.colorS = clamp(persistentState.colorS + valueDiff * 2, -8, 255);
    } else if (knobMode == MODE_SET_V) {
      persistentState.colorV = clamp(persistentState.colorV + valueDiff * 4, -8, 255);
    }

    EEPROM.put(0, persistentState);

    updateColor();
    oldEncoderValue = newEncoderValue;
  }

  // commit EEPROM every two seconds

  if (millis() - lastEepromCommit > 2000) {
    EEPROM.commit();
    lastEepromCommit = millis();
  }

  // reset knob mode when idled for a specified time
  if (millis() - idleSince > KNOB_MODE_IDLE_TIMEOUT) {
    knobMode = MODE_SET_V;
  }
}
